#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
#include "math/MathUtility.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// 初期化
/// </summary>
void CameraController::Initialize() {
	// カメラの初期化
	camera_->Initialize();
}

/// <summary>
/// リセット
/// </summary>
void CameraController::Reset() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}
/// <summary>
/// 更新
/// </summary>
void CameraController::Update() {

	const WorldTransform& w = target_->GetWorldTransform();

	// プレイヤーの raw velocity（生の速度）
	Vector3 rawVel = target_->GetVelocity();

	// 速度をスムージング
	smoothedVelocity_ = Lerp(smoothedVelocity_, rawVel, velocitySmoothRate_);

	// targetの少し先を見せる
	Vector3 lookAhead = {smoothedVelocity_.x * lookAheadScaleX_, smoothedVelocity_.y * lookAheadScaleY_, 0.0f};

	// 縦のゆれを抑えたいなら弱める
	lookAhead.y *= 0.3f;

	// 目標カメラ位置 = プレイヤー + 先読み + 固定オフセット
	targetPosition_ = w.translation_ + targetOffset_ + lookAhead;

	// カメラがゆっくり追従
	camera_->translation_ = Lerp(camera_->translation_, targetPosition_, kInterpolationRate);

	// 制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);

	camera_->UpdateMatrix();
}

/// <summary>
/// 線形補間
/// </summary>
/// <param name="p0"></param>
/// <param name="p1"></param>
/// <param name="t"></param>
/// <returns></returns>
Vector3 CameraController::Lerp(const Vector3& start, const Vector3& end, float interpolatinRate) { return start * (1.0f - interpolatinRate) + end * interpolatinRate; }

/// <summary>
/// セッター
/// </summary>
/// <param name="target"></param>
void CameraController::SetTarget(Player* target) { target_ = target; }
void CameraController::SetCamera(Camera* camera) { camera_ = camera; }
void CameraController::SetMovableArea(Rect area) { movableArea_ = area; }