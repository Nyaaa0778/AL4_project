#include "GameScene.h"
#include "Player.h"
#include"MapChipField.h"
#include "WorldTransformUpdate.h"
#include"CameraController.h"

using namespace KamataEngine;

// デストラクタ
GameScene::~GameScene() {
	/// 
	/// プレイヤー
	/// 
	
	delete player_;
	// モデル解放
	delete modelPlayer_;

	///
	/// ブロック
	/// 
	
	//モデル解放
	delete modelBlock_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	// マップチップフィールド
	delete mapChipField_;
}

// 初期化処理
void GameScene::Initialize() {
	///
	/// ブロック
	/// 

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	GenerateBlocks();

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
	modelPlayer_ = Model::CreateFromOBJ("block", true);

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);

	// 初期化
	player_->Initialize(modelPlayer_, &camera_, playerPosition);

	player_->SetMapChipField(mapChipField_);

	// カメラコントローラの初期化

	// 生成
	cameraController_ = new CameraController();
	cameraController_->SetCamera(&camera_);
	// 初期化
	cameraController_->Initialize();
	cameraController_->SetMovableArea(CameraController::Rect{12, static_cast<float>(mapChipField_->GetNumBlockHorizontal() - 12), 6, 6});
	cameraController_->SetTarget(player_);
	cameraController_->Reset();
}

// 更新処理
void GameScene::Update() {

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		Initialize();
	}

	///
	/// プレイヤー
	/// 
	
	player_->Update();

	///
	/// ブロック
	///

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {

			// nullptrならスキップ
			if (!worldTransformBlock) {
				continue;
			}

			WorldTransformUpdate(*worldTransformBlock);
		}
	}

	cameraController_->Update();

		// ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();

}

// 描画処理
void GameScene::Draw() {
	// モデルの描画前処理
	Model::PreDraw();

	/// <summary>
	/// プレイヤー
	/// </summary>

	player_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {

			// nullptrならスキップ
			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// モデルの描画後処理
	Model::PostDraw();
}

void GameScene::GenerateBlocks() {

	// ブロックモデルを生成
	modelBlock_ = Model::CreateFromOBJ("block", true);

	// 要素数
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更(縦のブロック数)
	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		// 1行の要素数を設定(横のブロック数)
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// キューブを生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}