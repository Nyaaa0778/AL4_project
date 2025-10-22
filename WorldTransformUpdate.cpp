#include"WorldTransformUpdate.h"
#include"MakeAffineMatrix.h"

using namespace KamataEngine;

/// <summary>
/// ワールドトランスフォームの更新
/// </summary>
void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform) {
	// スケール、拡縮、平行移動を合成して行列を計算
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

	// 定数バッファへ書き込む
	worldTransform.TransferMatrix();
}