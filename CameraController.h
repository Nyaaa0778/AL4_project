#pragma once
#include "KamataEngine.h"

class Player;

class CameraController {
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

private:
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	Player* target_ = nullptr;

	KamataEngine::Vector3 targetOffset_ = {0, 0, -15.0f};

	Rect movableArea_ = {0, 100, 0, 100};

	// カメラの目標座標
	KamataEngine::Vector3 targetPosition_;
	KamataEngine::Vector3 targetVelocity_;
	static inline const float kInterpolationRate = 0.3f;

	// 速度掛け率
	static inline const float kVelocityBias = 20.0f;
	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect margin = {0.0f, 8.0f, 0.0f, 0.0f};

	KamataEngine::Vector3 smoothedVelocity_ = {0, 0, 0};
	float velocitySmoothRate_ = 0.12f; // 小さめにするとホロウナイト風
	float lookAheadScaleX_ = 12.0f;    // 横の先読み量（調整可）
	float lookAheadScaleY_ = 4.0f;     // 縦の先読み量（かなり小さく）

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// リセット
	/// </summary>
	void Reset();
	/// <summary>
	/// 線形補間
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="interpolatinRate"></param>
	/// <returns></returns>
	KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& start, const KamataEngine::Vector3& end, float interpolatinRate);
	/// <summary>
	/// セッター
	/// </summary>
	/// <param name="target"></param>
	void SetTarget(Player* target);
	void SetCamera(KamataEngine::Camera* camera);
	void SetMovableArea(Rect area);
};
