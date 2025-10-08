#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	KamataEngine::Initialize(L"LE2B_27_ヤマダ_ナオ_AL4");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		//ゲームシーンの更新処理
		gameScene->Update();

		//描画前処理
		dxCommon->PreDraw();

		// 描画処理
		gameScene->Draw();

		//描画後処理
		dxCommon->PostDraw();
	}

	KamataEngine::Finalize();

	delete gameScene;
	gameScene = nullptr;

	return 0;
}
