#include "voxel_world_config.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "world_config.h"

#include <string>

namespace pgvoxel::config {

Variant VoxelWorldConfig::get(String property_name) {
	GET_WORLD_CONFIG(Variant(), config);
	if (property_name == "name") {
	    return config.name.c_str();
	} else if (property_name == "seed") {
	    return config.seed;
	} else if (property_name == "width") {
	    return config.width;
	} else {
		print_error("Invalid world config property name.");
		return Variant();
	}
}

void VoxelWorldConfig:: _bind_methods() {
	ClassDB::bind_static_method("VoxelWorldConfig" ,  D_METHOD("get_property", "property_name"), &VoxelWorldConfig::get);
}

}
