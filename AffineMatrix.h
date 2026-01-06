#pragma once
#include "math/Matrix4x4.h"
#include "math/Vector3.h"
#include <cmath>

/// <summary>
/// 行列の積
/// </summary>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

/// <summary>
/// 単位行列
/// </summary>
/// <returns></returns>
KamataEngine::Matrix4x4 Identity();

/// <summary>
/// 拡縮行列
/// </summary>
/// <param name="scale"></param>
/// <returns></returns>
KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);

/// <summary>
/// 回転行列
/// </summary>
/// <param name="rotate"></param>
/// <returns></returns>
KamataEngine::Matrix4x4 MakeRotateMatrix(const KamataEngine::Vector3& rotate);

/// <summary>
/// 平行移動行列
/// </summary>
/// <param name="translate"></param>
/// <returns></returns>
KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);

/// <summary>
///
/// </summary>
/// <param name="scale"></param>
/// <param name="rotate"></param>
/// <param name="translate"></param>
/// <returns></returns>
KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);
