#include "voxel_generator.h"

#include "chunk.inl"
#include "forward.h"
#include "voxel_generation_chunk.h"
#include "voxel_generator_layer.h"
#include "world_config.h"
#include "world_db.h"

#include "core/object/class_db.h"
#include "core/string/print_string.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include "modules/pgvoxel/thirdparty/thread-pool/include/BS_thread_pool.hpp"
#include "modules/pgvoxel/thirdparty/thread-pool/include/BS_thread_pool_utils.hpp"

#include <cstddef>
#include <memory>

namespace pgvoxel {

void VoxelPreGenerator::start() {
	generator_thread_ = std::thread([&]() {
		set_current_thread_safe_for_nodes(true);
		BS::timer tmr;
		tmr.start();

		GET_WORLD_CONFIG(, config);
		// 创建临时生成器数据库
		WorldDB::singleton().beginGeneration();

		TypedArray<Node> layers = get_children();
		// 遍历每一层
		BS::thread_pool pool;
		for (int i = 0; i < layers.size(); i++) {
			auto layer = Object::cast_to<VoxelGeneratorLayer>(layers[i]);
			print_line(String("Layer {0}").format(varray(layer->get_name())));
			layer->setIndex(i);
			// 遍历所有区块
			for (size_t x = 0; x < config.width; ++x) {
				for (size_t z = 0; z < config.width; ++z) {
					(void)pool.submit_task([&, x, z]() {
						set_current_thread_safe_for_nodes(true);
						Ref<VoxelGenerationChunk> chunk;
						chunk.instantiate(x, z, layer);
						layer->generate(chunk);
						chunk->save();
						print_verbose(String("Finished {0}, {1}").format(varray(x, z)));
					});
				}
			}
			pool.wait();
		}
		// 将临时生成器数据库的中的数据保存到世界数据库中
		saveGenerationResult();
		// 删除临时数据
		WorldDB::singleton().endGeneration();
		// call_deferred("generation_finished");
		emit_signal("generation_finished");
		tmr.stop();
		print_verbose(String("Cost {0} microseconds.").format(varray(tmr.ms())));
	});
}

void VoxelPreGenerator::saveGenerationResult() {
	GET_WORLD_CONFIG(, config);
	print_verbose("Start saveing generation result...");
	// 读取generation db中的所有柱状生成器区块，将其转化为正方体的普通区块，并存到terrain db中
	BS::thread_pool pool;
	for (CoordAxis gx = 0; gx < config.width; ++gx) {
		for (CoordAxis gz = 0; gz < config.width; ++gz) {
			(void)pool.submit_task([&, gx, gz]() {
				set_current_thread_safe_for_nodes(true);
				std::unique_ptr<GenerationChunk> generationChunk{ WorldDB::singleton().loadGenerationChunk(gx, gz) };
				generationChunk->fit();
				// 以 kLoadedChunkHeight 为步长在竖直方向上对generation chunk进行分割
				for (CoordAxis gy = 0; gy < kGeneratingChunkHeight; gy += kLoadedChunkHeight) {
					auto chunk = LoadedChunk::create({ gx, gy, gz });
					for (CoordAxis x = 0; x < kLoadedChunkWidth; ++x) {
						for (CoordAxis z = 0; z < kLoadedChunkWidth; ++z) {
							chunk->setBar(x, z, 0, kLoadedChunkHeight, generationChunk->getBar(x, z, gy, gy + kLoadedChunkHeight));
						}
					}
					WorldDB::singleton().saveChunk(chunk.get());
					print_verbose(String("Finished {0}, {1}, {2}").format(varray(gx, gy, gz)));
				}
			});
			print_verbose(String("Finished {0}, {1}").format(varray(gx, gz)));
		}
	}
	pool.wait();
}

PackedStringArray VoxelPreGenerator::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	// TODO: 目前只是检测子节点不为空，实际上要检测是否包含VoxelGeneratorLayer节点
	if (get_child_count() == 0) {
		warnings.push_back(RTR("This generator has no layer. Nothing will exist in the generated world."));
	}

	return warnings;
}

void VoxelPreGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &VoxelPreGenerator::start);

	ClassDB::bind_method(D_METHOD("setBatchSize", "batch_size"), &VoxelPreGenerator::setBatchSize);
	ClassDB::bind_method(D_METHOD("getBatchSize"), &VoxelPreGenerator::getBatchSize);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "batch_size"), "setBatchSize", "getBatchSize");

	ADD_SIGNAL(MethodInfo("generation_finished"));
}

} //namespace pgvoxel
