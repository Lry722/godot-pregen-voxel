#include "voxel_generator.h"
#include "constants.h"
#include "core/error/error_macros.h"
#include "voxel_terrain_generator.h"
#include "world_config.h"

#include "core/object/class_db.h"
#include "core/object/object.h"
#include <oneapi/tbb/task_group.h>

#include <cstddef>

namespace pgvoxel {

void VoxelGenerator::_bind_methods() {
	using namespace generator;

	ClassDB::bind_method(D_METHOD("start"), &VoxelGenerator::start);

	print_line(chunk_generation_finished);
	// 区块 (x, z) 生成完毕
	ADD_SIGNAL(MethodInfo(chunk_generation_finished, PropertyInfo(Variant::INT, "x"), PropertyInfo(Variant::INT, "z")));
	// 全部生成完毕
	ADD_SIGNAL(MethodInfo(generation_finished));
}

void VoxelGenerator::add_generator(Object *p_object) {
	auto terrain_generator = Object::cast_to<VoxelTerrainGenerator>(p_object);
	if (terrain_generator) {
		terrain_generator_ = terrain_generator;
		return;
	}

	// TODO: 其他的生成器
}

void VoxelGenerator::remove_generator(Object *p_object) {
	auto terrain_generator = Object::cast_to<VoxelTerrainGenerator>(p_object);
	if (terrain_generator == terrain_generator_) {
		terrain_generator_ = nullptr;
		return;
	}

	// TODO: 其他的生成器
}

void VoxelGenerator::start() {
	using namespace generator;

	const auto &config = WorldConfig::singleton();
	if (!config.loaded()) {
		ERR_PRINT(config::kBadConfig);
		return;
	}

	const auto &config_data = config.data;

	size_t max_count = config_data.width * config_data.width;
	for (size_t i = 0; i < config_data.width; ++i) {
		for (size_t j = 0; j < config_data.width; ++j) {
			tg_.run([&, max_count, i, j]() {
				if (terrain_generator_) {
					terrain_generator_->generate(i, j);
				}
				++count;
				Object::call_deferred(chunk_generation_finished, i, j);
				// emit_signal(chunk_generation_finished, i, j);
				if (count >= max_count) {
					Object::call_deferred(generation_finished);
					// emit_signal(generation_finished);
				}
			});
		}
	}
}

PackedStringArray VoxelGenerator::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	if (terrain_generator_ == nullptr) {
		warnings.push_back(RTR("This node has no generator. Nothing will exist in the generated world."));
	}

	return warnings;
}

} //namespace pgvoxel
