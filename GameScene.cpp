#include "GameScene.h"
#include "Player.h"

using namespace KamataEngine;

// デストラクタ
GameScene::~GameScene() {
	/// <summary>
	/// プレイヤー
	/// </summary>
	delete player_;
	// モデル解放
	delete modelPlayer_;
}

// 初期化処理
void GameScene::Initialize() {
	/// <summary>
	/// カメラ
	/// </summary>

	camera_.Initialize();

	/// <summary>
	/// プレイヤー
	/// </summary>

	// 生成
	player_ = new Player();
	// モデル生成
	modelPlayer_ = Model::CreateFromOBJ("cube", true);
	// 初期化
	player_->Initialize(modelPlayer_, &camera_, Vector3{0, 0, 0});
}

// 更新処理
void GameScene::Update() { player_->Update(); }

// 描画処理
void GameScene::Draw() {
	// モデルの描画前処理
	Model::PreDraw();

	/// <summary>
	/// プレイヤー
	/// </summary>
	
	player_->Draw();

	// モデルの描画後処理
	Model::PostDraw();
}