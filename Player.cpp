#include "Player.h"
#include<cassert>

using namespace KamataEngine;

// 初期化
void Player::Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position) { 
	//NULLチェック	
	assert(model);
	//プレイヤーモデル
	model_ = model;

	//プレイヤー初期位置
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

}
// 更新
void Player::Update() {
	//行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}
// 描画
void Player::Draw() {
	model_->Draw(worldTransform_, *camera_);
}