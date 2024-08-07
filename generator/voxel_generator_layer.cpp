#include "voxel_generator_layer.h"

#include "core/string/print_string.h"
#include "voxel_generation_chunk.h"
#include "voxel_generator.h"
#include "voxel_local_generator.h"
#include "world_db.h"

#include "scene/main/node.h"

#include "modules/pgvoxel/thirdparty/thread-pool/include/BS_thread_pool.hpp"
#include <tbb/task_group.h>
#include <vector>

namespace pgvoxel {

void VoxelGeneratorLayer::generate(Ref<VoxelGenerationChunk> chunk) {
	auto children = get_children();
	std::vector<VoxelLocalGenerator *> generators;
	for (int i = 0; i < children.size(); ++i) {
		generators.push_back(Object::cast_to<VoxelLocalGenerator>(children[i]));
	}

	for (auto generator : generators) {
		print_verbose(String("Generator : {0}").format(varray(generator->get_name())));
		GDVIRTUAL_CALL_PTR(generator, _generate, chunk);
		print_verbose(String("Generator : {0} finished").format(varray(generator->get_name())));
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
