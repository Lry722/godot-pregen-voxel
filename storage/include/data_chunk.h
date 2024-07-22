#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "buffer.h"
#include "forward.h"
#include "packed_array.h"
#include "palette.h"

namespace pgvoxel {

// DataChunk 是一个长宽均为 kWidth，高为 kHeight 的立方体，采用 Palette + PackedArray 的方式减小数据体积
// 局部坐标按照 vec3_to_index 的规定映射到 data 中的序号
template <CoordAxis kWidth, CoordAxis kHeight>
class DataChunk {
   public:
    // 2 * kWidthBits + kHeightBits 应小于 CoordAxis 的位数
    inline static const uint8_t kWidthBits = std::bit_width(kWidth - 1);
    inline static const uint8_t kHeightBits = std::bit_width(kHeight - 1);
    static uint64_t pos_to_index(const Coord &pos) { return pgvoxel::pos_to_index(pos, kWidthBits, kHeightBits); }

   public:
    // 单点操作
    void setVoxel(const Coord pos, const VoxelData data);
    VoxelData getVoxel(const Coord pos) const;

    // 竖列操作，比多次单点操作更快
	// 只有垂直方向上的数据是连续的，因此 setBar 只能处理竖列
    void setBar(const Coord &pos, const std::vector<VoxelData> &data);
    void setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data);
    std::vector<VoxelData> getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const;

    // 块操作，等效于遍历块的底面，依次调用竖列操作
	// 超出的部分会被忽略
    void setBlock(const Coord begin, const Coord end, const VoxelData data);
    void setBlock(const Coord pos, const Buffer &data);
    Buffer getBlock(const Coord begin, const Coord end) const;

    // 序列化/反序列化
    void serialize(std::ostringstream &oss) const;
    void deserialize(std::istringstream &iss, const uint32_t size);

    std::string toString() const;

    // 尝试清除冗余数据
    void fit();

   private:
    static constexpr VoxelData kSize{kWidth * kWidth * kHeight};
    Palette<VoxelData, VoxelData, kSize> palette_;
    PackedArray<> data_{kSize};
};

}  // namespace pgvoxel
