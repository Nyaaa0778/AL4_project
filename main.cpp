#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"
#include "TutorialScene.h"
#include <Windows.h>

using namespace KamataEngine;

GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
TutorialScene* tutorialScene = nullptr;

enum class Scene {
	kUnknown = 0,

	kTitle,
	kTutorial,
	kGame
};

// 現在のシーン
Scene scene = Scene::kUnknown;

/// <summary>
/// シーンの切り替え処理
/// </summary>
void ChangeScene();
/// <summary>
/// シーンの更新
/// </summary>
void UpdateScene();
/// <summary>
/// シーンの描画
/// </summary>
void DrawScene();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	KamataEngine::Initialize(L"LE2B_28_ヤマダ_ナオ_なりそこね");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 最初のシーンの初期化
	scene = Scene::kTitle;

	titleScene = new TitleScene;
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// シーン切り替え
		ChangeScene();
		// 現在シーンの更新
		UpdateScene();

		// 描画前処理
		dxCommon->PreDraw();

		// 現在のシーンを描画
		DrawScene();

		AxisIndicator::GetInstance()->Draw();

		// 描画後処理
		dxCommon->PostDraw();
	}

	KamataEngine::Finalize();

	delete tutorialScene;
	delete titleScene;
	delete gameScene;

	return 0;
}

/// <summary>
/// シーンの切り替え処理
/// </summary>
void ChangeScene() {
	switch (scene) {
	case Scene::kTitle: {
		auto* in = Input::GetInstance();

		// --- main内だけで選択を完了させるホットキー ---
		// 1 or G -> 本編 / 2 or T -> チュートリアル
		if (in->TriggerKey(DIK_1) || in->TriggerKey(DIK_G)) {
			scene = Scene::kGame;

			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
			break;
		}
		if (in->TriggerKey(DIK_2) || in->TriggerKey(DIK_T)) {
			scene = Scene::kTutorial;

			delete titleScene;
			titleScene = nullptr;
			tutorialScene = new TutorialScene;
			tutorialScene->Initialize();
			break;
		}

		// --- 既存の TitleScene 側の終了判定にも対応（従来互換）---
		if (titleScene && titleScene->IsFinished()) {
			// 何も指定が無ければデフォルトは本編へ
			scene = Scene::kGame;

			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
		}
	} break;

	case Scene::kTutorial:
		if (tutorialScene && tutorialScene->IsFinished()) {
			// チュートリアル終了後はタイトルへ戻す
			scene = Scene::kTitle;

			delete tutorialScene;
			tutorialScene = nullptr;
			titleScene = new TitleScene;
			titleScene->Initialize();

			// ※もしチュートリアル直後に本編へ進めたいなら、上の4行を以下に置き換え:
			// scene = Scene::kGame;
			// delete tutorialScene; tutorialScene = nullptr;
			// gameScene = new GameScene; gameScene->Initialize();
		}
		break;

	case Scene::kGame:
		if (gameScene && gameScene->IsFinished()) {
			// 本編終了 → タイトルへ
			scene = Scene::kTitle;

			delete gameScene;
			gameScene = nullptr;
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;

	default:
		break;
	}
}

/// <summary>
/// シーンの更新
/// </summary>
void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) {
			titleScene->Update();
		}
		break;
	case Scene::kTutorial:
		if (tutorialScene) {
			tutorialScene->Update();
		}
		break;
	case Scene::kGame:
		if (gameScene) {
			gameScene->Update();
		}
		break;
	default:
		break;
	}
}

/// <summary>
/// シーンの描画
/// </summary>
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) {
			titleScene->Draw();
		}
		break;
	case Scene::kTutorial:
		if (tutorialScene) {
			tutorialScene->Draw();
		}
		break;
	case Scene::kGame:
		if (gameScene) {
			gameScene->Draw();
		}
		break;
	default:
		break;
	}
}
