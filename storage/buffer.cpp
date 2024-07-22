#include "buffer.h"

#include <lz4.h>

#include <cstring>
#include <stdexcept>
#include <vector>

namespace pgvoxel {

// 设置一个点的值
void Buffer::setVoxel(const Coord pos, const VoxelData data) { data_[pos_to_index(pos)] = data; }

// 获取一个点的值
VoxelData Buffer::getVoxel(const Coord pos) const { return data_[pos_to_index(pos)]; }

// 设置位于(x, z)处，从buttom到top间的数据为data
void Buffer::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data) {
    std::fill(data_.begin() + pos_to_index({x, buttom, z}), data_.begin() + pos_to_index({x, top, z}), data);
}

// 设置位于(x, z)处，从buttom到top间的数据为data，data的size应当大于等于 top - buttom
void Buffer::setBar(const Coord &pos, const std::vector<VoxelData> &data) { memcpy(data_.data() + pos_to_index(pos), data.data(), data.size() * sizeof(VoxelData)); }

// 获取位于(x, z)处，从buttom到top间的数据
std::vector<VoxelData> Buffer::getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const {
    std::vector<VoxelData> result(top - buttom);
    memcpy(result.data(), data_.data() + pos_to_index({x, buttom, z}), (top - buttom) * sizeof(VoxelData));
    return result;
}

// 设置begin到end两点围成的区域中的值，效果等同于遍历水平面，逐个调用setBar
void Buffer::setBlock(const Coord begin, const Coord end, const VoxelData data) {
    for (CoordAxis x = begin.x; x < end.x; x++) {
        for (CoordAxis z = begin.z; z < end.z; z++) {
            setBar(x, z, begin.y, end.y, data);
        }
    }
}

void Buffer::setBlock(const Coord pos, const Buffer &data) {
    for (CoordAxis x = pos.x; x < pos.x + data.width_; x++) {
        for (CoordAxis z = pos.z; z < pos.z + data.depth_; z++) {
            setBar({x, 0, z}, data.getBar(x, z, 0, data.height_));
        }
    }
}

Buffer Buffer::getBlock(const Coord begin, const Coord end) const {}

// 序列化
void Buffer::serialize(std::ostringstream &oss) const {
    // 写入未压缩数据的大小，此处 size 表示未压缩数据的大小
    uint32_t size{static_cast<uint32_t>(data_.size() * sizeof(decltype(data_)::value_type))};
    oss.write(reinterpret_cast<const char *>(&size), sizeof(size));

    // 压缩
    std::string buffer;
    buffer.resize(LZ4_COMPRESSBOUND(size));
    // 此处size表示压缩后数据的大小
    size = LZ4_compress_default(reinterpret_cast<const char *>(data_.data()), buffer.data(), size, buffer.size());
    if (size <= 0) [[unlikely]] {
        throw std::runtime_error("Buffer: Compression failed!");
    }
}

// 反序列化
void Buffer::deserialize(std::istringstream &iss, const uint32_t size) {
    // 读取未压缩数据的大小
    uint32_t original_size;
    iss.read(reinterpret_cast<char *>(&original_size), sizeof(original_size));

    // 分配足够的内存来存储解压后的数据
    data_.resize(original_size);

    // 读取压缩过的数据
    std::string compressedData;
    compressedData.resize(size - sizeof(original_size));
    iss.read(compressedData.data(), compressedData.size());

    // 解压
    int decompressedSize = LZ4_decompress_safe(compressedData.data(), reinterpret_cast<char *>(data_.data()), compressedData.size(), original_size);
    if (decompressedSize <= 0) [[unlikely]] {
        throw std::runtime_error("Buffer: Decompression failed!");
    }
}

}  // namespace pgvoxel
