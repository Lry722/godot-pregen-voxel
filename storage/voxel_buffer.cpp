#include "voxel_buffer.h"
#include "core/object/class_db.h"
#include "forward.h"
#include <cstdint>
#include <vector>

namespace pgvoxel {

void VoxelBuffer::set_voxel(const Vector3i &pos, const int32_t data) {
	data_.setVoxel(toCoord(pos), data);
}

int32_t VoxelBuffer::get_voxel(const Vector3i &pos) const {
	return data_.getVoxel(toCoord(pos));
}

void VoxelBuffer::set_bar(const int32_t x, const int32_t z, const int32_t buttom, const int32_t top, const int32_t data) {
	data_.setBar(x, z, buttom, top, data);
}

void VoxelBuffer::set_block(const Vector3i &begin, const Vector3i &end, const VoxelData data) {
	data_.setBlock(toCoord(begin), toCoord(end), data);
}

void VoxelBuffer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_voxel", "pos", "data"), &VoxelBuffer::set_voxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "pos"), &VoxelBuffer::get_voxel);

	ClassDB::bind_method(D_METHOD("set_bar", "x", "z", "buttom", "top", "data"), &VoxelBuffer::set_bar);
	ClassDB::bind_method(D_METHOD("set_block", "begin", "end", "data"), &VoxelBuffer::set_block);

	ClassDB::bind_method(D_METHOD("init", "width", "height", "depth"), &VoxelBuffer::init);
}

} //namespace pgvoxel
