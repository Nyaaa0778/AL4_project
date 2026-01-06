#include "Fireworks.h"
#include "Random.h"
#include "WorldTransformUpdater.h"

#include <algorithm> // clamp
#include <cmath>     // sin, cos, pow
#include <numbers>   // pi_v
#include <utility>   // std::move

using namespace KamataEngine::MathUtility;

void Fireworks::Initialize(KamataEngine::Model* particleModel, KamataEngine::Camera* camera) {
	particleModel_ = particleModel;
	camera_ = camera;
}

void Fireworks::Burst(const KamataEngine::Vector3& explosionCenter, int particleCount, float minSpeed, float maxSpeed, float minLifetimeSec, float maxLifetimeSec) {
	Random::SeedEngine();

	for (int i = 0; i < particleCount; ++i) {
		const float azimuthAngleRad = Random::GeneraterFloat(0.0f, 2.0f * std::numbers::pi_v<float>); // θ
		const float polarAngleRad = Random::GeneraterFloat(0.0f, std::numbers::pi_v<float>);          // φ
		const float initialSpeed = Random::GeneraterFloat(minSpeed, maxSpeed);

		KamataEngine::Vector3 direction = {
		    std::sin(polarAngleRad) * std::cos(azimuthAngleRad),
		    std::cos(polarAngleRad),
		    std::sin(polarAngleRad) * std::sin(azimuthAngleRad),
		};

		// 粒をヒープ確保（auto禁止なので明示型で）
		std::unique_ptr<Spark> spark(new Spark());

		spark->worldTransform.Initialize();
		spark->worldTransform.translation_ = explosionCenter;

		spark->velocity = direction * initialSpeed;
		spark->elapsedTimeSec = 0.0f;
		spark->lifetimeSec = Random::GeneraterFloat(minLifetimeSec, maxLifetimeSec);
		spark->startScale = Random::GeneraterFloat(0.12f, 0.22f);
		spark->endScale = 0.0f;
		spark->isAlive = true;

		particles_.push_back(std::move(spark));
	}
}

void Fireworks::Update(float deltaTimeSec) {
	const float airDragFactor = 0.98f;              // 空気抵抗（1に近いほど弱い）
	const float gravityAcceleration = -9.8f * 0.6f; // 下向き重力（y+が上想定）

	// 物理更新
	for (std::size_t i = 0; i < particles_.size(); ++i) {
		Spark* s = particles_[i].get();
		if (!s->isAlive) {
			continue;
		}

		s->elapsedTimeSec += deltaTimeSec;
		if (s->elapsedTimeSec >= s->lifetimeSec) {
			s->isAlive = false;
			continue;
		}

		// 速度更新
		s->velocity.y += gravityAcceleration * deltaTimeSec;
		// 60FPS基準で drag を適用（可変フレームでも近似的に効く）
		s->velocity = s->velocity * std::pow(airDragFactor, deltaTimeSec * 60.0f);

		// 位置更新
		s->worldTransform.translation_ = s->worldTransform.translation_ + s->velocity * deltaTimeSec;

		// スケール補間
		const float lifeRatio = std::clamp(s->elapsedTimeSec / s->lifetimeSec, 0.0f, 1.0f);
		const float scale = Lerp(s->startScale, s->endScale, Smooth(lifeRatio));
		s->worldTransform.scale_ = {scale, scale, scale};

		WorldTransformUpdate(s->worldTransform);
	}

	// 死亡済みの粒を削除（手動ループ：auto不使用）
	for (std::size_t i = 0; i < particles_.size(); /* 手動で進める */) {
		if (!particles_[i]->isAlive) {
			particles_.erase(particles_.begin() + static_cast<std::ptrdiff_t>(i));
		} else {
			++i;
		}
	}
}

void Fireworks::Draw() {
	if (particleModel_ == nullptr || camera_ == nullptr) {
		return;
	}
	for (std::size_t i = 0; i < particles_.size(); ++i) {
		const Spark* s = particles_[i].get();
		if (!s->isAlive) {
			continue;
		}
		particleModel_->Draw(s->worldTransform, *camera_);
	}
}

float Fireworks::Lerp(float a, float b, float t) { return a + (b - a) * t; }

float Fireworks::Smooth(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}
