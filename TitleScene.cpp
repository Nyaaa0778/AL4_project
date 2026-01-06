#define NOMINMAX

#include "TitleScene.h"
#include "WorldTransformUpdater.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

/// <summary>
/// デストラクタ
/// </summary>
TitleScene::~TitleScene() {
	// フェード
	delete fade_;

	// モデル
	delete model_;
	delete modelTitleLogo_;
	// カメラ
	delete camera_;

	delete modelSkydome_;
	delete skydome_;

	delete modelTitleCloud_;

	delete spriteStartText_;
	delete spriteTutorialText_;
}

/// <summary>
/// 初期化
/// </summary>
void TitleScene::Initialize() {
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
	/// テキスト
	/// ===========================================

	// 3Dモデルの生成
	model_ = Model::CreateFromOBJ("text", true);
	modelTitleLogo_ = Model::CreateFromOBJ("titleLogo", true);

	// アニメーションの経過時間
	elapsedTimer_ = 0.0f;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 6.5f, 0.0f};

	// ロゴ用ワールド変換
	worldTransformTitleLogo_.Initialize();
	worldTransformTitleLogo_.translation_ = {0.0f, 1.5f, 0.0f};
	// ※サイズが小さければここを調整
	worldTransformTitleLogo_.scale_ = {2.0f, 2.0f, 2.0f};

	///===========================================
	/// カメラ
	/// ===========================================

	// カメラの初期化
	camera_ = new Camera();
	camera_->farZ = 2000.0f; // ← 追加（描画範囲を広めに）
	camera_->Initialize();

	modelTitleCloud_ = Model::CreateFromOBJ("titleCloud", true); // titleCloud.obj を読み込む
	worldTransformTitleCloud_.Initialize();

	// ロゴの少し後ろに1個だけ、やや大きめで
	worldTransformTitleCloud_.translation_ = {0.0f, 6.5f, 16.0f};
	// 　　　　　　　　　　　　　　　       ↑ 奥行き（+Zが奥側なら正、逆なら -16.0f に）
	worldTransformTitleCloud_.scale_ = {14.0f, 12.0f, 1.0f};

	modelSkydome_ = Model::CreateFromOBJ("skyDome", true);
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_, camera_);

	uint32_t texStart = TextureManager::Load("text/gameStartText.png");
	uint32_t texTutorial = TextureManager::Load("text/tutorialText.png");

	spriteStartText_ = Sprite::Create(texStart, {640.0f, 500.0f}, {1, 1, 1, 1}, {0.5f, 0.5f});
	spriteTutorialText_ = Sprite::Create(texTutorial, {640.0f, 640.0f}, {1, 1, 1, 1}, {0.5f, 0.5f});

}

/// <summary>
/// 更新
/// </summary>
void TitleScene::Update() {

	switch (phase_) {
	case Phase::kFadeIn:
		// フェードインの更新
		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}

		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, duration_);
			phase_ = Phase::kFadeOut;
		}

		break;
	case Phase::kFadeOut:
		// フェードアウトの更新
		fade_->Update();

		if (fade_->IsFinished()) {
			// タイトルシーンの終了
			isFinished_ = true;
		}

		break;
	}

	// テキストのイージング
	Easing();

	skydome_->Update();
	WorldTransformUpdate(worldTransformTitleCloud_);
	WorldTransformUpdate(worldTransformTitleLogo_);

	// ワールド変換の更新
	WorldTransformUpdate(worldTransform_);
}

// 補助関数
float TitleScene::Lerp(float a, float b, float t) { return a + (b - a) * t; }
float TitleScene::EaseOutCubic(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return 1.0f - std::pow(1.0f - t, 3.0f);
}
float TitleScene::EaseInCubic(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * t;
}

float TitleScene::Easing() {
	elapsedTimer_ += kDeltaTimer;

	const float barSec = 60.0f / kBPM;       // 1拍の長さ
	const float perPulse = barSec / kPulses; // 1パルスの長さ
	const float sumRatio = kUpRatio + kDownRatio + kGapRatio;
	const float upSec = perPulse * (kUpRatio / sumRatio);
	const float downSec = perPulse * (kDownRatio / sumRatio);

	float t = std::fmod(elapsedTimer_, barSec); // 拍内の時刻 [0, barSec)
	int pulse = static_cast<int>(t / perPulse); // どのパルスか 0..3
	float local = t - pulse * perPulse;         // パルス内の時刻
	float s = kBaseScale;

	const float peak = (pulse == 0 ? kAccentScale : kPulseScale); // 1発目だけ強くしたいならここで調整

	if (local < upSec) {
		// 上り：速く（鋭い立ち上がり）
		s = Lerp(kBaseScale, kBaseScale * peak, EaseOutCubic(local / upSec));
	} else if (local < upSec + downSec) {
		// 下り：ゆっくり
		float tt = (local - upSec) / downSec;
		s = Lerp(kBaseScale * peak, kBaseScale, EaseInCubic(tt));
	} else {
		// 間（「っ」）：一定
		s = kBaseScale;
	}

	// スケール反映（等倍拡縮）
	worldTransformTitleLogo_.scale_ = {s * 10.0f, s * 10.0f, s * 10.0f};

	// ちょい上下も付けたい場合（任意）：
	// worldTransform_.translation_.y = 6.5f + (s - kBaseScale) * 4.0f;

	return s;
}

/// <summary>
/// 描画
/// </summary>
void TitleScene::Draw() {

	// モデルの描画前処理
	Model::PreDraw();

	skydome_->Draw();

	// modelTitleCloud_->Draw(worldTransformTitleCloud_, *camera_);
	modelTitleCloud_->Draw(worldTransformTitleCloud_, *camera_);

	// タイトルロゴ描画
	modelTitleLogo_->Draw(worldTransformTitleLogo_, *camera_);

	// モデルの描画後処理
	Model::PostDraw();

	Sprite::PreDraw();

	spriteStartText_->Draw();
	spriteTutorialText_->Draw();

	Sprite::PostDraw();

	// フェードイン
	fade_->Draw();
}

/// <summary>
/// ゲッター
/// </summary>
/// <returns></returns>
bool TitleScene::IsFinished() const { return isFinished_; }