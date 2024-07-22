#pragma once

#include <lz4.h>

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "data_chunk.h"
#include "forward.h"
#include "palette.inl"
#include "serialize.h"

namespace pgvoxel {

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::setVoxel(const Coord pos, const VoxelData new_data) {
    auto old_data_index = data_[pos_to_index(pos)];

    const auto new_data_index = palette_.update(new_data, old_data_index);
    if (palette_.maxIndex() > data_.elementCapacity()) {
        data_.grow();
    }

    old_data_index = new_data_index;
}

template <CoordAxis kWidth, CoordAxis kHeight>
VoxelData DataChunk<kWidth, kHeight>::getVoxel(const Coord pos) const {
    return palette_.pick(data_[pos_to_index(pos)]);
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data) {
    const auto begin = pos_to_index({x, buttom, z});
    const auto end = pos_to_index({x, top, z});
    auto covered = data_.getRange(begin, end);

    for (size_t i = 0; i < covered.size(); ++i) {
        palette_.update(data, covered[i]);
    }
    // 由于最多只会添加一个元素，因此最多只需要增长一次
    if (palette_.maxIndex() > data_.elementCapacity()) {
        data_.grow();
    }

    data_.setRange(begin, end, palette_.indexOf(data));
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::setBar(const Coord &pos, const std::vector<VoxelData> &data) {
    const auto begin = pos_to_index(pos);
    const auto end = pos_to_index({pos.x, pos.y + data.size(), pos.z});
    auto covered = data_.getRange(begin, end);

    std::vector<VoxelData> result;
    result.reserve(covered.size());
    VoxelData max_data_index{palette_.maxIndex()};

    for (size_t i = 0; i < covered.size(); ++i) {
        result.push_back(palette_.update(data[i], covered[i]));
        max_data_index = std::max(max_data_index, result[i]);
    }

    // 需增长到可容纳所有新元素
    data_.transformTo(std::bit_width(max_data_index));
    data_.setRange(begin, end, result);
}

template <CoordAxis kWidth, CoordAxis kHeight>
std::vector<VoxelData> DataChunk<kWidth, kHeight>::getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const {
    const auto &dataIndeies = data_.getRange(pos_to_index({x, buttom, z}), pos_to_index({x, top, z}));
    std::vector<VoxelData> result;
    result.resize(dataIndeies.size());
    for (size_t i = 0; i < dataIndeies.size(); ++i) {
        result[i] = palette_.pick(dataIndeies[i]);
    }
    return result;
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::setBlock(const Coord begin, const Coord end, const VoxelData data) {
    for (CoordAxis x = begin.x; x < end.x; ++x) {
        for (CoordAxis z = begin.z; z < end.z; ++z) {
            setBar(x, z, begin.y, end.y, data);
        }
    }
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::setBlock(const Coord position, const Buffer &data) {
    for (CoordAxis dx = 0; dx < data.getWidth() && position.x + dx < kWidth; ++dx) {
        for (CoordAxis dz = position.z; dz < data.getDepth() && position.z + dz < kWidth; ++dz) {
            setBar(position.x + dx, position.z + dz, position.y, position.y + data.getHeight(), data.getBar(dx, dz, 0, data.getHeight()));
        }
    }
}

template <CoordAxis kWidth, CoordAxis kHeight>
Buffer DataChunk<kWidth, kHeight>::getBlock(const Coord begin, const Coord end) const {
    Buffer result(end.x - begin.x, end.y - begin.y, end.z - begin.z);
    for (CoordAxis dx = 0; dx < result.getWidth(); ++dx) {
        for (CoordAxis dz = 0; dz < result.getDepth(); ++dz) {
            result.setBar(dx, dz, 0, result.getHeight(), getBar(begin.x + dx, begin.z + dz, begin.y, begin.y + result.getHeight()));
        }
    }
    return result;
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::serialize(std::ostringstream &oss) const {
    // 生成原始数据
    std::ostringstream oss_uncompressed;
    oss_uncompressed << palette_;
    oss_uncompressed << data_;
    auto data = oss_uncompressed.view();

    // 写入未压缩时数据的大小
    uint32_t size{static_cast<uint32_t>(data.size())};
    oss.write(reinterpret_cast<const char *>(&size), sizeof(size));

    // 使用LZ4压缩
    std::string buffer;
    buffer.resize(LZ4_COMPRESSBOUND(size));
    // 此处 size 表示压缩后数据的大小
    size = LZ4_compress_default(data.data(), buffer.data(), size, buffer.size());
    if (size <= 0) [[unlikely]] {
        throw std::runtime_error("Chunk compression failed!");
    }

    oss.write(buffer.data(), size);
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::deserialize(std::istringstream &iss, const uint32_t size) {
    // 读取未压缩时数据的大小
    uint32_t original_size;
    iss.read(reinterpret_cast<char *>(&original_size), sizeof(original_size));

    // 分配足够的空间来存储解压后的数据
    std::string decompressedData;
    decompressedData.resize(original_size);

    // 读取压缩过数据
    std::string compressedData;
    compressedData.resize(size);
    iss.read(compressedData.data(), size);

    // 使用LZ4解压
    int decompressedSize = LZ4_decompress_safe(compressedData.data(), decompressedData.data(), size - sizeof(original_size), original_size);
    if (decompressedSize <= 0) [[unlikely]] {
        throw std::runtime_error("Chunk decompression failed!");
    }

    // 反序列化解压后的数据
    std::istringstream iss_uncompressed(decompressedData);
    iss_uncompressed >> palette_ >> data_;
}

template <CoordAxis kWidth, CoordAxis kHeight>
std::string DataChunk<kWidth, kHeight>::toString() const {
    std::ostringstream oss;
    oss << "Chunk{ " 
		<< "palette: " 
		<< palette_.toString() 
		<< ", terrain: " 
		<< data_.toString() 
		<< " }";
    return oss.str();
}

template <CoordAxis kWidth, CoordAxis kHeight>
void DataChunk<kWidth, kHeight>::fit() {
    palette_.fit();
    data_.fit();
}

}  // namespace pgvoxel
