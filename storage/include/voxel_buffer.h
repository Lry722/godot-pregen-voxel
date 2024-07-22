#pragma once

#include "buffer.h"
#include "core/error/error_macros.h"
#include "core/math/vector3.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#include "forward.h"
#include <cstdint>

namespace pgvoxel {

class VoxelBuffer : public RefCounted {
	GDCLASS(VoxelBuffer, RefCounted)
public:
	VoxelBuffer(CoordAxis width = 0, CoordAxis height = 0, CoordAxis depth = 0) :
			data_(width, height, depth) {
	}

	void init(CoordAxis width, CoordAxis height, CoordAxis depth) {
		data_.init(width, height, depth);
	}

	void set_voxel(const Vector3i &pos, const int32_t data);
	int32_t get_voxel(const Vector3i &pos) const;

	void set_bar(const int32_t x, const int32_t z, const int32_t buttom, const int32_t top, const int32_t data);
	void set_block(const Vector3i &begin, const Vector3i &end, const VoxelData data);

	Buffer &data() { return data_; }

private:
	static void _bind_methods();

	Buffer data_;
};

} //namespace pgvoxel
