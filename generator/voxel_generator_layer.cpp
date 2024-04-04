#include "voxel_generator_layer.h"
#include "core/error/error_macros.h"
#include "core/object/object.h"
#include "scene/main/node.h"
#include "voxel_chunk.h"
#include "voxel_generation_chunk.h"
#include "voxel_generator.h"
#include "voxel_local_generator.h"
#include "world_config.h"
#include <cstddef>
#include <memory>

namespace pgvoxel::generator {

void VoxelGeneratorLayer::generate(std::vector<Ref<VoxelGenerationChunk>> &chunks) {
	auto children = get_children();
	for (auto &chunk : chunks) {
		for (int i = 0;i < children.size(); ++i) {
			auto generator = Object::cast_to<VoxelLocalGenerator>(children[i]);
			GDVIRTUAL_CALL_PTR(generator, _generate, chunk);
		}
	}
}

PackedStringArray VoxelGeneratorLayer::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	if (get_child_count() == 0) {
		warnings.push_back(RTR("This layer has no local generator."));
	}

	return warnings;
}

void VoxelGeneratorLayer::_bind_methods() {

}

} //namespace pgvoxel
