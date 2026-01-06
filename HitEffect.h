#pragma once
#include "KamataEngine.h"

class HitEffect {
public:
	enum class State {
		kExpansionAnimetion, // 拡大アニメーション
		kFadeOut,            // フェードアウト
		kDisappear           // 消滅
	};

private:
	// 現在フェーズ
	State state_;

	float counter_ = 0.0f;
	static inline const float kExpansionAnimetionTime = 0.1f;
	static inline const float kFadeOutTime = 0.5f;

	// 円のワールドトランスフォーム
	KamataEngine::WorldTransform circleWorldTransform_;

	// 楕円の個数
	static inline const uint32_t kEllipseCount = 2;
	// 楕円のワールドトランスフォーム
	std::array<KamataEngine::WorldTransform, kEllipseCount> ellipseWorldTransforms_;

	// モデル
	static KamataEngine::Model* model_;

	// カメラ
	static KamataEngine::Camera* camera_;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const KamataEngine::Vector3& spawnPosition);

	/// <summary>
	/// インスタンス生成と初期化
	/// </summary>
	/// <returns></returns>
	static HitEffect* Create(const KamataEngine::Vector3& spawnPosition);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// フェーズの切り替え処理
	/// </summary>
	void ChangePhase();

	/// <summary>
	/// イージング
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	float EaseOut(float start, float end, float t);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ゲッター
	/// </summary>
	/// <returns></returns>
	bool IsDead() const { return state_ == State::kDisappear; }

	/// <summary>
	/// セッター
	/// </summary>
	/// <param name="model"></param>
	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }
};
