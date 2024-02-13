#pragma once

#include "../world.h"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <optional>
#include <tbb/concurrent_hash_map.h>

namespace lry {

template <size_t kWidth, size_t kHeight, typename TPalleteKey,
          typename TPalleteValue = std::size_t>
class Chunk {
  public:
    using Palette = tbb::concurrent_hash_map<TPalleteKey, TPalleteValue>;
    using InnerPosition = glm::vec<3, TPalleteKey>;

  public:
    Chunk(InnerPosition position_in_world)
        : global_chunk_position(position_in_world) {}
    virtual ~Chunk();

    [[nodiscard]] InnerPosition
    global2InnerPosition(const GlobalPosition global_position) const {
        return global_position - global_chunk_position;
    }

    [[nodiscard]] TPalleteKey
    global2InnerIndex(const GlobalPosition global_position) const {
        const auto [x, y, z] = global2InnerIndex(global_position);
        return z * kWidth * kHeight + x * kHeight + y;
    }

    [[nodiscard]] static constexpr TPalleteKey getSize() {
        return kWidth * kHeight * kWidth;
    }

  private:
    // 方块在区块内的id到真实id的映射，参考Minecraft中的palette
    Palette palette;

    const InnerPosition global_chunk_position;
    std::array<TPalleteKey, getSize()> data{};
    std::optional<std::array<float, getSize()>> damage{};

    // 是否生成/加载完毕，准备好被读取和修改
    std::atomic_bool finished{false};
};

} // namespace lry
