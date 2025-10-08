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

public:
	//初期化
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position);
	//更新
	void Update();
	//描画
	void Draw();

};
