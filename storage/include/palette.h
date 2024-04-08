#pragma once

#include "forward.h"

#include <cstddef>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace pgvoxel {

// 将大范围分布的值映射到从 0 开始的 index，就像在数量有限的调色板格子中只存放当前所需的颜料一样
// 需要注意的是该类永远只会自然增长，哪怕末尾的值已被删除。如果要删除末尾的空值，需要手动调用 fit。
// TODO: fit 函数还未实现
template<typename IndexType, typename DataType>
class Palette {
public:
	Palette() {
		clear();
	}
	void clear() {
		index_to_data.clear();
		index_to_data = { 0 };

		data_to_index.clear();
		data_to_index[0] = 0;

		data_to_ref.clear();
		// 初始用0填满
		// 这里还预设了一个使用条件，即 0 不会被消耗完，否则会出错，不过正常来说也不会扩容到 4294967295 个Entry吧
		data_to_ref[0] = kMaxVoxelData;
	}

	// 返回调色板目前的最大索引
	size_t paletteSize() const { return index_to_data.size(); }

	// 移除一条old_data_index指向的数据，新增一条new_data
	// 之所以old_data是index形式，是因为既然已经用了调色板，调用方持有的必定是压缩后的index而不是原始数据
	// 而要 new_data 调用方肯定只有原始形态，没有 index ，所以 new_data 是 DataType 形式的
	IndexType update(const DataType &new_data, const IndexType old_data_index);

	// 选择index对应的data
	const DataType &pick(const IndexType index) const;
	// 选择data对应的index
	IndexType indexOf(const DataType &data) const;

	// 序列化
	void serialize(std::ostringstream &oss) const;
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);
	std::string toString() const;

	void fit();

private:
	std::vector<DataType> index_to_data;
	std::unordered_map<DataType, IndexType> data_to_index;
	std::unordered_map<DataType, IndexType> data_to_ref;

	// 一条Entry实际上是由index, data, ref组成
	static inline const size_t kEntrySize = sizeof(IndexType) + sizeof(DataType) + sizeof(IndexType);
};
} // namespace pgvoxel
