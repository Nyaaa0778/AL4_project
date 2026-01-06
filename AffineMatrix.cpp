#include "AffineMatrix.h"

using namespace KamataEngine;

/// <summary>
/// 行列の積
/// </summary>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0;
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}

	return result;
}

/// <summary>
/// 単位行列
/// </summary>
/// <returns></returns>
Matrix4x4 Identity() {
	Matrix4x4 result{};

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	return result;
}

/// <summary>
/// 拡縮行列
/// </summary>
/// <param name="scale"></param>
/// <returns></returns>
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = Identity();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	return result;
}

/// <summary>
/// 回転行列
/// </summary>
/// <param name="rotate"></param>
/// <returns></returns>
Matrix4x4 MakeRotateMatrix(const Vector3& rotate) {
	Matrix4x4 rotationX = Identity();
	rotationX.m[1][1] = std::cos(rotate.x);
	rotationX.m[1][2] = std::sin(rotate.x);
	rotationX.m[2][1] = -std::sin(rotate.x);
	rotationX.m[2][2] = std::cos(rotate.x);

	Matrix4x4 rotationY = Identity();
	rotationY.m[0][0] = std::cos(rotate.y);
	rotationY.m[0][2] = -std::sin(rotate.y);
	rotationY.m[2][0] = std::sin(rotate.y);
	rotationY.m[2][2] = std::cos(rotate.y);

	Matrix4x4 rotationZ = Identity();
	rotationZ.m[0][0] = std::cos(rotate.z);
	rotationZ.m[0][1] = std::sin(rotate.z);
	rotationZ.m[1][0] = -std::sin(rotate.z);
	rotationZ.m[1][1] = std::cos(rotate.z);

	return Multiply(Multiply(rotationX, rotationY), rotationZ);
}

/// <summary>
/// 平行移動行列
/// </summary>
/// <param name="translate"></param>
/// <returns></returns>
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = Identity();
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

/// <summary>
/// アフィン変換行列
/// </summary>
/// <param name="scale"></param>
/// <param name="rotate"></param>
/// <param name="translate"></param>
/// <returns></returns>
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	Matrix4x4 worldTransformMatrix = Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);

	return worldTransformMatrix;
}