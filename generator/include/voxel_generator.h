#pragma once

#include "scene/main/node.h"

#include <cstdint>
#include <atomic>
#include <oneapi/tbb/task_group.h>

namespace pgvoxel {

class VoxelTerrainGenerator;

class VoxelGenerator : public Node {
	GDCLASS(VoxelGenerator, Node)
public:
	~VoxelGenerator() noexcept { tg_.wait(); }

	void start();
	void add_generator(Object *p_object);
	void remove_generator(Object *p_object);
	PackedStringArray get_configuration_warnings() const override;

private:
	static void _bind_methods();

	VoxelTerrainGenerator *terrain_generator_{nullptr};
	// uint64_t static_entity_generator_{};
	// uint64_t dynamic_enrity_generator_{};

	std::atomic<uint32_t> count{0};
	tbb::task_group tg_{};
};

} //namespace pgvoxel
