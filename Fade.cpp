#define NOMINMAX
#include "Fade.h"
#include <algorithm>

using namespace KamataEngine;

/// <summary>
/// 初期化
/// </summary>
void Fade::Initialize() {

	// スプライトの生成
	sprite_ = Sprite::Create(0, Vector2(0, 0), Vector4(0, 0, 0, 1));
	sprite_->SetSize(Vector2(1280, 720));
	sprite_->SetColor(Vector4(0, 0, 0, 1));
}

/// <summary>
/// 更新
/// </summary>
void Fade::Update() {
	// フェード状態による分岐
	switch (status_) {
	case Status::None:

		// 何もしない

		break;
	case Status::FadeIn:

		// フェードイン
		UpdateFadeIn();

		break;
	case Status::FadeOut:

		// フェードアウト
		UpdateFadeOut();

		break;
	}
}

/// <summary>
/// フェード開始
/// </summary>
/// <param name="status"></param>
/// <param name="duration"></param>
void Fade::Start(Status status, float duration) {
	// フェード状態を変更
	status_ = status;
	// フェードの継続時間を初期化
	duration_ = duration;
	// 継続時間カウンターをリセット
	counter_ = 0.0f;
}

/// <summary>
/// フェードインの更新
/// </summary>
void Fade::UpdateFadeIn() {
	// 1フレーム分の秒数をカウントアップ
	counter_ += 1.0f / 60.0f;
	// フェード経過時間に達したら打ち止め
	counter_ = std::min(counter_, duration_);
	// 1.0f ~ 0.0fの間で経過時間がフェード時間に近付くほどアルファ値を小さくする
	sprite_->SetColor(Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));
}
/// <summary>
/// フェードアウトの更新
/// </summary>
void Fade::UpdateFadeOut() {
	// 1フレーム分の秒数をカウントアップ
	counter_ += 1.0f / 60.0f;
	// フェード継続時間に達したら打ち止め
	counter_ = std::min(counter_, duration_);
	// 0.0f ~ 1.0fの間で経過時間がフェード時間に近付くほどアルファ値を大きくする
	sprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
}

/// <summary>
/// フェード終了
/// </summary>
void Fade::Stop() { status_ = Status::None; }

/// <summary>
/// フェード終了判定
/// </summary>
/// <returns></returns>
bool Fade::IsFinished() const {
	// フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		return counter_ >= duration_;
	}

	return true;
}

/// <summary>
/// 描画
/// </summary>
void Fade::Draw() {
	// StatusがNoneなら何もしない
	if (status_ == Status::None) {
		return;
	}

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	// スプライトインスタンスの描画
	sprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
}