#pragma once

#include "constants.h"
#include <cstdint>

namespace pgvoxel::world
{
    // 用于触发区块加载/卸载
    class Viewer
    {
		void move(size_t x, size_t z) {

		}

        void view(storage::LoadedChunk &level) const
        {

        }

        void unview(storage::LoadedChunk &info) const
        {

        }

        size_t x_, z_;
        size_t view_distance;
        LoadLevel load_level;
    };

} // namespace lry
