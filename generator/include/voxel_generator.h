#pragma once

#include "scene/main/node.h"

#include <oneapi/tbb/task_group.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>

namespace pgvoxel::generator {

class VoxelGeneratorLayer;

class VoxelGenerator : public Node {
	GDCLASS(VoxelGenerator, Node)
public:
	void start();

	size_t getBatchSize() const { return batch_size_; }
	void setBatchSize(size_t batch_size) { batch_size_ = batch_size; }

	PackedStringArray get_configuration_warnings() const override;

private:
	static void _bind_methods();

	size_t batch_size_ = 4;
	std::thread generator_thread_;
};

} //namespace pgvoxel::generator
