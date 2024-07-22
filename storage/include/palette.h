#pragma once

#include <cstdint>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace pgvoxel {

// 将离散分布的值映射到从 0 开始的连续 index，就像在数量有限的调色板格子中只存放当前所需的颜料一样
// 注意，该类永远只会自然增长，哪怕末尾的值已被删除。如果要删除末尾的空值，需要手动调用 fit。
// TODO: fit 函数还未实现
template <typename IndexType, typename DataType, IndexType kMaxSize>
class Palette {
	public:
	typedef uint32_t size_type;
   public:
    Palette() { clear(); }
    void clear();

	// 返回调色板中元素的数量
    size_type size() const { return size_; }
	// 返回调色板中最大的 Index，包括已失效的 Entry，因此可能比实际情况更大
    size_type maxIndex() const { return index_to_data.size() - 1; }

    // 移除一条 old_data_index 指向的数据，新增一条 new_data
    // 之所以 old_data 是 IndexType，是因为调用方持有的必定是映射后的 index 而不是原始 data
    // 反之对于 new_data 调用方肯定只有原始 data，没有 index ，所以 new_data 是 DataType
    IndexType update(const DataType &new_data, const IndexType old_data_index);

    // 选择 index 对应的data
    const DataType &pick(const IndexType index) const;
    // 选择 data 对应的index
    IndexType indexOf(const DataType &data) const;

    // 序列化/反序列化
    void serialize(std::ostringstream &oss) const;
    void deserialize(std::istringstream &iss, const uint32_t size);

    std::string toString() const;

    void fit();

   private:
    size_type size_{1};
    std::vector<DataType> index_to_data;
    std::unordered_map<DataType, IndexType> data_to_index;
    std::unordered_map<DataType, IndexType> data_to_ref;

    // 一条Entry实际上是由index, data, ref组成
    static inline const uint32_t kEntrySize = sizeof(IndexType) + sizeof(DataType) + sizeof(IndexType);
};
}  // namespace pgvoxel
