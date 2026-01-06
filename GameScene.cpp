#define NOMINMAX
#include "GameScene.h"
#include "Fireworks.h"
#include "Random.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// デストラクタ
/// </summary>
GameScene::~GameScene() {
	// プレイヤー
	delete modelPlayer_;
	delete modelAttack_;
	delete player_;

	// 死亡時のパーティクル
	delete modelDeathParticle_;
	delete deathParticles_;

	// 敵
	delete modelEnemy_;
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	// ヒットエフェクト
	delete modelHitEffect_;
	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	hitEffects_.clear();

	// ブロック
	delete modelBlock_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	// ゴール
	delete modelGoal_;

	delete fireworks_;
	fireworks_ = nullptr;

	// 天球
	delete modelSkydome_;
	delete skydome_;

	// マップチップフィールド
	delete mapChipField_;

	// デバッグカメラ
	delete debugCamera_;
	delete cameraController_;

	delete sprClearBanner_;
	sprClearBanner_ = nullptr;

	delete sprVignette_;
	sprVignette_ = nullptr;

	delete sprToTitle_;
	sprToTitle_ = nullptr;

	delete modelCloud_;
	modelCloud_ = nullptr;
	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		delete worldTransformClouds_[i];
	}
	worldTransformClouds_.clear();

	delete sprHintMove_;
	sprHintMove_ = nullptr;
	delete sprHintJump_;
	sprHintJump_ = nullptr;
}

/// <summary>
/// 初期化処理
/// </summary>
void GameScene::Initialize() {
	isGameStart_ = false;
	// 開始タイマー
	startCounter_ = 3.0f;

	///===========================================
	/// フェーズ
	/// ===========================================

	// ゲームプレイフェーズから開始
	phase_ = Phase::kFadeIn;

	///===========================================
	/// フェード
	/// ===========================================

	// フェード画面の生成
	fade_ = new Fade();
	// フェード画面の初期化
	fade_->Initialize();
	// フェード開始
	fade_->Start(Fade::Status::FadeIn, duration_);

	///===========================================
	/// マップチップフィールド
	/// ===========================================

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	///===========================================
	/// プレイヤー
	/// ===========================================

	// 3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelAttack_ = Model::CreateFromOBJ("attackEffect", true);

	// プレイヤーの生成
	player_ = new Player();

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(5, 15);

	// プレイヤーの初期化
	player_->Initialize(modelPlayer_, modelAttack_, &camera_, playerPosition);

	player_->SetMapChipField(mapChipField_);

	///===========================================
	/// 死亡時のパーティクル
	/// ===========================================

	// 3Dモデルの生成
	modelDeathParticle_ = Model::CreateFromOBJ("deathParticle", true);

	///===========================================
	/// 敵
	/// ===========================================

	// 3Dモデルの生成
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);

	for (uint32_t i = 0; i < 3; ++i) {
		// 敵の生成
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(30 + 10 * i, 18);

		// 敵の初期化
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
		newEnemy->SetGameScene(this);
		newEnemy->SetMapChipField(mapChipField_);

		enemies_.push_back(newEnemy);
	}

	///===========================================
	/// ヒットエフェクト
	/// ===========================================

	// 3Dモデルの生成
	modelHitEffect_ = Model::CreateFromOBJ("hitEffect", true);

	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(&camera_);

	///===========================================
	/// ブロック
	/// ===========================================

	GenerateBlocks();

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

	Random::SeedEngine(); // 乱数エンジン初期化

	///===========================================
	/// ゴール
	/// ===========================================

	// === ゴールの検索と配置 ===
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

	///===========================================
	/// ゴール
	/// ===========================================

	modelGoal_ = Model::CreateFromOBJ("goal", true);
	modelFireworksParticle_ = Model::CreateFromOBJ("fireworks", true);

	///===========================================
	/// 天球
	/// ===========================================

	modelSkydome_ = Model::CreateFromOBJ("skyDome", true);

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	texClearBanner_ = TextureManager::Load("text/clear.png");
	sprClearBanner_ = Sprite::Create(texClearBanner_, {640.0f, 240.0f}, {1, 1, 1, 0}, {0.5f, 0.5f});

	bannerScale_ = 1.0f;
	bannerAlpha_ = 0.0f;
	vignetteAlpha_ = 0.0f;
	clearStep_ = ClearStep::kSlow;
	clearTimer_ = 0.0f;

	///===========================================
	/// カメラ
	/// ===========================================

	// カメラ
	camera_.farZ = 2000.0f;
	camera_.Initialize();

	// デバッグカメラ
	debugCamera_ = new DebugCamera(1280, 720);
	debugCamera_->SetFarZ(2000.0f);

	// カメラコントローラの初期化

	// 生成
	cameraController_ = new CameraController();
	cameraController_->SetCamera(&camera_);
	// 初期化
	cameraController_->Initialize();
	cameraController_->SetMovableArea(CameraController::Rect{12, static_cast<float>(mapChipField_->GetNumBlockHorizontal() - 12), 6, 6});
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

	texCount_[0] = TextureManager::Load("text/number/0.png");
	texCount_[1] = TextureManager::Load("text/number/1.png");
	texCount_[2] = TextureManager::Load("text/number/2.png");
	texCount_[3] = TextureManager::Load("text/number/3.png");

	// アンカーを中心にして作る（ズレ防止）
	for (int i = 0; i <= 3; ++i) {
		sprCount_[i] = Sprite::Create(texCount_[i], countPos_, {1, 1, 1, 1}, {0.5f, 0.5f});
	}
	countIndex_ = 3;

	// === スタート画像 ===
	texStart_ = TextureManager::Load("text/startText.png");
	sprStart_ = Sprite::Create(texStart_, countPos_, {1, 1, 1, 1}, {0.5f, 0.5f});

	// 最初はカウント中
	startPhase_ = StartPhase::kCounting;

	texToTitle_ = TextureManager::Load("text/backTitleText.png");
	sprToTitle_ = Sprite::Create(texToTitle_, {640.0f, 520.0f}, {1, 1, 1, 0}, {0.5f, 0.5f});

	// === 下部チュートリアルヒント ===
	texHintMove_ = TextureManager::Load("text/tutorial/move.png"); // 「いどう ← →」
	texHintJump_ = TextureManager::Load("text/tutorial/jump.png"); // 「スペース ジャンプ」
	texHintAttack_ = TextureManager::Load("text/tutorial/attack.png");

	// アンカーを下中央にしておくと、Yの整列が楽
	sprHintMove_ = Sprite::Create(texHintMove_, hintMovePos_, {1, 1, 1, 1}, {0.5f, 1.0f});
	sprHintJump_ = Sprite::Create(texHintJump_, hintJumpPos_, {1, 1, 1, 1}, {0.5f, 1.0f});
	sprHintAttack_ = Sprite::Create(texHintAttack_, hintAttackPos_, {1, 1, 1, 1}, {0.5f, 1.0f});
	// 例：幅360px・高さ72pxくらいに
	sprHintMove_->SetSize({360.0f, 72.0f});
	sprHintJump_->SetSize({400.0f, 72.0f}); // 画像の横幅に合わせて微調整
	sprHintAttack_->SetSize({400.0f, 72.0f});
}

/// <summary>
/// ブロックの初期化
/// </summary>
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

/// <summary>
/// 更新処理
/// </summary>
void GameScene::Update() {
	const float dt = 1.0f / 60.0f;

	if (isGameStart_) {
		// 死亡フラグの立った敵を削除
		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->IsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		hitEffects_.remove_if([](HitEffect* hitEffect) {
			if (hitEffect->IsDead()) {
				delete hitEffect;
				return true;
			}
			return false;
		});
	} else if (startPhase_ == StartPhase::kShowStart) {
		// 「スタート！」表示中
		startTextTimer_ -= dt;
		if (startTextTimer_ <= 0.0f) {
			startPhase_ = StartPhase::kPlaying;
			isGameStart_ = true; // ここでゲーム開始
		}
	} else {
		if (startPhase_ == StartPhase::kCounting) {
			// 3→2→1 表示（0は出さない）
			if (startCounter_ > 0.0f) {
				startCounter_ -= dt;

				int idx = static_cast<int>(startCounter_ + 0.999f); // 3,2,1 に丸め
				if (idx < 1)
					idx = 1;
				if (idx > 3)
					idx = 3;
				countIndex_ = idx;
			} else {
				// 0になったら「スタート！」を表示するフェーズへ
				startPhase_ = StartPhase::kShowStart;
				startTextTimer_ = kStartTextTime;
			}
		} else if (startPhase_ == StartPhase::kShowStart) {
			// 「スタート！」表示中
			startTextTimer_ -= dt;
			if (startTextTimer_ <= 0.0f) {
				startPhase_ = StartPhase::kPlaying;
				isGameStart_ = true; // ここでゲーム開始
			}
		}
	}

	// フェーズの更新
	ChangePhase();
}

/// <summary>
/// エフェクト生成
/// </summary>
/// <param name="spawnPosition"></param>
void GameScene::CreateHitEffect(const Vector3& spawnPosition) {
	HitEffect* newHitEffect = HitEffect::Create(spawnPosition);
	hitEffects_.push_back(newHitEffect);
}

void GameScene::CheckAllCollisions() {
#pragma region プレイヤーと敵の当たり判定
	{
		// 判定対象1と2の座標
		AABB aabb1;
		AABB aabb2;

		// プレイヤーの座標
		aabb1 = player_->GetAABB();

		// プレイヤーと敵の弾全ての当たり判定
		for (Enemy* enemy : enemies_) {
			// 敵の弾の座標
			aabb2 = enemy->GetAABB();

			if (enemy->IsCollisionDisabled()) {
				// 当たり判定無効の敵はスキップ
				continue;
			}

			// AABB同士の交差判定
			if (IsAABBCollision(aabb1, aabb2)) {
				// プレイヤーの衝突時にコールバックを呼び出す
				player_->OnCollision(enemy);
				// 敵の衝突時にコールバックを呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion

#pragma region プレイヤーとゴールの当たり判定
	if (isGameStart_ && hasGoal_) {
		const Vector3 gp = worldTransformGoal_.translation_;
		AABB goal{};
		goal.min = {gp.x - kGoalWidth * 0.5f, gp.y - kGoalHeight * 0.5f, gp.z - kGoalWidth * 0.5f};
		goal.max = {gp.x + kGoalWidth * 0.5f, gp.y + kGoalHeight * 0.5f, gp.z + kGoalWidth * 0.5f};

		if (IsAABBCollision(player_->GetAABB(), goal)) {
			phase_ = Phase::kClear;
			isClear_ = true;

			// クリア演出の初期化
			clearStep_ = ClearStep::kSlow;
			clearTimer_ = 0.0f;
			bannerScale_ = 0.1f;
			bannerAlpha_ = 0.0f;
			vignetteAlpha_ = 0.0f;

			// 透明から開始
			if (sprClearBanner_)
				sprClearBanner_->SetColor({1, 1, 1, 0});
			if (sprVignette_)
				sprVignette_->SetColor({1, 1, 1, 0});

			// 花火生成
			if (!fireworks_) {
				fireworks_ = new Fireworks();
				// 粒モデルは既存の死亡パーティクル用モデルを使い回し可
				fireworks_->Initialize(modelFireworksParticle_, &camera_);
			}
			// まずは3発ボンッと出す（ゴール付近に）
			const Vector3 base = worldTransformGoal_.translation_;
			fireworks_->Burst(base + Vector3{-4, 7, 0}, 90, 3.5f, 7.5f);
			fireworks_->Burst(base + Vector3{0, 9, 0}, 90, 3.5f, 7.5f);
			fireworks_->Burst(base + Vector3{4, 11, 0}, 90, 3.5f, 7.5f);

			// 以後は UpdateClear() で自動的に打ち上げ周期的に出す
			fireworksTimer_ = 0.0f;
			nextFirework_ = 0.25f; // 最初の間隔
		}
	}
#pragma endregion

#pragma region プレイヤーとアイテムの当たり判定
#pragma endregion

#pragma region プレイヤーの弾と敵の当たり判定
#pragma endregion
}

/// <summary>
/// フェーズの切り替え処理
/// </summary>
void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kFadeIn:

		// フェードインの更新
		fade_->Update();

		// フェードイン中の処理
		UpdateFadeIn();

		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
		}
		// break; ← 意図的にフォールスルー？

	case Phase::kPlay:

		// ゲームプレイの更新
		UpdatePlay();

		break;
	case Phase::kDeath:

		// デス演出の更新
		UpdateDeath();

		break;

	case Phase::kClear:

		UpdateClear();

		break;
	case Phase::kFadeOut:
		// フェードアウトの更新
		fade_->Update();

		// フェードアウト中の更新
		UpdateFadeOut();

		if (fade_->IsFinished()) {
			// ゲームシーンの終了
			isFinished_ = true;
		}

		break;
	}
}

/// <summary>
/// フェードイン中の処理
/// </summary>
void GameScene::UpdateFadeIn() {
	///===========================================
	/// ブロック
	/// ===========================================

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

	// ゴールの行列更新（見た目を出すために必須）
	if (hasGoal_) {
		WorldTransformUpdate(worldTransformGoal_);
	}

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		WorldTransformUpdate(*worldTransformClouds_[i]); // ポインタ参照（auto不使用）
	}
}

/// <summary>
/// ゲームプレイフェーズの処理
/// </summary>
void GameScene::UpdatePlay() {
	///===========================================
	/// 天球
	/// ===========================================

	skydome_->Update();

	if (isGameStart_) {
		///===========================================
		/// プレイヤー
		/// ===========================================

		player_->Update();

		if (player_->IsDead()) {
			// 死亡演出フェーズ(デスフェーズ)に切り替え
			phase_ = Phase::kDeath;

			// プレイヤーの座標を取得
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			// デスパーティクルを発生、初期化
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticle_, &camera_, deathParticlesPosition);
		}

		///===========================================
		/// 敵
		/// ===========================================

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		///===========================================
		/// ヒットエフェクト
		/// ===========================================
		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}
	} else {

		player_->UpdateMatricesOnly();
	}

	///===========================================
	/// カメラ
	/// ===========================================

	camera_.matView = debugCamera_->GetCamera().matView;
	camera_.matProjection = debugCamera_->GetCamera().matProjection;

	// ビュープロジェクション行列の更新と転送
	camera_.TransferMatrix();
	cameraController_->Update();

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F)) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#endif

	if (isDebugCameraActive_) {
		// デバッグカメラの更新
		debugCamera_->Update();

		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の更新と転送
		camera_.TransferMatrix();

	} else {

		// ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();
	}

	///===========================================
	/// ブロック
	/// ===========================================

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

	// ゴールの行列更新（見た目を出すために必須）
	if (hasGoal_) {
		WorldTransformUpdate(worldTransformGoal_);
	}

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		WorldTransformUpdate(*worldTransformClouds_[i]); // ポインタ参照（auto不使用）
	}

	if (isGameStart_) {
		// 全ての当たり判定を行う
		CheckAllCollisions();
	}
}

/// <summary>
/// デスフェーズの処理
/// </summary>
void GameScene::UpdateDeath() {
	if (deathParticles_ && deathParticles_->IsFinished()) {
		// フェードアウト開始
		fade_->Start(Fade::Status::FadeOut, duration_);
		phase_ = Phase::kFadeOut;
	}

	///===========================================
	/// 天球
	/// ===========================================

	skydome_->Update();

	///===========================================
	/// 敵
	/// ===========================================

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

	///===========================================
	/// 死亡時のパーティクル
	/// ===========================================

	if (deathParticles_) {
		deathParticles_->Update();
	}

	///===========================================
	/// ブロック
	/// ===========================================

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

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		WorldTransformUpdate(*worldTransformClouds_[i]); // ポインタ参照（auto不使用）
	}

	///===========================================
	/// カメラ
	/// ===========================================

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F)) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#endif

	if (isDebugCameraActive_) {
		// デバッグカメラの更新
		debugCamera_->Update();

		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の更新と転送
		camera_.TransferMatrix();

	} else {
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の更新と転送
		camera_.TransferMatrix();
		cameraController_->Update();
	}
}

void GameScene::UpdateClear() {
	const float dt = 1.0f / 60.0f;

	clearTimer_ += dt;

	// 花火の更新
	if (fireworks_) {
		fireworks_->Update(dt);

		// クリアバナーが出た後（kShowTime 以降）は周期的に打つ
		if (clearStep_ == ClearStep::kShowTime) {
			fireworksTimer_ += dt;
			if (fireworksTimer_ >= nextFirework_) {
				fireworksTimer_ = 0.0f;
				// 次の間隔をちょいランダムに（0.20～0.45秒）
				nextFirework_ = 0.20f + (float)(rand() % 26) / 100.0f; // 簡易乱数

				// ゴール上空の箱にランダム生成
				Vector3 c = worldTransformGoal_.translation_;
				float rx = ((rand() % 100) / 100.0f) * 16.0f - 8.0f; // [-8,8]
				float ry = ((rand() % 100) / 100.0f) * 6.0f + 6.0f;  // [6,12]
				float rz = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;  // [-3,3]

				fireworks_->Burst(c + Vector3{rx, ry, rz}, 80, 3.0f, 7.0f);
			}
		}
	}

	// 背景（天球など）は動いててOK
	skydome_->Update();

	switch (clearStep_) {
	case ClearStep::kSlow: {
		// ビネットをサッと入れる（0.25秒）
		float t = std::min(clearTimer_ / 0.25f, 1.0f);
		float a = Smooth(t);
		vignetteAlpha_ = Lerp(0.0f, 0.55f, a);
		if (sprVignette_)
			sprVignette_->SetColor({1, 1, 1, vignetteAlpha_});

		if (t >= 1.0f) {
			clearStep_ = ClearStep::kBannerIn;
			clearTimer_ = 0.0f;
		}
	} break;

	case ClearStep::kBannerIn: {
		// バナーをボヨンと出す（0.6秒）
		float t = std::min(clearTimer_ / 0.6f, 1.0f);
		float e = EaseOutBack(t);
		bannerScale_ = Lerp(0.2f, 1.0f, e);
		bannerAlpha_ = t;

		if (sprClearBanner_) {
			sprClearBanner_->SetSize({bannerScale_, bannerScale_});
			sprClearBanner_->SetColor({1, 1, 1, bannerAlpha_});
		}

		if (t >= 1.0f) {
			clearStep_ = ClearStep::kShowTime;
			clearTimer_ = 0.0f;
		}
	} break;

	case ClearStep::kShowTime: {
		// 「タイトルへ」の画像を点滅表示（約3Hz）
		if (sprToTitle_) {
			float blink = 0.5f + 0.5f * std::sin(clearTimer_ * 6.0f);
			sprToTitle_->SetColor({1, 1, 1, blink});
		}

		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			fade_->Start(Fade::Status::FadeOut, duration_);
			phase_ = Phase::kFadeOut;
		}
	} break;

	case ClearStep::kFade: {
		// 使わない（FadeOutフェーズに任せる）
	} break;
	}

	// カメラ行列など最低限の更新
	camera_.TransferMatrix();

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		WorldTransformUpdate(*worldTransformClouds_[i]);
	}
}

/// <summary>
/// フェードアウト中の処理
/// </summary>
void GameScene::UpdateFadeOut() {
	///===========================================
	/// 天球
	/// ===========================================

	skydome_->Update();

	///===========================================
	/// 敵
	/// ===========================================

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

	///===========================================
	/// 死亡時のパーティクル
	/// ===========================================

	if (deathParticles_) {
		deathParticles_->Update();
	}

	///===========================================
	/// ブロック
	/// ===========================================

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

	///===========================================
	/// カメラ
	/// ===========================================

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F)) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#endif

	if (isDebugCameraActive_) {
		// デバッグカメラの更新
		debugCamera_->Update();

		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の更新と転送
		camera_.TransferMatrix();

	} else {
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の更新と転送
		camera_.TransferMatrix();
		cameraController_->Update();
	}

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		WorldTransformUpdate(*worldTransformClouds_[i]);
	}
}

/// <summary>
/// 描画処理
/// </summary>
void GameScene::Draw() {

	// モデルの描画前処理
	Model::PreDraw();

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

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

	if (hasGoal_) {
		modelGoal_->Draw(worldTransformGoal_, camera_);
	}

	skydome_->Draw();

	for (size_t i = 0; i < worldTransformClouds_.size(); ++i) {
		modelCloud_->Draw(*worldTransformClouds_[i], camera_);
	}

	// プレイヤーの描画
	player_->Draw();

	// 死亡時のパーティクルの描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// ヒットエフェクトの描画
	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	// 花火
	if ((phase_ == Phase::kClear || phase_ == Phase::kFadeOut) && fireworks_) {
		fireworks_->Draw();
	}

	// モデルの描画後処理
	Model::PostDraw();

	// ========================
	// カウントダウン/スタート描画
	// ========================
	Sprite::PreDraw();

	if (sprHintMove_) {
		sprHintMove_->Draw();
	}
	if (sprHintJump_) {
		sprHintJump_->Draw();
	}
	if (sprHintAttack_) {
		sprHintAttack_->Draw();
	}

	if (!isGameStart_) {
		if (startPhase_ == StartPhase::kCounting) {
			if (sprCount_[countIndex_]) {
				sprCount_[countIndex_]->Draw();
			}
		} else if (startPhase_ == StartPhase::kShowStart) {
			if (sprStart_) {
				sprStart_->Draw();
			}
		}
	}

	if (phase_ == Phase::kClear || phase_ == Phase::kFadeOut) {
		if (sprVignette_)
			sprVignette_->Draw();
		if (sprClearBanner_)
			sprClearBanner_->Draw();
	}

	if (sprToTitle_) {
		sprToTitle_->Draw();
	}

	Sprite::PostDraw();

	// フェードの描画
	fade_->Draw();
}

float GameScene::Lerp(float a, float b, float t) { return a + (b - a) * t; }
float GameScene::Smooth(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}
float GameScene::EaseOutBack(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	float x = t - 1.0f;
	return 1.0f + c3 * (x * x * x) + c1 * (x * x);
}

/// <summary>
/// ゲッター
/// </summary>
/// <returns></returns>
bool GameScene::IsFinished() const { return isFinished_; }
