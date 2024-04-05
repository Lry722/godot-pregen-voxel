#include "voxel_generator.h"

#include "voxel_generation_chunk.h"
#include "voxel_generator_layer.h"
#include "world_config.h"

#include "world_db.h"

#include <oneapi/tbb/task_group.h>
#include <tbb/concurrent_queue.h>

#include <memory>
#include <vector>

namespace pgvoxel{

void VoxelGenerator::start() {
	generator_thread_ = std::thread([&]() {
		set_current_thread_safe_for_nodes(true);

		GET_WORLD_CONFIG(, config);

		TypedArray<Node> layers = get_children();
		for (int i = 0; i < layers.size(); i++) {
			print_line(String("Layer {0}").format(varray(i)));
			auto layer = Object::cast_to<VoxelGeneratorLayer>(layers[i]);
			layer->setIndex(i);
			std::vector<Ref<VoxelGenerationChunk>> chunks;
			size_t batch_count = 0;
			for (size_t x = 0; x < config.width; ++x) {
				for (size_t z = 0; z < config.width; ++z) {
					Ref<VoxelGenerationChunk> chunk;
					chunk.instantiate(x, z, layer);
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

void VoxelGenerator::saveGenerationResult() {
	GET_WORLD_CONFIG(, config);
	// 读取generation db中的所有柱状生成器区块，将其转化为正方体的普通区块，并存到terrain db中
	for (size_t x = 0; x < config.width; ++x) {
		for (size_t z = 0; z < config.width; ++z) {
			std::unique_ptr<GenerationChunk> generationChunk{WorldDB::singleton().loadGenerationChunk(x, z)};


		}
	}
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
	ClassDB::bind_method(D_METHOD("start"), &VoxelGenerator::start);

	ClassDB::bind_method(D_METHOD("setBatchSize", "batch_size"), &VoxelGenerator::setBatchSize);
	ClassDB::bind_method(D_METHOD("getBatchSize"), &VoxelGenerator::getBatchSize);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "batch_size"), "setBatchSize", "getBatchSize");

	// 全部生成完毕
	ADD_SIGNAL(MethodInfo("generation_finished"));
}

} //namespace pgvoxel::generator
