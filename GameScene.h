#pragma once
#include "KamataEngine.h"

//前方宣言
class Player;
class MapChipField;
class CameraController;

class GameScene {
private:
	/// <summary>
	/// カメラ
	/// </summary>
	KamataEngine::Camera camera_;

	CameraController* cameraController_ = nullptr;

	/// <summary>
	/// プレイヤー
	/// </summary>	
	Player* player_ = nullptr;

	//プレイヤーモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	/// <summary>
	/// マップチップ
	/// </summary>
	MapChipField* mapChipField_ = nullptr;

	// モデルデータ
	KamataEngine::Model* modelBlock_ = nullptr;
	// ブロック用のWorldTransform(二次配列)
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

public:
	// デストラクタ
	~GameScene();

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	/// <summary>
	/// ブロックの初期化
	/// </summary>
	void GenerateBlocks();
};
