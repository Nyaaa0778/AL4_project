#pragma once
#include "CameraController.h"
#include "Fade.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"

class TutorialScene {
public:
	// シーンのフェーズ
	enum class Phase { kFadeIn, kRun, kFadeOut };

	// チュートリアルのステップ
	enum class Step {
		kMove,   // 左右に動く（キー入力検知）
		kJump,   // ジャンプ（キー入力検知）
		kFinish  // 完了（フェードアウトへ）
	};

private:
	// ===== フェード =====
	Fade* fade_ = nullptr;
	float duration_ = 1.5f;

	// ===== フラグ / 状態 =====
	bool isFinished_ = false;
	Phase phase_ = Phase::kFadeIn;
	Step step_ = Step::kMove;

	// ===== プレイヤー =====
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelAttack_ = nullptr;
	Player* player_ = nullptr;

	// ===== マップ =====
	MapChipField* mapChipField_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// ===== ゴール =====
	KamataEngine::Model* modelGoal_ = nullptr;
	KamataEngine::WorldTransform worldTransformGoal_;
	bool hasGoal_ = false;
	static inline const float kGoalWidth = 1.0f;
	static inline const float kGoalHeight = 1.0f;

	// ===== 天球 =====
	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome* skydome_ = nullptr;

	// ===== カメラ =====
	KamataEngine::Camera camera_;
	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	CameraController* cameraController_ = nullptr;

	// ===== UI（説明スプライト） =====
	// 画像は任意：text/tutorial/*.png を想定。無ければ任意のパスに差し替え可
	uint32_t texMove_ = 0;
	uint32_t texJump_ = 0;
	uint32_t texDone_ = 0;

	KamataEngine::Sprite* sprMove_ = nullptr;
	KamataEngine::Sprite* sprJump_ = nullptr;
	KamataEngine::Sprite* sprDone_ = nullptr;

	KamataEngine::Vector2 uiCenter_ = {640.0f, 200.0f};

	///===========================================
	/// 雲（背景）
	/// ===========================================

	// 雲（背景）
	KamataEngine::Model* modelCloud_ = nullptr;
	std::vector<KamataEngine::WorldTransform*> worldTransformClouds_;

private:
	// 補助
	static float Lerp(float a, float b, float t);
	static float Smooth01(float t);

	void SetSpriteAlpha(KamataEngine::Sprite* sp, float a);
	void ShowOnly(KamataEngine::Sprite* target);
	void LoadUI();
	void GenerateBlocks();
	void FindAndPlaceGoal();

	// 入力検知（ゆるめに複数キーを受け付ける）
	bool PressedMove() const;
	bool PressedJump() const;
	bool PressedAttack() const;

	// フェーズ別
	void UpdateFadeIn();
	void UpdateRun();
	void UpdateFadeOut();

public:
	~TutorialScene();
	void Initialize();
	void Update();
	void Draw();
	bool IsFinished() const { return isFinished_; }
};
