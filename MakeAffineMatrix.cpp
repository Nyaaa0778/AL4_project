#include "MakeAffineMatrix.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

/// <summary>
/// 行列の乗算
/// </summary>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) { return m1 * m2; }

/// <summary>
/// アフィン変換行列作成
/// </summary>
/// <param name="scale"></param>
/// <param name="rotation"></param>
/// <param name="translation"></param>
Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale,
	                       const KamataEngine::Vector3& rotation, 
	                       const KamataEngine::Vector3& translation) {

	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateMatrix = Multiply(Multiply(MakeRotateXMatrix(rotation.x), MakeRotateYMatrix(rotation.y)), MakeRotateZMatrix(rotation.z));
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translation);

	return Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);
}