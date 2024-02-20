#pragma once

#include "constants.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace pgvoxel {
class Palette {
public:
	Palette() {
		index_to_data[0] = 0;
		data_to_ref[0] = storage::kMaxVoxelData;
	}

	size_t size() const { return index_to_data.size(); }

	// 更新一条数据，返回值表示是否导致size增加
	bool update(const VoxelData new_data, const VoxelData old_data);

	// 选择index对应的data
	VoxelData pick(const size_t index) const;
	// 选择data对应的index
	size_t indexOf(const VoxelData data) const;

	// 序列化
	void serialize(std::ostringstream &oss) const;
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);

private:
	std::map<size_t, VoxelData> index_to_data;
	std::unordered_map<VoxelData, size_t> data_to_ref;

	// 一条Entry实际上是由index, data, ref组成
	static inline const size_t kEntrySize = sizeof(size_t) + sizeof(VoxelData) + sizeof(size_t);
};
} // namespace pgvoxel
