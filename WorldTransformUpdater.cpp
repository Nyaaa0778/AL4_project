#include "WorldTransformUpdater.h"

/// <summary>
/// ワールドトランスフォームの更新
/// </summary>
/// <param name="worldTransform">ワールドトランスフォーム</param>
void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform) { 
	//スケール、拡縮、平行移動を合成して行列を計算
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

	//定数バッファへ書き込む
	worldTransform.TransferMatrix();
}