#pragma once

#include "forward.h"
#include "modules/pgvoxel/storage/include/forward.h"

namespace pgvoxel {

// 用于触发区块加载/卸载
class Viewer {
	void move(size_t x, size_t z) {
	}

	void view(LoadedChunk &level) const {
	}

	void unview(LoadedChunk &info) const {
	}

	size_t x_, z_;
	size_t view_distance;
	LoadLevel load_level;
};

} //namespace pgvoxel
