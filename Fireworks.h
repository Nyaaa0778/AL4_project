#pragma once
#include "KamataEngine.h"
#include <cstddef> // std::size_t
#include <memory>  // std::unique_ptr
#include <vector>

class Fireworks {
public:
	~Fireworks() {}

	// モデルとカメラを受け取って初期化
	void Initialize(KamataEngine::Model* particleModel, KamataEngine::Camera* camera);

	// 爆発を1発生成
	void Burst(const KamataEngine::Vector3& explosionCenter, int particleCount = 80, float minSpeed = 3.0f, float maxSpeed = 8.0f, float minLifetimeSec = 0.7f, float maxLifetimeSec = 1.4f);

	// 毎フレーム更新（deltaTimeSec = 1/60 など）
	void Update(float deltaTimeSec);

	// 描画
	void Draw();

	bool IsEmpty() const { return particles_.empty(); }

private:
	struct Spark {
		KamataEngine::WorldTransform worldTransform; // 粒の変換
		KamataEngine::Vector3 velocity;              // 速度
		float elapsedTimeSec;                        // 生存時間の経過
		float lifetimeSec;                           // 寿命
		float startScale;                            // 開始スケール
		float endScale;                              // 終了スケール
		bool isAlive;                                // 生存フラグ
	};

	KamataEngine::Model* particleModel_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// コピー不要にするため unique_ptr で保持
	std::vector<std::unique_ptr<Spark>> particles_;

	static float Lerp(float a, float b, float t);
	static float Smooth(float t);
};
