#pragma once
#include "AABB.h"
#include "KamataEngine.h"

class Player;
class GameScene;
class MapChipField;

class Enemy {
public:
	enum class Behavior {
		kWalk,  // 歩行
		kDeath, // 死亡演出

		kUnknown // 変更リクエストなし
	};

	// プレイヤーと同じ左右向き（見た目の向き制御に使う）
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 壁ヒット後の「待ち → 旋回 → 反対へ歩行」
	enum class TurnState {
		kWalk,
		kWait,
		kTurn,
	};

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// マップとの当たり判定情報（Playerを参考に）
	struct CollisionMapInfo {
		bool isHitWall = false;
		KamataEngine::Vector3 moveAmount;
		int wallDirection = 0; // -1: 左の壁, 1: 右の壁, 0: なし
	};

	// 角（Playerと同じ並び）
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// 振るまい
	Behavior behavior_ = Behavior::kWalk;
	// 次の振るまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.03f;
	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 左右向き（見た目用）
	LRDirection lrDirection_ = LRDirection::kLeft;

	// 壁ヒット後の旋回シーケンス
	TurnState turnState_ = TurnState::kWalk;
	LRDirection nextDirection_ = LRDirection::kLeft;
	float waitTurnTimer_ = 0.0f;
	float turnTimer_ = 0.0f;
	float turnFirstRotationY_ = 0.0f;

	// 壁に当たってから旋回を始めるまでの待ち時間
	static inline const float kWaitBeforeTurn = 0.15f;
	// 旋回にかける時間（Playerの kTimeTurn 相当）
	static inline const float kTimeTurn = 0.20f;

	// 死亡フラグ
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;
	// 死亡アニメーションタイマー
	float deathAnimetionTimer_ = 0.0f;
	static inline const float kDeathAnimetionTime = 0.5f;

	// 最初の角度
	static inline const float kWalkMotionAngleStart = 0.0f;
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = 15.0f;
	// アニメーションの周期となる時間
	static inline const float kWalkMotionTime = 1.0f;
	// 経過時間
	float walkTimer_ = 0.0f;

	// 敵のサイズ
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.0f;
	// 微妙にずらして判定を取る（Playerと同様）
	static inline const float kBlank = 0.01f;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	GameScene* gameScene_ = nullptr;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 歩行状態の初期化
	/// </summary>
	void BehaviorWalkInitialize();
	/// <summary>
	/// 死亡状態の初期化
	/// </summary>
	void BehaviorDeathInitialize();

	/// <summary>
	/// 歩行状態の更新
	/// </summary>
	void BehaviorWalkUpdate();
	/// <summary>
	/// 死亡状態の更新
	/// </summary>
	void BehaviorDeathUpdate();

	// 角の位置を取得
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// マップ当たり判定（左右だけ）
	void IsMapCollision(CollisionMapInfo& info);
	void IsRightMapCollision(CollisionMapInfo& info);
	void IsLeftMapCollision(CollisionMapInfo& info);

	// 当たり判定の結果を反映させて移動
	void MoveByCollisionResult(const CollisionMapInfo& info);
	// 壁に当たったときの処理（反転）
	void ReactToWallHit(const CollisionMapInfo& info);

	// 旋回用のイージング（Playerと同じ）
	float EaseInOutSine(float turnProgress);

	/// <summary>
	/// 衝突応答
	/// </summary>
	/// <param name="player"></param>
	void OnCollision(const Player* player);

	/// <summary>
	/// イージング
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	float EaseInOut(float start, float end, float t);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ゲッター
	/// </summary>
	/// <returns></returns>
	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();
	bool IsDead() const;
	bool IsCollisionDisabled() const;

	/// <summary>
	/// セッター
	/// </summary>
	/// <param name="gameScene"></param>
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	/// <summary>
	/// セッター
	/// </summary>
	/// <param name="mapChipField"></param>
	void SetMapChipField(MapChipField* mapChipField);
};
