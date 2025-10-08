#pragma once
#include "KamataEngine.h"

//前方宣言
class Player;

class GameScene {
private:
	/// <summary>
	/// カメラ
	/// </summary>
	KamataEngine::Camera* camera_ = nullptr;

	/// <summary>
	/// プレイヤー
	/// </summary>	
	Player* player_ = nullptr;

	//プレイヤーモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

public:
	// デストラクタ
	~GameScene();

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update();

	// 描画処理
	void Draw();
};
