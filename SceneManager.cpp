#include "SceneManager.h"
#include "GameScene.h"

// デストラクタ
SceneManager::~SceneManager() {
	// ゲームシーン
	delete gameScene_;
}
// 初期化
void SceneManager::Initialize() { 
	//起動時のシーン
	scene_ = Scene::kGamePlay;
	//ゲームシーンの生成
	gameScene_ = new GameScene();
	//ゲームシーンの初期化
	gameScene_->Initialize();
}
// シーンのチェンジ
void SceneManager::ChangeScene() {
	switch (scene_) { 
	case Scene::kTitle:
	
	break;
	case Scene::kSelect:

		break;
	case Scene::kGamePlay:

		break;
	case Scene::kGameClear:

		break;
	case Scene::kGameOver:

		break;
	}
}
// 更新
void SceneManager::Update() {
	//シーンチェンジの処理
	ChangeScene();

	switch (scene_) {
	case Scene::kTitle:

		break;
	case Scene::kSelect:

		break;
	case Scene::kGamePlay:

		//ゲームシーンの更新
		gameScene_->Update();

		break;
	case Scene::kGameClear:

		break;
	case Scene::kGameOver:

		break;
	}
}
// 描画
void SceneManager::Draw() {
switch (scene_) {
	case Scene::kTitle:

		break;
	case Scene::kSelect:

		break;
	case Scene::kGamePlay:

		//ゲームシーンの描画
		gameScene_->Draw();

		break;
	case Scene::kGameClear:

		break;
	case Scene::kGameOver:

		break;
	}
}