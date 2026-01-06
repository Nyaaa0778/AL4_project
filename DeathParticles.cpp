#define NOMINMAX
#include "DeathParticles.h"
#include "WorldTransformUpdater.h"
#include "math/MathUtility.h"
#include <cassert>

#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// 初期化
/// </summary>
/// <param name="model"></param>
/// <param name="camera"></param>
/// <param name="position"></param>
void DeathParticles::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);

	model_ = model;

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

	// 色
	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};

	// ワールド変換の初期化
	for (WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}
}

/// <summary>
/// 更新
/// </summary>
void DeathParticles::Update() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限を達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱い
		isFinished_ = true;
	}

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		Vector3 velocity = {kSpeed, 0, 0};
		// 回転角を計算
		float angle = kAngleUnit * i;
		// z軸まわりの回転行列
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = Transform(velocity, matrixRotation);
		// 移動処理
		worldTransforms_[i].translation_ += velocity;
	}

	for (WorldTransform& worldTransform : worldTransforms_) {
		// 行列の更新
		WorldTransformUpdate(worldTransform);
	}

	color_.w = std::clamp(1.0f - counter_ / kDuration, 0.0f, 1.0f);
	// 色変更オブジェクトに色の数値を設定
	objectColor_.SetColor(color_);
}

/// <summary>
/// 描画
/// </summary>
void DeathParticles::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	for (WorldTransform& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}

/// <summary>
/// ゲッター
/// </summary>
/// <returns></returns>
bool DeathParticles::IsFinished() const { return isFinished_; }