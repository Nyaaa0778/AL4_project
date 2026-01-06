#include "Skydome.h"
#include <3d\Model.h>
#include <cassert>

using namespace KamataEngine;

void Skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	// NULLポインタチェック
	assert(model);

	model_ = model;

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

	worldTransform_.Initialize();
}

/// <summary>
/// 更新処理
/// </summary>
void Skydome::Update() {
	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Skydome::Draw() {
	//3Dモデルの描画
	model_->Draw(worldTransform_, *camera_);
}