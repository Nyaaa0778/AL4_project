#pragma once
#include <math/Vector3.h>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	struct Rect {
		float left;   // 左端
		float right;  // 右端
		float bottom; // 下端
		float top;    // 上端
	};

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;

public:
	/// <summary>
	/// マップチップデータをリセット
	/// </summary>
	void ResetMapChipData();
	/// <summary>
	/// マップチップファイルを読み込む
	/// </summary>
	/// <param name="filePath"></param>
	void LoadMapChipCsv(const std::string& filePath);
	/// <summary>
	/// マップチップ種別を取得
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	/// <summary>
	/// マップチップのワールド座標を取得
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
	uint32_t GetNumBlockVirtical();
	uint32_t GetNumBlockHorizontal();

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
};