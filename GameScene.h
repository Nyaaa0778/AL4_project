#pragma once
#include "AffineMatrix.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "HitEffect.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"

#include <vector>

class Fireworks;

class GameScene {
public:
	// ゲームのフェーズ
	enum class Phase {
		kFadeIn, // フェードイン
		kPlay,   // ゲームプレイ
		kDeath,  // デス演出
		kClear,  // ゴール演出
		kFadeOut // フェードアウト
	};

	// ゲームの現在フェーズ
	Phase phase_;

private:
	///===========================================
	/// シーン
	/// ===========================================

	// スタートフラグ
	bool isGameStart_ = false;

	// 終了フラグ
	bool isFinished_ = false;
	bool isClear_ = false;

	float startCounter_ = 3.0f;
	static inline const float kStartWaitTime = 0.0f;

	// === カウントダウン/スタート表示 ===
	uint32_t texCount_[5] = {}; // 0〜4（0は別用途で保持）
	KamataEngine::Sprite* sprCount_[5] = {nullptr};
	int countIndex_ = 3; // 3→2→1

	uint32_t texStart_ = 0; // 「スタート！」画像
	KamataEngine::Sprite* sprStart_ = nullptr;

	enum class StartPhase { kCounting, kShowStart, kPlaying };
	StartPhase startPhase_ = StartPhase::kCounting;

	float startTextTimer_ = 0.0f;                       // 「スタート！」の残り時間(秒)
	static inline const float kStartTextTime = 0.7f;    // 表示時間
	KamataEngine::Vector2 countPos_ = {640.0f, 360.0f}; // 画面中央

	// === ゴール ===
	KamataEngine::Model* modelGoal_ = nullptr;        // 見た目（任意）
	KamataEngine::WorldTransform worldTransformGoal_; // 位置・見た目更新用
	bool hasGoal_ = false;                            // 見つかったか
	static inline const float kGoalWidth = 1.0f;      // AABB幅（ブロックと同じ）
	static inline const float kGoalHeight = 1.0f;     // AABB高さ

	// ==== クリア演出 ====
	enum class ClearStep { kSlow, kBannerIn, kShowTime, kFade };
	ClearStep clearStep_ = ClearStep::kSlow;
	float clearTimer_ = 0.0f;

	Fireworks* fireworks_ = nullptr;
	float fireworksTimer_ = 0.0f;
	float nextFirework_ = 0.25f;

	// バナー & ビネット
	uint32_t texClearBanner_ = 0;
	KamataEngine::Sprite* sprClearBanner_ = nullptr;

	uint32_t texVignette_ = 0; // 中央透過・周辺暗いPNGを想定（無ければ黒1x1でもOK）
	KamataEngine::Sprite* sprVignette_ = nullptr;

	// パラメータ（アニメ用）
	float bannerScale_ = 0.1f;
	float bannerAlpha_ = 0.0f;
	float vignetteAlpha_ = 0.0f;

	// 補助（簡易イージング）
	static float Lerp(float a, float b, float t);
	static float Smooth(float t);      // 0→1へスムーズ
	static float EaseOutBack(float t); // ボヨンと出る

	uint32_t texToTitle_ = 0;
	KamataEngine::Sprite* sprToTitle_ = nullptr;

	KamataEngine::Model* modelFireworksParticle_ = nullptr;

	///===========================================
	/// フェード
	/// ===========================================

	// フェード
	Fade* fade_ = nullptr;
	// フェード時間
	float duration_ = 1.0f;

	///===========================================
	/// プレイヤー
	/// ===========================================

	// モデルデータ
	KamataEngine::Model* modelPlayer_ = nullptr;
	// プレイヤー
	Player* player_ = nullptr;

	// 攻撃時のエフェクト
	KamataEngine::Model* modelAttack_ = nullptr;

	///===========================================
	/// 死亡時のパーティクル
	/// ===========================================

	// モデルデータ
	KamataEngine::Model* modelDeathParticle_ = nullptr;
	// パーティクル
	DeathParticles* deathParticles_ = nullptr;

	///===========================================
	/// 敵
	/// ===========================================

	// モデルデータ
	KamataEngine::Model* modelEnemy_ = nullptr;
	// 敵
	std::list<Enemy*> enemies_;

	///===========================================
	/// ヒットエフェクト
	/// ===========================================

	// 攻撃ヒット時のエフェクト
	KamataEngine::Model* modelHitEffect_ = nullptr;
	// ヒットエフェクト
	std::list<HitEffect*> hitEffects_;

	///===========================================
	/// ブロック
	/// ===========================================

	// モデルデータ
	KamataEngine::Model* modelBlock_ = nullptr;
	// ブロック用のWorldTransform(二次配列)
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	///===========================================
	/// 天球
	/// ===========================================

	// モデルデータ
	KamataEngine::Model* modelSkydome_ = nullptr;
	// 天球
	Skydome* skydome_ = nullptr;

	///===========================================
	/// マップチップフィールド
	/// ===========================================

	MapChipField* mapChipField_ = nullptr;

	///===========================================
	/// 雲（背景）
	/// ===========================================

	// 雲（背景）
	KamataEngine::Model* modelCloud_ = nullptr;
	std::vector<KamataEngine::WorldTransform*> worldTransformClouds_;

	///===========================================
	/// カメラ
	/// ===========================================
	KamataEngine::Camera camera_;

	// デバッグカメラ
	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	CameraController* cameraController_ = nullptr;

	// === 下部チュートリアルヒント ===
	uint32_t texHintMove_ = 0;
	uint32_t texHintJump_ = 0;
	uint32_t texHintAttack_ = 0;
	KamataEngine::Sprite* sprHintMove_ = nullptr;
	KamataEngine::Sprite* sprHintJump_ = nullptr;
	KamataEngine::Sprite* sprHintAttack_ = nullptr;
	// 画面下に並べて表示（アンカーを下中央に）
	KamataEngine::Vector2 hintMovePos_ = {200.0f, 100.0f}; // 左側
	KamataEngine::Vector2 hintJumpPos_ = {600.0f, 100.0f}; // 右側
	KamataEngine::Vector2 hintAttackPos_ = {1000.0f, 100.0f}; // 右側

public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();
	/// <summary>
	/// ブロックの初期化
	/// </summary>
	void GenerateBlocks();
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// エフェクト生成
	/// </summary>
	/// <param name="spawnPosition"></param>
	void CreateHitEffect(const KamataEngine::Vector3& spawnPosition);

	/// <summary>
	/// 全ての当たり判定を行う
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// フェードイン中の処理
	/// </summary>
	void UpdateFadeIn();
	/// <summary>
	/// ゲームプレイフェーズの処理
	/// </summary>
	void UpdatePlay();
	/// <summary>
	/// デスフェーズの処理
	/// </summary>
	void UpdateDeath();
	void UpdateClear();
	/// <summary>
	/// フェードアウト中の処理
	/// </summary>
	void UpdateFadeOut();

	/// <summary>
	/// フェーズの切り替え処理
	/// </summary>
	void ChangePhase();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ゲッター
	/// </summary>
	/// <returns></returns>
	bool IsFinished() const;
};
