#include "forward.h"
#include "palette.h"
#include "../../common/include/serialize.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <format>
#include <sstream>
#include <string>

namespace pgvoxel {

template <typename IndexType, typename DataType>
const DataType &Palette<IndexType, DataType>::pick(const IndexType index) const {
	return index_to_data.at(index);
}

template <typename IndexType, typename DataType>
IndexType Palette<IndexType, DataType>::indexOf(const DataType &data) const {
	return data_to_index.at(data);
}

template <typename IndexType, typename DataType>
IndexType Palette<IndexType, DataType>::update(const DataType &new_data, const IndexType old_data_index) {
	const auto old_data = index_to_data.at(old_data_index);
	if (old_data == 0 && old_data_index != 0) {
		// 只有 index 0 的情况下 data 是 0
		// 其他 data 是 0 的情况表示数据还不存在
		// 外部不应当传入不存在的数据的 index
		throw std::runtime_error(std::format("Palette: Invalid old data index {}.\nStatus:\n{}", old_data_index, toString()));
	}

	if (old_data == new_data) {
		return old_data_index;
	}

	if (--data_to_ref[old_data] == 0) {
		// old data的 ref 归零后要删除对应的 Entry
		data_to_ref.erase(old_data);
		data_to_index.erase(old_data);
		index_to_data[old_data_index] = 0;
	}

	auto new_data_iter = data_to_ref.find(new_data);
	if (new_data_iter != data_to_ref.end()) {
		// 如果new data已存在，则简单地将引用加一
		++(new_data_iter->second);
		return data_to_index.at(new_data);
	} else {
		// 如果new data原先不存在，则要寻找新的index进行插入
		size_t new_index = 1;
		// palette size 很大时，这个遍历可能会很慢，但是考虑到 palette 一般不会太大，且插入的频率通常不高，应该可以接受
		while (new_index < index_to_data.size() && index_to_data[new_index] != 0) {
			++new_index;
		}
		if (new_index == index_to_data.size()) {
			// 检索完未发现空位则需扩容
			index_to_data.push_back(new_data);
		} else {
			// 否则使用找到的空位
			index_to_data[new_index] = new_data;
		}
		data_to_index[new_data] = new_index;
		data_to_ref[new_data] = 1;

		return new_index;
	}
}

template <typename IndexType, typename DataType>
void Palette<IndexType, DataType>::serialize(std::ostringstream &oss) const {
	if (paletteSize() == 0) {
		return;
	}
	const size_t palette_size = paletteSize();
	oss.write(reinterpret_cast<const char *>(&palette_size), sizeof(palette_size));
	DataType data;
	IndexType ref;
	for (IndexType i = 1; i < index_to_data.size(); ++i) {
		data = index_to_data[i];
		ref = data_to_ref.at(data);
		if (data != 0) {
			SERIALIZE_WRITE(oss, i);
			SERIALIZE_WRITE(oss, data);
			SERIALIZE_WRITE(oss, ref);
		}
	}
}

template <typename IndexType, typename DataType>
void Palette<IndexType, DataType>::deserialize(std::istringstream &iss, const size_t size) {
	clear();
	if (size == 0) {
		return;
	}

	size_t palette_size;
	iss.read(reinterpret_cast<char *>(&(palette_size)), sizeof(palette_size));
	index_to_data.resize(palette_size);

	IndexType index, ref;
	DataType data;
	IndexType ref_sum = 0;
	// 调色板中可能有空位，因此这里要根据 entry size 计算实际的 entry 数目
	for (size_t i = 0; i < (size - sizeof(palette_size)) / kEntrySize; ++i) {
		DESERIALIZE_READ(iss, index);
		DESERIALIZE_READ(iss, data);
		DESERIALIZE_READ(iss, ref);

		index_to_data[index] = data;
		data_to_index.insert({ data, index });
		data_to_ref.insert({ data, ref });
		ref_sum += ref;
	}
	// 序列化的结果中不保存 0 对应的 ref count
	// 要计算 0 的 ref count，只需要将其初始值 kMaxVoxelData 减去其余 data 的 ref count 即可
	data_to_ref[0] -= ref_sum;
	// print_line(toString().c_str());
}

template <typename IndexType, typename DataType>
std::string Palette<IndexType, DataType>::toString() const {
	std::ostringstream oss;
	oss << "Palette: {\n"
		<< "\tsize: " << index_to_data.size() << "\n"
		<< "\titems: [\n";
	for (size_t i = 0; i < paletteSize(); ++i) {
		if (index_to_data[i] != 0 || i == 0) {
			oss << "\t\t{ Index: " << i << ", Data: " << index_to_data[i] << ", RefCount: " << data_to_ref.at(index_to_data[i]) << " }\n";
		}
	}
	oss << "\t]\n}";
	return oss.str();
}

template <typename IndexType, typename DataType>
void Palette<IndexType, DataType>::fit() {
	
}

} //namespace pgvoxel
