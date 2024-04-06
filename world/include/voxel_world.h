#pragma once

#include "forward.h"

#include "core/variant/dictionary.h"
#include "core/object/class_db.h"

namespace pgvoxel {

class VoxelWorld : public Object {
	GDCLASS(VoxelWorld, Object)
public:

	static Dictionary getMetadata(int32_t x, int32_t z);
	static void setMetadata(int32_t x, int32_t z, const Dictionary &metadata);

private:
	static void _bind_methods();
};

}
