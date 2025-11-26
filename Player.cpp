#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// 初期化
/// </summary>
/// <param name="model"></param>
/// <param name="camera"></param>
/// <param name="position"></param>
void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLチェック
	assert(model);
	assert(camera);

	// プレイヤーモデル
	model_ = model;
	// カメラ
	camera_ = camera;

	// プレイヤー初期位置
	worldTransform_.Initialize();
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransform_.translation_ = position;

	jumpCount_ = 0;
}
/// <summary>
/// 更新
/// </summary>
void Player::Update() {

	// 着地フラグ
	bool isLanded = false;

	if (velocity_.y < 0) {
		// Y座標が地面以下になったら着地
		if (worldTransform_.translation_.y <= 1.0f) {
			isLanded = true;
		}
	}

	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		}

		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			jumpCount_++;
			// ジャンプ初速度
			velocity_ = Vector3(velocity_.x, kJumpAcceleration, 0);
		}

		// ジャンプ回数リセット
		jumpCount_ = 0;

	} else {
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kMaxFallSpeed);

		if (Input::GetInstance()->TriggerKey(DIK_UP)) {

			// --- 壁ジャンプ優先 ---
			if (isOnWall_ && wallDirection_ != 0) {
				// 壁の反対側に向かう横速度
				velocity_.x = -wallDirection_ * kWallJumpHorizontalSpeed;
				// 上方向の速度
				velocity_.y = kWallJumpVerticalSpeed;

				// 壁から離れたので解除
				isOnWall_ = false;
				wallDirection_ = 0;

			} else if (jumpCount_ < kMaxJumpCount) {
				jumpCount_++;
				velocity_.y = kJumpAcceleration;
			}
		}

		if (isLanded) {
			// めり込み排斥
			worldTransform_.translation_.y = 1.0f;
			// 摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
			// 接地状態に移行
			onGround_ = true;
		}
	}

	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		// 左右加速
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			acceleration.x += kAcceleration;

			// 右移動中の入力
			if (velocity_.x < 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.x *= (1.0f - kAttenuation);
			}
		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			acceleration.x -= kAcceleration;

			// 左移動中の入力
			if (velocity_.x > 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.x *= (1.0f - kAttenuation);
			}
		}

		// 加速、減速
		velocity_ += acceleration;

		// 速度制限
		velocity_.x = std::clamp(velocity_.x, -kMaxSpeed, kMaxSpeed);
	} else {
		// 非入力時に移動減衰をかける
		velocity_.x *= (1.0f - kAttenuation);
	}

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = velocity_;

	// マップ衝突チェック
	IsMapCollision(collisionMapInfo);
	// 結果を反映させて移動
	MoveByCollisionResult(collisionMapInfo);
	ChangeGroundState(collisionMapInfo);

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
}
/// <summary>
/// 描画
/// </summary>
void Player::Draw() { model_->Draw(worldTransform_, *camera_); }

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
			// 右壁に衝突
			info.wallDirection = 1;
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
			// 左壁に衝突
			info.wallDirection = -1;
		}
	}
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

			// 隣接セルがともにブロックならヒット
			if (mapChipType == MapChipType::kBlock) {
				isHit = true;
			}

			// 右下の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundCheckOffset, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			// 隣接セルがともにブロックならヒット
			if (mapChipType == MapChipType::kBlock) {
				isHit = true;
			}

			if (!isHit) {
				onGround_ = false;
			}
		}
	} else {
		if (info.isHitLanding) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;

			// 着地したら壁状態解除
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
		// 衝突していなければ壁状態を解除
		isOnWall_ = false;
		wallDirection_ = 0;
	}
}

const WorldTransform& Player::GetWorldTransform() const { return worldTransform_; }
const Vector3& Player::GetVelocity() const { return velocity_; }
