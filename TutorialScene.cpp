#define NOMINMAX
#include "TutorialScene.h"
#include "AABB.h"
#include "WorldTransformUpdater.h"
#include "Random.h"

using namespace KamataEngine;

// ===== ユーティリティ =====
float TutorialScene::Lerp(float a, float b, float t) { return a + (b - a) * t; }
float TutorialScene::Smooth01(float t) {
	if (t < 0.0f)
		t = 0.0f;
	else if (t > 1.0f)
		t = 1.0f;
	return t * t * (3.0f - 2.0f * t);
}
void TutorialScene::SetSpriteAlpha(Sprite* sp, float a) {
	if (!sp)
		return;
	sp->SetColor({1, 1, 1, a});
}
void TutorialScene::ShowOnly(Sprite* target) {
	SetSpriteAlpha(sprMove_, (target == sprMove_) ? 1.0f : 0.0f);
	SetSpriteAlpha(sprJump_, (target == sprJump_) ? 1.0f : 0.0f);
	SetSpriteAlpha(sprDone_, (target == sprDone_) ? 1.0f : 0.0f);
}

// ===== UIロード =====
void TutorialScene::LoadUI() {
	texMove_ = TextureManager::Load("text/tutorial/move.png");     // 「← → で移動」
	texJump_ = TextureManager::Load("text/tutorial/jump.png");     // 「スペースでジャンプ」
	texDone_ = TextureManager::Load("text/tutorial/done.png");     // 「完了！Enterで進む」

	sprMove_ = Sprite::Create(texMove_, uiCenter_, {1, 1, 1, 1}, {0.5f, 0.5f});
	sprJump_ = Sprite::Create(texJump_, uiCenter_, {1, 1, 1, 0}, {0.5f, 0.5f});
	sprDone_ = Sprite::Create(texDone_, uiCenter_, {1, 1, 1, 0}, {0.5f, 0.5f});
}

// ===== ブロック生成（GameSceneと同じ流儀） =====
void TutorialScene::GenerateBlocks() {
	modelBlock_ = Model::CreateFromOBJ("block", true);

	uint32_t v = mapChipField_->GetNumBlockVirtical();
	uint32_t h = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(v);
	for (uint32_t i = 0; i < v; ++i) {
		worldTransformBlocks_[i].resize(h);
	}

	for (uint32_t i = 0; i < v; ++i) {
		for (uint32_t j = 0; j < h; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = wt;
			}
		}
	}
}

// ===== ゴール検索（GameSceneの要点を踏襲） =====
void TutorialScene::FindAndPlaceGoal() {
	hasGoal_ = false;

	for (uint32_t vi = 0; vi < mapChipField_->GetNumBlockVirtical(); ++vi) {
		for (uint32_t hj = 0; hj < mapChipField_->GetNumBlockHorizontal(); ++hj) {
			if (mapChipField_->GetMapChipTypeByIndex(hj, vi) == MapChipType::kGoal) {
				worldTransformGoal_.Initialize();
				worldTransformGoal_.translation_ = mapChipField_->GetMapChipPositionByIndex(hj, vi);
				hasGoal_ = true;
				break;
			}
		}
		if (hasGoal_)
			break;
	}

	modelGoal_ = Model::CreateFromOBJ("goal", true);
}

// ===== 入力アクション検知（ゆるめ） =====
bool TutorialScene::PressedMove() const {
	auto* in = Input::GetInstance();
	return in->TriggerKey(DIK_LEFT) || in->TriggerKey(DIK_RIGHT) || in->TriggerKey(DIK_A) || in->TriggerKey(DIK_D);
}
bool TutorialScene::PressedJump() const {
	auto* in = Input::GetInstance();
	return in->TriggerKey(DIK_SPACE) || in->TriggerKey(DIK_UP) || in->TriggerKey(DIK_Z);
}
bool TutorialScene::PressedAttack() const {
	auto* in = Input::GetInstance();
	return in->TriggerKey(DIK_X) || in->TriggerKey(DIK_J);
}

// ===== ライフサイクル =====
TutorialScene::~TutorialScene() {
	// フェード
	delete fade_;

	// プレイヤー関連
	delete modelPlayer_;
	delete modelAttack_;
	delete player_;

	// マップ/ブロック
	delete modelBlock_;
	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();

	// ゴール
	delete modelGoal_;

	// 天球
	delete modelSkydome_;
	delete skydome_;

	// マップ
	delete mapChipField_;

	// カメラ
	delete debugCamera_;
	delete cameraController_;

	// UI
	delete sprMove_;
	delete sprJump_;
	delete sprDone_;

	delete modelCloud_;
	modelCloud_ = nullptr;
	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		delete worldTransformClouds_[i];
	}
	worldTransformClouds_.clear();
}

void TutorialScene::Initialize() {
	// ===== フェード =====
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, duration_);

	// ===== マップ =====
	mapChipField_ = new MapChipField;
	// チュートリアル用CSVに差し替えてOK（無ければ blocks.csv で可）
	mapChipField_->LoadMapChipCsv("Resources/tutorialBlocks.csv");

	// ===== プレイヤー =====
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelAttack_ = Model::CreateFromOBJ("attackEffect", true);

	player_ = new Player();
	// 開始位置（例：インデックス 5,15）。必要なら差し替え
	Vector3 playerPos = mapChipField_->GetMapChipPositionByIndex(5, 15);
	player_->Initialize(modelPlayer_, modelAttack_, &camera_, playerPos);
	player_->SetMapChipField(mapChipField_);

	// ===== ゴール =====
	FindAndPlaceGoal();

	// ===== ブロック =====
	GenerateBlocks();

	// ===== 天球 =====
	modelSkydome_ = Model::CreateFromOBJ("skyDome", true);
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	// ===== カメラ =====
	camera_.farZ = 2000.0f;
	camera_.Initialize();

	debugCamera_ = new DebugCamera(1280, 720);
	debugCamera_->SetFarZ(2000.0f);

	cameraController_ = new CameraController();
	cameraController_->SetCamera(&camera_);
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);
	cameraController_->SetMovableArea(CameraController::Rect{12, static_cast<float>(mapChipField_->GetNumBlockHorizontal() - 12), 6, 6});
	cameraController_->Reset();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

	///===========================================
	/// 雲（背景）
	///===========================================
	modelCloud_ = Model::CreateFromOBJ("cloud", true);

	// 乱数エンジン初期化（毎回同じ配置で良いなら省略可）
	Random::SeedEngine();

	// マップの横幅から X 範囲を決める（左右に少し余白）
	uint32_t mapW = mapChipField_->GetNumBlockHorizontal();
	Vector3 leftPos = mapChipField_->GetMapChipPositionByIndex(0, 0);
	Vector3 rightPos = mapChipField_->GetMapChipPositionByIndex(mapW > 0 ? (mapW - 1) : 0, 0);

	float xMin = leftPos.x - 10.0f;
	float xMax = rightPos.x + 10.0f;

	// Y・Z の範囲（お好みで調整）
	// Zは “奥” 側に置く範囲です。もし前後が逆に見えるなら zMin/zMax の符号を反転してください。
	float yMin = 8.0f, yMax = 16.0f;
	float zMin = 12.0f, zMax = 26.0f;

	// スケールの範囲（大小のバラつき）
	float sMin = 1.0f, sMax = 1.5f;

	// 生成数（好きなだけ増やせます）
	const int cloudCount = 12;

	for (int i = 0; i < cloudCount; ++i) {
		WorldTransform* wt = new WorldTransform();
		wt->Initialize();

		// ランダム座標とスケール（Random::GeneraterFloat(min, max) を使用）
		float x = Random::GeneraterFloat(xMin, xMax);
		float y = Random::GeneraterFloat(yMin, yMax);
		float z = Random::GeneraterFloat(zMin, zMax);
		float s = Random::GeneraterFloat(sMin, sMax);

		wt->translation_ = {x, y, z};
		wt->scale_ = {s, s, 1.0f};

		worldTransformClouds_.push_back(wt);
	}

	// （必要ならスケール設定も同様に：wt0->scale_ = {8,8,1}; など）

	Random::SeedEngine(); // 乱数エンジン初期化

	// ===== UI =====
	LoadUI();
	ShowOnly(sprMove_); // 最初は移動ヒント
}

void TutorialScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		UpdateFadeIn();

	case Phase::kRun:
		UpdateRun();
		break;

	case Phase::kFadeOut:
		UpdateFadeOut();
		break;
	}

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		WorldTransformUpdate(*worldTransformClouds_[i]); // ポインタ参照（auto不使用）
	}
}

// ---- フェーズ別 ----
void TutorialScene::UpdateFadeIn() {
	fade_->Update();
	// 背景の最低限の行列更新
	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			if (!wt)
				continue;
			WorldTransformUpdate(*wt);
		}
	}
	if (hasGoal_) {
		WorldTransformUpdate(worldTransformGoal_);
	}
	if (fade_->IsFinished()) {
		phase_ = Phase::kRun;
		step_ = Step::kMove;
		ShowOnly(sprMove_);
	}
}

void TutorialScene::UpdateRun() {
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	// ===== プレイヤー/カメラ =====
	player_->Update();

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		cameraController_->Update();
		camera_.UpdateMatrix();
	}

	// ===== 天球 =====
	skydome_->Update();

	// ===== ブロック行列更新 =====
	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			if (!wt)
				continue;
			WorldTransformUpdate(*wt);
		}
	}
	if (hasGoal_) {
		WorldTransformUpdate(worldTransformGoal_);
	}

	// ===== ステップ進行 =====
	switch (step_) {
	case Step::kMove:
		if (PressedMove()) {
			step_ = Step::kJump;
			ShowOnly(sprJump_);
		}
		break;

	case Step::kJump:
		if (PressedJump()) {
			step_ = Step::kFinish;
			ShowOnly(sprDone_);
		}
		break;
	case Step::kFinish:
		// Enterで終了（タイトル/ゲームへ遷移は外側の管理に合わせて）
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			fade_->Start(Fade::Status::FadeOut, duration_);
			phase_ = Phase::kFadeOut;
		}
		break;
	}
}

void TutorialScene::UpdateFadeOut() {
	fade_->Update();

	// 最低限の更新（見た目維持）
	skydome_->Update();
	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			if (!wt)
				continue;
			WorldTransformUpdate(*wt);
		}
	}
	if (hasGoal_) {
		WorldTransformUpdate(worldTransformGoal_);
	}

	if (fade_->IsFinished()) {
		isFinished_ = true;
	}
}

// ===== 描画 =====
void TutorialScene::Draw() {
	// ----- 3D -----
	Model::PreDraw();

	// ブロック
	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			if (!wt)
				continue;
			modelBlock_->Draw(*wt, camera_);
		}
	}

	// ゴール
	if (hasGoal_) {
		modelGoal_->Draw(worldTransformGoal_, camera_);
	}

	// 天球
	skydome_->Draw();

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		modelCloud_->Draw(*worldTransformClouds_[i], camera_);
	}

	// プレイヤー
	player_->Draw();

	Model::PostDraw();

	// ----- 2D(UI) -----
	Sprite::PreDraw();
	if (sprMove_)
		sprMove_->Draw();
	if (sprJump_)
		sprJump_->Draw();
	if (sprDone_)
		sprDone_->Draw();
	Sprite::PostDraw();

	// フェード
	fade_->Draw();
}
