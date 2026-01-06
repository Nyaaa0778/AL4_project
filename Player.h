#pragma once
#include "AABB.h"
//#include "AffineMatrix.h"
#include "KamataEngine.h"
#include "WorldTransformUpdater.h"

class MapChipField;
class Enemy;

class Player {
public:
	// プレイヤーが向いている方向.
	enum class LRDirection { kRight, kLeft };

	// 振るまい
	enum class Behavior {
		kRoot,   // 通常状態
		kAttack, // 攻撃状態

		kUnknown // 変更リクエストなし
	};

	enum class AttackPhase {
		kCharge,  // 溜め
		kDash,    // 突進
		kCoolDown // 余韻
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool isHitCeiling = false;
		bool isHitLanding = false;
		bool isHitWall = false;
		KamataEngine::Vector3 moveAmount;
		int wallDirection = 0; // -1: 左の壁, 1: 右の壁, 0: なし
	};

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// 振るまい
	Behavior behavior_ = Behavior::kRoot;
	// 次の振るまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 加速度定数
	static inline const float kAcceleration = 0.0025f;
	// 移動減衰定数
	static inline const float kAttenuation = 0.05f;
	// 速度制限
	static inline const float kMaxSpeed = 3.0f;

	// 死亡フラグ
	bool isDead_ = false;

	// プレイヤーが向いている方向
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回開始タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;
	// 重力加速度(下が正)
	static inline const float kGravityAcceleration = 0.03f;
	// 最大落下速度(下が正)
	static inline const float kMaxFallSpeed = 5.0f;
	// ジャンプの初速度(上が正)
	static inline const float kJumpAcceleration = 0.5f;
	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.0005f;
	static inline const float kAttenuationWall = 0.05f;

	// 空中で使えるジャンプ回数（※地上ジャンプとは別カウント）
	static inline const int kMaxJumpCount = 2;
	int jumpCount_ = 0;

	// 壁ジャンプ用
	bool isOnWall_ = false;
	int wallDirection_ = 0; // -1: 左壁, 1: 右壁, 0: なし
	static inline const float kWallJumpHorizontalSpeed = 0.4f;
	static inline const float kWallJumpVerticalSpeed = 0.6f;

	// 攻撃ギミックの経過時間カウンター
	float attackParameter_ = 0.0f;
	// 攻撃時間
	static inline const float kAttackTime = 0.3f;
	KamataEngine::Vector3 attackVelocity = {0.5f, 0.0f, 0.0f};

	// 現在の攻撃フェーズ
	AttackPhase attackPhase_;
	// 溜め動作時間
	static inline const float kChargeTime = 0.08f;
	// 突進動作時間
	static inline const float kDashTime = 0.08f;
	// 余韻動作時間
	static inline const float kCoolDownTime = 0.04f;

	// 攻撃エフェクトモデル
	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::WorldTransform worldTransformAttack_;

	// 攻撃エフェクトの可視化フラグ
	bool isAttackEffect_ = false;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;
	// キャラクターの当たり判定のサイズ
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.0f;

	// 微妙にずらして判定を取る
	static inline const float kGroundCheckOffset = 0.01f;
	static inline const float kBlank = 0.01f;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// モデル
	KamataEngine::Model* model_ = nullptr;

public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 移動入力
	/// </summary>
	void UpdateMovementInput();

	/// <summary>
	/// 通常行動の初期化
	/// </summary>
	void BehaviorRootInitialize();
	/// <summary>
	/// 攻撃行動の初期化
	/// </summary>
	void BehaviorAttackInitialize();

	/// <summary>
	/// 通常行動の更新
	/// </summary>
	void BehaviorRootUpdate();
	/// <summary>
	/// 攻撃行動の更新
	/// </summary>
	void BehaviorAttackUpdate();

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="info"></param>
	void IsMapCollision(CollisionMapInfo& info);
	/// <summary>
	/// 上方向の当たり判定
	/// </summary>
	/// <param name="info"></param>
	void IsTopMapCollision(CollisionMapInfo& info);
	/// <summary>
	/// 下方向の当たり判定
	/// </summary>
	/// <param name="info"></param>
	void IsBottomMapCollision(CollisionMapInfo& info);
	/// <summary>
	/// 右方向の当たり判定
	/// </summary>
	/// <param name="info"></param>
	void IsRightMapCollision(CollisionMapInfo& info);
	/// <summary>
	/// 左方向の当たり判定
	/// </summary>
	/// <param name="info"></param>
	void IsLeftMapCollision(CollisionMapInfo& info);

	/// <summary>
	/// 衝突応答
	/// </summary>
	/// <param name="enemy"></param>
	void OnCollision(const Enemy* enemy);

	/// <summary>
	/// 角の位置を取得
	/// </summary>
	/// <param name="center"></param>
	/// <param name="corner"></param>
	/// <returns></returns>
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);
	/// <summary>
	/// 接地状態を切り替える処理
	/// </summary>
	/// <param name="info"></param>
	void ChangeGroundState(const CollisionMapInfo& info);
	/// <summary>
	/// 天井に当たったときの処理
	/// </summary>
	/// <param name="info"></param>
	void ReactToCeilingHit(const CollisionMapInfo& info);
	/// <summary>
	/// 壁に接触しているときの処理
	/// </summary>
	/// <param name="info"></param>
	void ReactToWallHit(const CollisionMapInfo& info);
	/// <summary>
	/// 当たり判定の結果を反映させて移動
	/// </summary>
	/// <param name="info"></param>
	void MoveByCollisionResult(const CollisionMapInfo& info);

	void UpdateMatricesOnly();

	/// <summary>
	/// イージング
	/// </summary>
	/// <param name="t"></param>
	/// <returns></returns>
	float EaseInOutSine(float t);
	float EaseOut(float start, float end, float t);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ゲッター
	/// </summary>
	/// <returns></returns>
	const KamataEngine::WorldTransform& GetWorldTransform() const;
	const KamataEngine::Vector3& GetVelocity() const;
	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();
	bool IsDead() const;
	bool IsAttack() const;

	/// <summary>
	/// セッター
	/// </summary>
	/// <param name="mapChipField"></param>
	void SetMapChipField(MapChipField* mapChipField);
};
