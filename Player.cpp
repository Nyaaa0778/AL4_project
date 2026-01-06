#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "cassert"
#include <cmath>
#include <numbers>

#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// 初期化処理
/// </summary>
/// <param name="model">モデル</param>
/// <param name="camera">カメラ</param>
void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position) {
	// NULLポインタチェック
	assert(model);
	assert(modelAttack);

	model_ = model;
	modelAttack_ = modelAttack;

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransformAttack_.Initialize();

	// 位置調整
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 攻撃エフェクト用も同じ初期位置・向きに
	worldTransformAttack_.translation_ = position;
	worldTransformAttack_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

/// <summary>
/// 更新処理
/// </summary>
void Player::Update() {

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振るまいを変更
		behavior_ = behaviorRequest_;
		// 各振るまいごとの初期化を実行
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			// ルートビヘイビア(通常行動)の初期化
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			// 攻撃ビヘイビア(攻撃行動)の初期化
			BehaviorAttackInitialize();
			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kRoot:
	default:
		// ルートビヘイビア(通常行動)の更新
		BehaviorRootUpdate();
		break;
	case Behavior::kAttack:
		// 攻撃ビヘイビア(攻撃行動)の更新
		BehaviorAttackUpdate();
		break;
	}

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
	// 攻撃エフェクト用の行列更新も必ず呼ぶ
	WorldTransformUpdate(worldTransformAttack_);
}

/// <summary>
/// 移動入力
/// </summary>
void Player::UpdateMovementInput() {

	// ======= 縦方向（ジャンプ・重力） =======
	if (onGround_) {
		// 地上にいるとき、上向き速度がついたら空中へ
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		}

		// 地上ジャンプ（スペース）
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			velocity_.y = kJumpAcceleration;
		}
	} else {
		// 空中にいるとき

		// 重力
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kMaxFallSpeed);

		// 空中ジャンプ入力（スペース）
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

			// --- 壁ジャンプ優先 ---
			if (isOnWall_ && wallDirection_ != 0) {
				// 壁の反対側に向かう横速度
				velocity_.x = -wallDirection_ * kWallJumpHorizontalSpeed;
				// 上方向の速度
				velocity_.y = kWallJumpVerticalSpeed;

				// 壁から離れるので解除
				isOnWall_ = false;
				wallDirection_ = 0;

			} else if (jumpCount_ < kMaxJumpCount) {
				// 追加空中ジャンプ
				++jumpCount_;
				velocity_.y = kJumpAcceleration;
			}
		}
	}

	// ======= 横方向（左右移動） =======
	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		Vector3 acceleration{};

		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			// 右入力中に左向き速度が出ていたら急ブレーキ
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x += kAcceleration;

			// 右向きでないなら右を向き始める
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;

				// 旋回開始時の角度を保存
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				// 旋回タイマーをセット
				turnTimer_ = kTimeTurn;
			}

		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			// 左入力中に右向き速度が出ていたら急ブレーキ
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x -= kAcceleration;

			// 左向きでないなら左を向き始める
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;

				// 旋回開始時の角度を保存
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				// 旋回タイマーをセット
				turnTimer_ = kTimeTurn;
			}
		}

		// 加速度を速度に加算
		velocity_ += acceleration;
		// 最大速度を制限
		velocity_.x = std::clamp(velocity_.x, -kMaxSpeed, kMaxSpeed);

	} else {
		// 入力していないときは移動減衰をかける
		velocity_.x *= (1.0f - kAttenuation);
	}
}

/// <summary>
/// 通常行動の初期化
/// </summary>
void Player::BehaviorRootInitialize() {}

/// <summary>
/// 攻撃行動の初期化
/// </summary>
void Player::BehaviorAttackInitialize() {
	// カウンター初期化
	attackParameter_ = 0.0f;
	attackPhase_ = AttackPhase::kCharge;
}

/// <summary>
/// 通常行動の更新
/// </summary>
void Player::BehaviorRootUpdate() {
	///===========================================
	/// 移動処理
	/// ===========================================
	UpdateMovementInput();

	///===========================================
	/// 攻撃処理
	/// ===========================================

	// 攻撃キー(E)を押したら
	if (Input::GetInstance()->TriggerKey(DIK_E)) {

		// 攻撃ビヘイビアをリクエスト
		behaviorRequest_ = Behavior::kAttack;
	}

	///===========================================
	/// マップとの当たり判定
	/// ===========================================

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo{};
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = velocity_;

	// マップ衝突チェック
	IsMapCollision(collisionMapInfo);
	// 結果を反映させて移動
	MoveByCollisionResult(collisionMapInfo);
	ChangeGroundState(collisionMapInfo);

	///===========================================
	/// プレイヤーモデルの向きの調整
	/// ===========================================
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// イージング
		float turnProgress = std::clamp(1.0f - turnTimer_ / kTimeTurn, 0.0f, 1.0f);
		float smoothTurnProgress = EaseInOutSine(turnProgress);

		// 左右のキャラ角度テーブル
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		// 状態に応じた角度を取得
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// イージングをつけて振り返る
		worldTransform_.rotation_.y = std::lerp(turnFirstRotationY_, destinationRotationY, smoothTurnProgress);
	}
}

/// <summary>
/// 攻撃行動の更新
/// </summary>
void Player::BehaviorAttackUpdate() {
	// 予備動作
	attackParameter_ += 1.0f / 60.0f;

	// 攻撃動作用の速度
	Vector3 velocity{};

	switch (attackPhase_) {
	case AttackPhase::kCharge:
	default: {

		float t = attackParameter_ / kChargeTime;
		worldTransform_.scale_.z = EaseOut(1.0f, 0.3f, t);
		worldTransform_.scale_.y = EaseOut(1.0f, 1.6f, t);

		// 前進動作へ移行
		if (attackParameter_ >= kChargeTime) {
			isAttackEffect_ = true;

			attackPhase_ = AttackPhase::kDash;

			// カウンターをリセット
			attackParameter_ = 0.0f;
		}

		break;
	}
	case AttackPhase::kDash: {

		float t = attackParameter_ / kDashTime;
		worldTransform_.scale_.z = EaseOut(0.3f, 0.3f, t);
		worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

		if (lrDirection_ == LRDirection::kRight) {
			velocity = attackVelocity;
		} else {
			velocity = -attackVelocity;
		}

		// 前進動作へ移行
		if (attackParameter_ >= kDashTime) {
			attackPhase_ = AttackPhase::kCoolDown;

			// カウンターをリセット
			attackParameter_ = 0.0f;
		}

		break;
	}
	case AttackPhase::kCoolDown: {

		float t = attackParameter_ / kCoolDownTime;
		worldTransform_.scale_.z = EaseOut(1.3f, 1.0f, t);
		worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

		if (attackParameter_ >= kCoolDownTime) {
			isAttackEffect_ = false;

			velocity_.x = 0.0f;
		}

		break;
	}
	}

	///===========================================
	/// マップとの当たり判定
	/// ===========================================

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo{};
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = velocity;

	// マップ衝突チェック
	IsMapCollision(collisionMapInfo);
	// 結果を反映させて移動
	MoveByCollisionResult(collisionMapInfo);
	ChangeGroundState(collisionMapInfo);

	// トランスフォームの値をコピー
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	if (attackParameter_ >= kAttackTime) {
		// 攻撃終了して通常状態に戻す
		behaviorRequest_ = Behavior::kRoot;
	}
}

/// <summary>
/// マップとの当たり判定
/// </summary>
/// <param name="info"></param>
void Player::IsMapCollision(CollisionMapInfo& info) {
	IsTopMapCollision(info);
	IsBottomMapCollision(info);
	IsRightMapCollision(info);
	IsLeftMapCollision(info);
}

/// <summary>
/// 天井との当たり判定
/// </summary>
/// <param name="info"></param>
void Player::IsTopMapCollision(CollisionMapInfo& info) {
	// 下に移動しているか？
	if (info.moveAmount.y <= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i)
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool isHit = false;

	MapChipField::IndexSet indexSet;

	// 左上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock)
		isHit = true;

	// 右上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock)
		isHit = true;

	if (isHit) {
		// めり込みを排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(0, kHeight / 2.0f, 0));

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, kHeight / 2.0f, 0));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先のブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			// 天井に当たったことを記録
			info.isHitCeiling = true;
		}
	}
}

void Player::IsBottomMapCollision(CollisionMapInfo& info) {
	// 上に移動しているか？
	if (info.moveAmount.y >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i)
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));

	MapChipType mapChipType, mapChipTypeNext;
	MapChipField::IndexSet indexSet;
	bool isHit = false;

	// 左下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
	}

	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
	}

	if (isHit) {
		// めり込みを排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(0, -kHeight / 2.0f, 0));

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先のブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));

			// 床に当たったことを記録
			info.isHitLanding = true;
		}
	}
}

void Player::IsRightMapCollision(CollisionMapInfo& info) {
	// 左に移動しているか？
	if (info.moveAmount.x <= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i)
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));

	MapChipType mapChipType;
	MapChipField::IndexSet indexSet;
	bool isHit = false;

	// 右上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		isHit = true;
	}

	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		isHit = true;
	}

	if (isHit) {
		// めり込みを排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(kWidth / 2.0f, 0, 0));

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先のブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));

			// 壁に当たったことを記録
			info.isHitWall = true;
			info.wallDirection = 1; // 右壁
		}
	}
}

void Player::IsLeftMapCollision(CollisionMapInfo& info) {
	// 右に移動しているか？
	if (info.moveAmount.x >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i)
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));

	MapChipType mapChipType;
	MapChipField::IndexSet indexSet;
	bool isHit = false;

	// 左上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		isHit = true;
	}

	// 左下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		isHit = true;
	}

	if (isHit) {
		// めり込みを排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + Vector3(-kWidth / 2.0f, 0, 0));

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先のブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));

			// 壁に当たったことを記録
			info.isHitWall = true;
			info.wallDirection = -1; // 左壁
		}
	}
}

/// <summary>
/// 衝突応答
/// </summary>
/// <param name="player"></param>
void Player::OnCollision(const Enemy* enemy) {

	// 攻撃中はダメージ無効
	if (IsAttack()) {
		return;
	}

	(void)enemy;

	isDead_ = true;
}

/// <summary>
/// 接地状態を切り替える処理
/// </summary>
/// <param name="info"></param>
void Player::ChangeGroundState(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 移動後の4つの角の座標
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Corner>(i));
			}

			// 真下の当たり判定
			bool isHit = false;

			MapChipType mapChipType;
			MapChipField::IndexSet indexSet;

			// 左下の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundCheckOffset, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				isHit = true;
			}

			// 右下の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundCheckOffset, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				isHit = true;
			}

			if (!isHit) {
				onGround_ = false;
			}
		}
	} else {
		if (info.isHitLanding) {
			// 着地状態に切り替える
			onGround_ = true;
			// 着地時にx方向の速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// y方向の速度を0にする
			velocity_.y = 0.0f;

			// 着地したので空中ジャンプ回数と壁状態をリセット
			jumpCount_ = 0;
			isOnWall_ = false;
			wallDirection_ = 0;
		}
	}
}

/// <summary>
/// 角の位置を取得
/// </summary>
/// <param name="center"></param>
/// <param name="corner"></param>
/// <returns></returns>
Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {kWidth / 2.0f,  -kHeight / 2.0f, 0}, //  右下
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  左下
	    {kWidth / 2.0f,  kHeight / 2.0f,  0}, //  右上
	    {-kWidth / 2.0f, kHeight / 2.0f,  0}  //  左上
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

/// <summary>
/// 当たり判定の結果を反映させて移動
/// </summary>
/// <param name="info"></param>
void Player::MoveByCollisionResult(const CollisionMapInfo& info) {
	ReactToCeilingHit(info);
	ReactToWallHit(info);

	worldTransform_.translation_ += info.moveAmount;
}

/// <summary>
/// 天井に当たったときの処理
/// </summary>
/// <param name="info"></param>
void Player::ReactToCeilingHit(const CollisionMapInfo& info) {
	if (info.isHitCeiling) {
		velocity_.y = 0;
	}
}

/// <summary>
/// 壁に接触しているときの処理
/// </summary>
/// <param name="info"></param>
void Player::ReactToWallHit(const CollisionMapInfo& info) {
	if (info.isHitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
		isOnWall_ = true;
		wallDirection_ = info.wallDirection;
	} else {
		// 壁から離れた
		isOnWall_ = false;
		wallDirection_ = 0;
	}
}

void Player::UpdateMatricesOnly() {
	// 見た目が破綻しないよう、行列だけは毎フレーム更新
	WorldTransformUpdate(worldTransform_);
	WorldTransformUpdate(worldTransformAttack_);
}

/// <summary>
/// イージング
/// </summary>
/// <param name="t"></param>
/// <returns></returns>
float Player::EaseInOutSine(float turnProgress) { return -(std::cos(std::numbers::pi_v<float> * turnProgress) - 1.0f) * 0.5f; }
float Player::EaseOut(float start, float end, float t) {
	// tを0〜1の範囲に制限
	t = std::clamp(t, 0.0f, 1.0f);

	// EaseOut補間
	float easedT = 1.0f - std::pow(1.0f - t, 3.0f);

	// 線形補間を実行
	return start + (end - start) * easedT;
}

/// <summary>
/// 描画処理
/// </summary>
void Player::Draw() {
	if (!isDead_) {
		// 3Dモデルを描画
		model_->Draw(worldTransform_, *camera_);
		if (isAttackEffect_) {
			modelAttack_->Draw(worldTransformAttack_, *camera_);
		}
	}
}

/// <summary>
/// ゲッター
/// </summary>
/// <returns></returns>
const WorldTransform& Player::GetWorldTransform() const { return worldTransform_; }
const Vector3& Player::GetVelocity() const { return velocity_; }
Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}
AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}
bool Player::IsDead() const { return isDead_; }
bool Player::IsAttack() const { return behavior_ == Behavior::kAttack; }

/// <summary>
/// セッター
/// </summary>
/// /// <param name="mapChipField"></param>
void Player::SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
