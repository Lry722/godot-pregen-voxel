#pragma once

#include "buffer.h"
#include "core/io/resource.h"
#include "voxel_block_library.h"
#include "voxel_buffer.h"
#include <sys/types.h>

namespace pgvoxel {

class VoxelMesher : public Resource {
	GDCLASS(VoxelMesher, Resource)
public:
	void set_library(Ref<VoxelBlockLibrary> library) { library_ = library; }
	Ref<VoxelBlockLibrary> get_library() const { return library_; }

	void set_padding(bool padding) { padding_ = padding; }
	bool get_padding() const { return padding_; }

	// 根据给定的体素数据构建 Mesh
	Ref<ArrayMesh> build(const Buffer &data);
	// 暴露给 GDScript 的接口
	Ref<ArrayMesh> gds_build(Ref<VoxelBuffer> data);

private:
	static void _bind_methods();
	bool is_face_visible(const Model &target_block, uint32_t neighbor_block_id, Side side) const;

	Ref<VoxelBlockLibrary> library_;

	// 在构建区块时，会希望对区块相邻的部分也进行面剔除，因此需要在六个方向上各填充一层相邻区块的数据
	// 如果不是在构建区块，而是单纯希望用体素数据构建 Mesh，可以将 padding 关闭
	bool padding_{ true };
};

} //namespace pgvoxel
