#define NOMINMAX
#include "Player.h"
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
		//ジャンプ開始
		if (velocity_.y > 0.0f) {
		//空中状態に移行
			onGround_ = false;
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

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速度
			velocity_ += Vector3(0, kJumpAcceleration, 0);
		}

	} else {
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kMaxFallSpeed);

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

	// 移動
	worldTransform_.translation_ += velocity_;

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
}
/// <summary>
/// 描画
/// </summary>
void Player::Draw() { model_->Draw(worldTransform_, *camera_); }