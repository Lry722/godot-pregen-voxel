#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "data_chunk.h"
#include "forward.h"

namespace pgvoxel {

template <CoordAxis Width, CoordAxis Height>
class Chunk {
   public:
    static inline const CoordAxis kWidth = Width;
    static inline const CoordAxis kHeight = Height;
    static inline const uint8_t kDataChunkNums{8};
    // 辅助函数，方便创建
    static std::unique_ptr<Chunk<kWidth, Height>> create(const Coord &position) { return std::make_unique<Chunk<kWidth, Height>>(position); }

   public:
    Chunk(const Coord &position) : kPosition(position) {}
    // Chunk 太重了，没有理由被整个拷贝
    Chunk(const Chunk<kWidth, Height> &other) = delete;
    Chunk<kWidth, Height> &operator=(const Chunk<kWidth, Height> &other) = delete;

    Coord getPosition() const { return kPosition; }

    // 单点操作
    void setVoxel(const Coord &pos, const VoxelData data, uint8_t layer);
    VoxelData getVoxel(const Coord &pos, uint8_t layer) const;

    // 竖列操作，比多次单点操作更快
    // terrain 只有垂直方向上的数据是连续的，所以 setBar 只能处理竖列
    void setBar(const Coord &pos, const std::vector<VoxelData> &data, uint8_t layer);
    void setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data, uint8_t layer);
    std::vector<VoxelData> getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, uint8_t layer) const;

    // 块操作，等效于遍历块的底面，依次调用竖列操作
    // 超出的部分会被忽略
    void setBlock(const Coord &begin, const Coord &end, const VoxelData data, uint8_t layer);
    void setBlock(const Coord &pos, const Buffer &data, uint8_t layer);
    Buffer getBlock(const Coord &begin, const Coord &end, uint8_t layer) const;

    // 序列化/反序列化
    void serialize(std::ostringstream &oss) const;
    void deserialize(std::istringstream &iss, const uint32_t size);

    // 尝试清理冗余数据
    void fit();

   private:
    const Coord kPosition;
    std::array<DataChunk<kWidth, Height>, kDataChunkNums> dataChunks_;
    std::unordered_map<Coord, std::string> metadatas;
};

template <CoordAxis kWidth, CoordAxis kHeight>
Chunk<kWidth, kWidth> slice(const Chunk<kWidth, kHeight> *const chunk, int index);

}  // namespace pgvoxel
