#pragma once

#include "core/object/class_db.h"

namespace pgvoxel{

class VoxelWorldConfig : public Object {
	GDCLASS(VoxelWorldConfig, Object)
public:
	static Variant get(String property_name);
private:
	static void _bind_methods();
};

} //namespace pgvoxel::config
