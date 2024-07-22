#pragma once

#include "voxel_generation_chunk.h"

#include "scene/main/node.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace pgvoxel{

class VoxelGeneratorLayer : public Node {
	GDCLASS(VoxelGeneratorLayer, Node)
public:
	size_t getIndex() const { return index_; }
	void setIndex(const size_t index) { index_ = index; }

	void generate(Ref<VoxelGenerationChunk> chunks);
	PackedStringArray get_configuration_warnings() const override;

private:
	static void _bind_methods();

	size_t index_;
	std::unordered_map<size_t, std::unique_ptr<GenerationChunk>> cache;
};

} //namespace pgvoxel::generator
