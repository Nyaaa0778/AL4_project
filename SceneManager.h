#pragma once

// 前方宣言
class GameScene;

class SceneManager {
private:
	enum class Scene {
		kUnknown = 0,
		kTitle,    // タイトル
		kSelect,  // 選択
		kGamePlay, // ゲームプレイ
		kGameClear,    // ゲームクリア
		kGameOver, // ゲームオーバー
	};

	// 現在のシーン
	Scene scene_ = Scene::kUnknown;

	GameScene* gameScene_ = nullptr;

public:
	//デストラクタ
	~SceneManager();
	//初期化
	void Initialize();
	// シーンのチェンジ
	void ChangeScene();
	//更新
	void Update();
	//描画
	void Draw();
	
};
