#pragma once
#include "KamataEngine.h"

class Fade {
public:
	// フェードの状態
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

private:
	KamataEngine::Sprite* sprite_ = nullptr;

	// 現在のフェードの状態
	Status status_ = Status::None;
	// フェードの継続時間
	float duration_ = 0.0f;
	// 経過時間カウンター
	float counter_ = 0.0f;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// フェード開始
	/// </summary>
	/// <param name="status"></param>
	/// <param name="duration"></param>
	void Start(Status status, float duration);
	/// <summary>
	/// フェードインの更新
	/// </summary>
	void UpdateFadeIn();
	/// <summary>
	/// フェードアウトの更新
	/// </summary>
	void UpdateFadeOut();
	/// <summary>
	/// フェード終了
	/// </summary>
	void Stop();

	/// <summary>
	/// フェード終了判定
	/// </summary>
	/// <returns></returns>
	bool IsFinished() const;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
};
