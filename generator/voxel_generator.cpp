#include "voxel_generator.h"
#include "constants.h"
#include "core/error/error_macros.h"
#include "core/object/method_bind.h"
#include "core/string/print_string.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "voxel_chunk.h"
#include "voxel_generation_chunk.h"
#include "voxel_generator_layer.h"
#include "voxel_local_generator.h"
#include "world_config.h"

#include "core/core_bind.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/os/thread_safe.h"

#include <oneapi/tbb/task_group.h>
#include <tbb/concurrent_queue.h>

#include <cstddef>
#include <exception>
#include <memory>
#include <vector>

namespace pgvoxel::generator {

void VoxelGenerator::start() {
	generator_thread_ = std::thread([&]() {
		set_current_thread_safe_for_nodes(true);
		using namespace generator;
		GET_WORLD_CONFIG(, config);

		TypedArray<Node> layers = get_children();
		for (int i = 0; i < layers.size(); i++) {
			print_line(String("Layer {0}").format(varray(i)));
			auto layer = Object::cast_to<VoxelGeneratorLayer>(layers[i]);
			std::vector<Ref<VoxelGenerationChunk>> chunks;
			size_t batch_count = 0;
			for (size_t x = 0; x < config.width; ++x) {
				for (size_t z = 0; z < config.width; ++z) {
					Ref<VoxelGenerationChunk> chunk;
					chunk.instantiate(x, z);
					chunks.push_back(chunk);
					if (chunks.size() >= batch_size_) {
						print_line(String("  Batch {0}").format(varray(batch_count)));
						layer->generate(chunks);
						chunks.clear();
						batch_count++;
					}
				}
			}
			if (!chunks.empty()) {
				layer->generate(chunks);
			}
		}
	});
}

PackedStringArray VoxelGenerator::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	// TODO: 目前只是检测子节点不为空，实际上要检测是否包含VoxelGeneratorLayer节点
	if (get_child_count() == 0) {
		warnings.push_back(RTR("This generator has no layer. Nothing will exist in the generated world."));
	}

	return warnings;
}

void VoxelGenerator::_bind_methods() {
	using namespace generator;

	ClassDB::bind_method(D_METHOD("start"), &VoxelGenerator::start);

	ClassDB::bind_method(D_METHOD("setBatchSize", "batch_size"), &VoxelGenerator::setBatchSize);
	ClassDB::bind_method(D_METHOD("getBatchSize"), &VoxelGenerator::getBatchSize);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "batch_size"), "setBatchSize", "getBatchSize");

	// 全部生成完毕
	ADD_SIGNAL(MethodInfo(generation_finished));
}

} //namespace pgvoxel::generator
