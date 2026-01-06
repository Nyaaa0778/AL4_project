#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "Skydome.h"

class TitleScene {
public:
	// シーンのフェーズ
	enum class Phase {
		kFadeIn, // フェードイン
		kMain,   // メイン部分
		kFadeOut // フェードアウト
	};

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// アニメーション用の経過時間
	float elapsedTimer_ = 0.0f;

	// 1フレームあたりの経過時間
	static inline const float kDeltaTimer = 1.0f / 60.0f;
	// 上下移動の幅（Y座標の変化量）
	static inline const float kMoveRangeY = 0.6f;
	// 1往復にかかる時間（秒）
	static inline const float kMoveCycle = 2.0f;

	// 等間隔4連パルス設定
	static inline const int kPulses = 4;            // 4連
	static inline const float kBPM = 24.0f;         // テンポ（お好みで 80〜110）
	static inline const float kBaseScale = 1.0f;    // 基本スケール
	static inline const float kPulseScale = 1.08f;  // 各パルスのピーク倍率
	static inline const float kAccentScale = 1.08f; // 1発目の強調（均一なら kPulseScale と同じ）

	// 1パルス内の時間配分（比率）
	static inline const float kUpRatio = 3.0f;   // 上りは短く鋭く
	static inline const float kDownRatio = 5.0f; // 下りはやや長め
	static inline const float kGapRatio = 2.0f;  // 「っ」の間

	// 終了フラグ
	bool isFinished_ = false;

	// フェード
	Fade* fade_ = nullptr;
	// フェード時間
	float duration_ = 1.0f;

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// タイトルロゴ（3D）
	KamataEngine::Model* modelTitleLogo_ = nullptr;
	KamataEngine::WorldTransform worldTransformTitleLogo_;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome* skydome_ = nullptr;

	KamataEngine::Model* modelTitleCloud_ = nullptr;
	KamataEngine::WorldTransform worldTransformTitleCloud_;

	// スプライト
	KamataEngine::Sprite* spriteStartText_ = nullptr;
	KamataEngine::Sprite* spriteTutorialText_ = nullptr;

public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// 補助関数
	static float Lerp(float a, float b, float t);
	static float EaseOutCubic(float t);
	static float EaseInCubic(float t);
	/// <summary>
	/// イージング
	/// </summary>
	float Easing();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ゲッター
	/// </summary>
	/// <returns></returns>
	bool IsFinished() const;
};
