#pragma once
#include<KamataEngine.h>

class Player {
private:
	/// <summary>
	/// ワールドトランスフォーム
	/// </summary>
	KamataEngine::WorldTransform worldTransform_;

	/// <summary>
	/// カメラ
	/// </summary>
	KamataEngine::Camera* camera_ = nullptr;

	/// <summary>
	/// モデル
	/// </summary>
	KamataEngine::Model* model_ = nullptr;

	//速度
	KamataEngine::Vector3 velocity_ = {};
	//加速度定数
	static inline const float kAcceleration = 0.001f;
	//移動減衰定数
	static inline const float kAttenuation = 0.05f;
	//速度制限
	static inline const float kMaxSpeed = 3.0f;

	//接地状態のフラグ
	bool onGround_ = true;
	//重力加速度定数
	static inline const float kGravityAcceleration = 0.03f;
	//最大落下速度定数
	static inline const float kMaxFallSpeed = 5.0f;
	//ジャンプ初速度定数
	static inline const float kJumpAcceleration = 0.5f;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model"></param>
	/// <param name="camera"></param>
	/// <param name="position"></param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

};
