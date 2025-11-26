#pragma once
#include <KamataEngine.h>

class MapChipField;

class Player {
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

public:

	const KamataEngine::WorldTransform& GetWorldTransform() const;
	const KamataEngine::Vector3& GetVelocity() const;

	/// <summary>
	/// セッター
	/// </summary>
	/// <param name="mapChipField"></param>
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

private:
	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool isHitCeiling = false;
		bool isHitLanding = false;
		bool isHitWall = false;
		KamataEngine::Vector3 moveAmount;

		int wallDirection = 0; // -1で左、1で右、0でなし
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

	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 加速度定数
	static inline const float kAcceleration = 0.001f;
	// 移動減衰定数
	static inline const float kAttenuation = 0.05f;
	// 速度制限
	static inline const float kMaxSpeed = 3.0f;

	// 接地状態のフラグ
	bool onGround_ = true;
	// 重力加速度定数
	static inline const float kGravityAcceleration = 0.03f;
	// 最大落下速度定数
	static inline const float kMaxFallSpeed = 5.0f;
	// ジャンプ初速度定数
	static inline const float kJumpAcceleration = 0.5f;
	// ジャンプ回数制限
	static inline const int kMaxJumpCount = 2;
	// ジャンプ回数カウント
	int jumpCount_ = 0;

	// 壁蹴りジャンプ
	bool isOnWall_ = false;
	// 左右のどちらの壁か？
	int wallDirection_ = 0;

	static inline const float kWallJumpHorizontalSpeed = 0.4f;
	static inline const float kWallJumpVerticalSpeed = 0.6f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.0005f;
	static inline const float kAttenuationWall = 0.05f;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;
	// キャラクターの当たり判定のサイズ
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.0f;

	// 微妙にずらして判定を取る
	static inline const float kGroundCheckOffset = 0.01f;
	static inline const float kBlank = 0.01f;

private:
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
	/// 接地状態を切り替える処理
	/// </summary>
	/// <param name="info"></param>
	void ChangeGroundState(const CollisionMapInfo& info);

	/// <summary>
	/// 角の位置を取得
	/// </summary>
	/// <param name="center"></param>
	/// <param name="corner"></param>
	/// <returns></returns>
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

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
};
