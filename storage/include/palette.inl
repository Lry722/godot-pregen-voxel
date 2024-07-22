#pragma once

#include <format>
#include <sstream>
#include <stdexcept>
#include <string>

#include "palette.h"
#include "serialize.h"

namespace pgvoxel {

template <typename IndexType, typename DataType, IndexType kMaxSize>
void Palette<IndexType, DataType, kMaxSize>::clear() {
    index_to_data = {0};

    data_to_index.clear();
    data_to_index[0] = 0;

    data_to_ref.clear();
    // 初始用 0 填满
    data_to_ref[0] = kMaxSize;

    size_ = 1;
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
const DataType &Palette<IndexType, DataType, kMaxSize>::pick(const IndexType index) const {
    return index_to_data.at(index);
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
IndexType Palette<IndexType, DataType, kMaxSize>::indexOf(const DataType &data) const {
    return data_to_index.at(data);
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
IndexType Palette<IndexType, DataType, kMaxSize>::update(const DataType &new_data, const IndexType old_data_index) {
    // 这里用 at 是因为可能误传入不存在的的 old_data
    const auto old_data = index_to_data.at(old_data_index);
    if (data_to_ref[old_data] == 0) [[unlikely]] {
        // old_data 的 ref 是 0 表示数据曾经存在，但现在不存在了，是空位
        throw std::out_of_range(std::format("Palette: Invalid index {}.\nStatus:\n{}", old_data_index, toString()));
    }

    if (old_data == new_data) [[unlikely]] {
        return old_data_index;
    }

    // 将 old data 的 ref 减一
    --data_to_ref[old_data];
    if (data_to_ref[old_data] == 0) {
        --size_;
    }

    auto new_data_iter = data_to_ref.find(new_data);
    if (new_data_iter != data_to_ref.end()) {
        // 若 new data 已存在，则简单地将 ref 加一
        ++(new_data_iter->second);
        return data_to_index.at(new_data);
    } else {
        // 若 new data 原先不存在，则要寻找新的 index 进行插入
        size_type new_index = 0;
        // 在已有的 index 中查找空位
        // palette size 很大时，这个遍历可能会很慢，但是考虑到 palette 一般不会太大，且新增数据的频率通常不高，应该可以接受
        while (new_index < index_to_data.size() && data_to_ref[index_to_data[new_index]] != 0) {
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
        ++size_;

        return new_index;
    }
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
void Palette<IndexType, DataType, kMaxSize>::serialize(std::ostringstream &oss) const {
    DataType data;
    IndexType ref;
    for (IndexType i = 1; i < index_to_data.size(); ++i) {
        data = index_to_data[i];
        ref = data_to_ref.at(data);
        if (ref != 0) {
            SERIALIZE_WRITE(oss, i);
            SERIALIZE_WRITE(oss, data);
            SERIALIZE_WRITE(oss, ref);
        }
    }
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
void Palette<IndexType, DataType, kMaxSize>::deserialize(std::istringstream &iss, const uint32_t size) {
    clear();
    size_ = size / kEntrySize + 1;

    IndexType index, ref;
    DataType data;
    IndexType sum{0};
    for (size_type i = 1; i < size_; ++i) {
        DESERIALIZE_READ(iss, index);
        DESERIALIZE_READ(iss, data);
        DESERIALIZE_READ(iss, ref);

        if (index >= index_to_data.size()) {
            index_to_data.resize(index + 1);
        }
        index_to_data[index] = data;
        data_to_index.insert({data, index});
        data_to_ref.insert({data, ref});
        sum += ref;
    }
    // 空气数量不会被记录，需要单独处理
    data_to_ref[0] = kMaxSize - sum;
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
std::string Palette<IndexType, DataType, kMaxSize>::toString() const {
    std::ostringstream oss;
    oss << "Palette{ "
        << "size: " << size_ << ", items: [";
    for (size_type i = 1; i < index_to_data.size(); ++i) {
        if (index_to_data[i] != 0 || i == 0) {
            oss << "{ Index: " << i << ", Data: " << index_to_data[i] << ", RefCount: " << data_to_ref.at(index_to_data[i]) << " }";
            if (i != index_to_data.size() - 1) {
                oss << ", ";
            }
        }
    }
    oss << "] }";
    return oss.str();
}

template <typename IndexType, typename DataType, IndexType kMaxSize>
void Palette<IndexType, DataType, kMaxSize>::fit() {
    // TODO
}

}  // namespace pgvoxel
