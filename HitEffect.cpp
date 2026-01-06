#include "HItEffect.h"
#include "Random.h"
#include "WorldTransformUpdater.h"
#include <numbers>
#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

// 静的メンバ変数の実体
Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

/// <summary>
/// 初期化
/// </summary>
void HitEffect::Initialize(const KamataEngine::Vector3& spawnPosition) {

	state_ = State::kExpansionAnimetion;
	counter_ = 0.0f;

	///============================================
	/// 円形エフェクト
	/// ===========================================

	circleWorldTransform_.translation_ = spawnPosition;
	circleWorldTransform_.Initialize();

	///============================================
	/// 楕円エフェクト
	/// ===========================================

	// 乱数
	Random::SeedEngine();

	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		worldTransform.scale_ = {0.2f, 3.0f, 1.0f};
		worldTransform.rotation_ = {0.0f, 0.0f, Random::GeneraterFloat(-std::numbers::pi_v<float>, std::numbers::pi_v<float>)};
		worldTransform.translation_ = spawnPosition;

		worldTransform.Initialize();
	}
}

/// <summary>
/// インスタンス生成と初期化
/// </summary>
/// <returns></returns>
HitEffect* HitEffect::Create(const KamataEngine::Vector3& spawnPosition) {
	// インスタンス生成
	HitEffect* instance = new HitEffect();
	// newの失敗を検出
	assert(instance);
	// インスタンスの初期化
	instance->Initialize(spawnPosition);
	// 初期化したインスタンスを返す
	return instance;
}

/// <summary>
/// 更新
/// </summary>
void HitEffect::Update() {

	ChangePhase();

	///============================================
	/// 円形エフェクト
	/// ===========================================

	//  行列の更新
	WorldTransformUpdate(circleWorldTransform_);

	for (KamataEngine::WorldTransform& worldTransform : ellipseWorldTransforms_) {
		WorldTransformUpdate(worldTransform);
	}
}

/// <summary>
/// フェーズの切り替え処理
/// </summary>
void HitEffect::ChangePhase() {

	counter_ += 1.0f / 60.0f;

	switch (state_) {
	case State::kExpansionAnimetion:

		///============================================
		/// 円形エフェクト
		/// ===========================================

		circleWorldTransform_.scale_ += Vector3(0.03f, 0.03f, 0.03f);

		///============================================
		/// 楕円エフェクト
		/// ===========================================

		for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
			worldTransform.scale_ += Vector3(0.01f, 0.01f, 0.01f);
		}

		if (counter_ >= kExpansionAnimetionTime) {
			state_ = State::kFadeOut;

			counter_ = 0.0f;
		}

		break;
	case State::kFadeOut: {

		// 0～1 に正規化
		float t = std::clamp(counter_ / kFadeOutTime, 0.0f, 1.0f);
		// EaseOut で 0→1、そこから 1− で 1→0 に反転
		float alpha = 1.0f - EaseOut(0.0f, 1.0f, t);

		// Model クラスについている SetAlpha を使う
		model_->SetAlpha(alpha);

		if (counter_ >= kFadeOutTime) {
			state_ = State::kDisappear;

			counter_ = 0.0f;
		}
	} break;
	case State::kDisappear:
	default:

		break;
	}
}

/// <summary>
/// イージング
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="t"></param>
/// <returns></returns>
float HitEffect::EaseOut(float start, float end, float t) {
	// tを0〜1の範囲に制限
	t = std::clamp(t, 0.0f, 1.0f);

	// EaseOut補間
	float easedT = 1.0f - std::pow(1.0f - t, 3.0f);

	// 線形補間を実行
	return start + (end - start) * easedT;
}

/// <summary>
/// 描画
/// </summary>
void HitEffect::Draw() {

	///============================================
	/// 円形エフェクト
	/// ===========================================

	model_->Draw(circleWorldTransform_, *camera_);

	///============================================
	/// 楕円エフェクト
	/// ===========================================

	for (KamataEngine::WorldTransform& wt : ellipseWorldTransforms_) {
		model_->Draw(wt, *camera_);
	}

}