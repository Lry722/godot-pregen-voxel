#include "buffer.h"
#include "forward.h"

#include "core/string/print_string.h"

#include <lz4.h>
#include <vector>

namespace pgvoxel {

// 设置一个点的值
void Buffer::setVoxel(const Coord pos, const VoxelData data) {
	data_[pos_to_index(pos)] = data;
}

// 获取一个点的值
VoxelData Buffer::getVoxel(const Coord pos) const {
	return data_[pos_to_index(pos)];
}

// 设置位于(x, z)处，从buttom到top间的数据为data
void Buffer::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data) {
	std::fill(data_.begin() + pos_to_index({ x, buttom, z }), data_.begin() + pos_to_index({ x, top, z }), data);
}

// 设置位于(x, z)处，从buttom到top间的数据为data，data的size应当大于等于 top - buttom
void Buffer::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const std::vector<VoxelData> &data) {
	const CoordAxis begin = pos_to_index({ x, buttom, z });
	const CoordAxis end = pos_to_index({ x, top, z });
	const size_t length = end - begin;
	std::copy(data.begin(), data.begin() + length, data_.begin() + begin);
}

// 获取位于(x, z)处，从buttom到top间的数据
std::vector<VoxelData> Buffer::getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const {
	return std::vector<VoxelData>(data_.begin() + pos_to_index({ x, buttom, z }), data_.begin() + pos_to_index({ x, top, z }));
}

// 设置begin到end两点围成的区域中的值，效果等同于遍历水平面，逐个调用setBar
void Buffer::setBlock(const Coord begin, const Coord end, const VoxelData data) {
	for (CoordAxis x = begin.x; x < end.x; x++) {
		for (CoordAxis z = begin.z; z < end.z; z++) {
			setBar(x, z, begin.y, end.y, data);
		}
	}
}

// 序列化
void Buffer::serialize(std::ostringstream &oss) {
	// 写入原始数据的大小，此处size表示原始数据的大小
	std::size_t size{ data_.size() * sizeof(decltype(data_)::value_type) };
	oss.write(reinterpret_cast<const char *>(&size), sizeof(size));

	// 使用LZ4压缩
	std::string buffer;
	buffer.resize(LZ4_COMPRESSBOUND(size));
	// 此处size表示压缩后数据的大小
	size = LZ4_compress_default(reinterpret_cast<char *>(data_.data()), buffer.data(), size, buffer.size());
	if (size <= 0) [[unlikely]] {
		print_error("Buffer: Compression failed!");
	}

	oss.write(buffer.data(), size);
}

// 反序列化
void Buffer::deserialize(std::istringstream &iss, const size_t size) {
	// 读取原始数据的大小
	std::size_t original_size;
	iss.read(reinterpret_cast<char *>(&original_size), sizeof(original_size));

	// 分配足够的内存来存储解压后的数据
	std::string decompressedData;
	decompressedData.resize(original_size);

	// 分配内存来存储压缩数据
	std::string compressedData;
	compressedData.resize(size);

	// 读取压缩数据
	iss.read(compressedData.data(), size);

	// 使用LZ4解压
	int decompressedSize = LZ4_decompress_safe(compressedData.data(), reinterpret_cast<char *>(data_.data()), size - sizeof(original_size), original_size);
	if (decompressedSize <= 0) [[unlikely]] {
		print_error("Buffer: Decompression failed!");
	}
}

} //namespace pgvoxel
