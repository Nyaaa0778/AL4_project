#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"
#include "Player.h"
#include "WorldTransformUpdater.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// 初期化
/// </summary>
void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	model_ = model;

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

	worldTransform_.Initialize();

	// 速度を設定
	velocity_ = {-kWalkSpeed, 0, 0};

	// 見た目の左右向きも初期化
	lrDirection_ = (velocity_.x >= 0.0f) ? LRDirection::kRight : LRDirection::kLeft;
	nextDirection_ = lrDirection_;
	turnState_ = TurnState::kWalk;
	waitTurnTimer_ = 0.0f;
	turnTimer_ = 0.0f;

	walkTimer_ = 0.0f;

	// 位置調整
	worldTransform_.translation_ = position;
	// Player と同じ角度テーブル（Right: 90°, Left: 270°）
	float destinationRotationYTable[] = {std::numbers::pi_v<float>, 0.0f};
	worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
}
/// <summary>
/// 更新
/// </summary>
void Enemy::Update() {

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振るまいを変更
		behavior_ = behaviorRequest_;
		// 各振るまいごとの初期化を実行
		switch (behavior_) {
		case Behavior::kWalk:
		default:
			// 歩行状態の初期化
			BehaviorWalkInitialize();

			break;
		case Behavior::kDeath:

			// 死亡状態の初期化
			BehaviorDeathInitialize();

			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kWalk:
	default:

		// 歩行状態の更新
		BehaviorWalkUpdate();

		break;
	case Behavior::kDeath:

		// 死亡状態の更新
		BehaviorDeathUpdate();

		break;
	}

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
}

/// <summary>
/// 歩行状態の初期化
/// </summary>
void Enemy::BehaviorWalkInitialize() {}
/// <summary>
/// 死亡状態の初期化
/// </summary>
void Enemy::BehaviorDeathInitialize() {
	// カウンター初期化
	deathAnimetionTimer_ = 0.0f;
}

/// <summary>
/// 歩行状態の更新
/// </summary>
void Enemy::BehaviorWalkUpdate() {
	const float dt = 1.0f / 60.0f;

	//====================================================
	// 壁ヒット後の「待ち → 旋回」
	//====================================================
	if (turnState_ == TurnState::kWait) {
		waitTurnTimer_ -= dt;
		if (waitTurnTimer_ <= 0.0f) {
			turnState_ = TurnState::kTurn;
			turnTimer_ = kTimeTurn;
			turnFirstRotationY_ = worldTransform_.rotation_.y;
		}
	}

	if (turnState_ == TurnState::kTurn) {
		turnTimer_ -= dt;

		float progress = std::clamp(1.0f - turnTimer_ / kTimeTurn, 0.0f, 1.0f);
		float smooth = EaseInOutSine(progress);

		float destinationRotationYTable[] = {std::numbers::pi_v<float>, 0.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(nextDirection_)];

		// イージングをつけて振り返る
		worldTransform_.rotation_.y = std::lerp(turnFirstRotationY_, destinationRotationY, smooth);

		// 旋回終了
		if (turnTimer_ <= 0.0f) {
			lrDirection_ = nextDirection_;
			worldTransform_.rotation_.y = destinationRotationY;
			velocity_.x = (lrDirection_ == LRDirection::kRight) ? +kWalkSpeed : -kWalkSpeed;
			turnState_ = TurnState::kWalk;
		}
	}

	// 旋回中/待ち中は歩行移動しない
	if (turnState_ != TurnState::kWalk) {
		return;
	}

	//==============================
	// 移動 + マップチップ当たり判定
	//==============================
	if (mapChipField_) {
		CollisionMapInfo info{};
		info.moveAmount = velocity_;

		IsMapCollision(info);
		MoveByCollisionResult(info); // ここで translation_ += moveAmount される
	} else {
		// マップ未設定なら単純移動
		worldTransform_.translation_ += velocity_;
	}

	//==============================
	// 歩行アニメ（既存）
	//==============================
	walkTimer_ += dt;

	float param = std::sin(2.0f * std::numbers::pi_v<float> * (walkTimer_ / kWalkMotionTime));
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	worldTransform_.rotation_.x = degree * (std::numbers::pi_v<float> / 180.0f);
}

/// <summary>
/// 死亡状態の更新
/// </summary>
void Enemy::BehaviorDeathUpdate() {
	// 死亡アニメーションタイマーを加算
	deathAnimetionTimer_ += 1.0f / 60.0f;

	// 正規化タイマー（0.0～1.0）
	float t = std::clamp(deathAnimetionTimer_ / kDeathAnimetionTime, 0.0f, 1.0f);

	// 回転アニメーション
	worldTransform_.rotation_.y = EaseInOut(0.0f, 3.0f * 2.0f * std::numbers::pi_v<float>, t);
	worldTransform_.rotation_.x = EaseInOut(0.0f, std::numbers::pi_v<float> / 2.0f, t);

	// 演出終了後に死亡フラグを立てる
	if (deathAnimetionTimer_ >= kDeathAnimetionTime) {
		isDead_ = true;
	}
}

//================================================================================
// マップチップ当たり判定（左右）
//================================================================================

Vector3 Enemy::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {kWidth / 2.0f,  -kHeight / 2.0f, 0}, //  右下
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  左下
	    {kWidth / 2.0f,  kHeight / 2.0f,  0}, //  右上
	    {-kWidth / 2.0f, kHeight / 2.0f,  0}  //  左上
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Enemy::IsMapCollision(CollisionMapInfo& info) {
	IsRightMapCollision(info);
	IsLeftMapCollision(info);
}

void Enemy::IsRightMapCollision(CollisionMapInfo& info) {
	// 左に移動しているか？
	if (info.moveAmount.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	MapChipField::IndexSet indexSet;
	bool isHit = false;

	// 右上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		isHit = true;
	}

	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		isHit = true;
	}

	if (isHit) {
		// めり込みを排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(kWidth / 2.0f, 0, 0));

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.isHitWall = true;
			info.wallDirection = 1;
		}
	}
}

void Enemy::IsLeftMapCollision(CollisionMapInfo& info) {
	// 右に移動しているか？
	if (info.moveAmount.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
	}

	MapChipField::IndexSet indexSet;
	bool isHit = false;

	// 左上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		isHit = true;
	}

	// 左下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		isHit = true;
	}

	if (isHit) {
		// めり込みを排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(-kWidth / 2.0f, 0, 0));

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
			info.isHitWall = true;
			info.wallDirection = -1;
		}
	}
}

void Enemy::MoveByCollisionResult(const CollisionMapInfo& info) {
	ReactToWallHit(info);
	worldTransform_.translation_ += info.moveAmount;
}

void Enemy::ReactToWallHit(const CollisionMapInfo& info) {
	if (!info.isHitWall) {
		return;
	}

	// すでに旋回中/待ち中なら、二重に開始しない
	if (turnState_ != TurnState::kWalk) {
		return;
	}

	// 壁に当たったら一旦停止し、少し待ってから旋回して反対へ進む
	turnState_ = TurnState::kWait;
	waitTurnTimer_ = kWaitBeforeTurn;

	// 次に向く方向を決める（現在の向きの反対）
	nextDirection_ = (lrDirection_ == LRDirection::kRight) ? LRDirection::kLeft : LRDirection::kRight;

	// 待ち/旋回中は移動を止める
	velocity_.x = 0.0f;
}

/// <summary>
/// 衝突応答
/// </summary>
/// <param name="player"></param>
void Enemy::OnCollision(const Player* player) {
	(void)player;

	// 敵がやられているなら何もしない
	if (behavior_ == Behavior::kDeath) {
		return;
	}

	// プレイヤーが攻撃中なら死ぬ
	if (player->IsAttack()) {
		// 当たり判定無効フラグを立てる
		isCollisionDisabled_ = true;

		// 敵の振るまいを死亡演出に変更
		behaviorRequest_ = Behavior::kDeath;

		// 敵とプレイヤーの中間位置にエフェクトを生成
		Vector3 effectPos = (worldTransform_.translation_ + player->GetWorldTransform().translation_) / 2.0f;
		gameScene_->CreateHitEffect(effectPos);
	}
}

/// <summary>
/// イージング
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="t"></param>
/// <returns></returns>
float Enemy::EaseInOut(float start, float end, float t) {
	// 補間率を0～1にクランプ
	t = std::clamp(t, 0.0f, 1.0f);

	// イージングインアウトの曲線：3t^2 - 2t^3
	float easeT = t * t * (3 - 2 * t);

	return std::lerp(start, end, easeT);
}

float Enemy::EaseInOutSine(float t) {
	// Player::EaseInOutSine と同じ式
	t = std::clamp(t, 0.0f, 1.0f);
	return -(std::cos(std::numbers::pi_v<float> * t) - 1.0f) * 0.5f;
}

/// <summary>
/// 描画
/// </summary>
void Enemy::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}

/// <summary>
/// ゲッター
/// </summary>
/// <returns></returns>
Vector3 Enemy::GetWorldPosition() {

	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}
AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}
bool Enemy::IsDead() const { return isDead_; }
bool Enemy::IsCollisionDisabled() const { return isCollisionDisabled_; }

void Enemy::SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
