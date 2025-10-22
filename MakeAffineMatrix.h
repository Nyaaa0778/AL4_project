#pragma once
#include"math/MathUtility.h"

/// <summary>
/// 行列の乗算
/// </summary>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

/// <summary>
/// アフィン変換行列作成
/// </summary>
/// <param name="scale"></param>
/// <param name="rotation"></param>
/// <param name="translation"></param>
KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotation, const KamataEngine::Vector3& translation);