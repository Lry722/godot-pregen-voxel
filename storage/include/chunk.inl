#pragma once

#include "chunk.h"
#include "data_chunk.h"
#include <memory>

namespace pgvoxel {

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::setVoxel(const Coord& pos, const VoxelData data, uint8_t layer) {
    dataChunks_[layer].setVoxel(pos, data);
}

template <CoordAxis Width, CoordAxis Height>
VoxelData Chunk<Width, Height>::getVoxel(const Coord& pos, uint8_t layer) const {
    return dataChunks_[layer].getVoxel(pos);
}

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::setBar(const Coord& pos, const std::vector<VoxelData>& data, uint8_t layer) {
    dataChunks_[layer].setBar(pos, data);
}

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data, uint8_t layer) {
    dataChunks_[layer].setBar(x, z, buttom, top, data);
}

template <CoordAxis Width, CoordAxis Height>
std::vector<VoxelData> Chunk<Width, Height>::getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, uint8_t layer) const {
    return dataChunks_[layer].getBar(x, z, buttom, top);
}

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::setBlock(const Coord& begin, const Coord& end, const VoxelData data, uint8_t layer) {
    dataChunks_[layer].setBlock(begin, end, data);
}

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::setBlock(const Coord& pos, const Buffer& data, uint8_t layer) {
    dataChunks_[layer].setBlock(pos, data);
}

template <CoordAxis Width, CoordAxis Height>
Buffer Chunk<Width, Height>::getBlock(const Coord& begin, const Coord& end, uint8_t layer) const {
    return dataChunks_[layer].getBlock(begin, end);
}
template <CoordAxis Width, CoordAxis Height>
std::unique_ptr<Chunk<Width, Width>> slice(const Chunk<Width, Height>* const sourceChunk, int index) {
	const auto& pos = sourceChunk->getPosition();
	auto targetChunk = LoadedChunk::create({pos.x, index * Width, pos.z});
	for (CoordAxis x = 0; x < Width; ++x) {
		for (CoordAxis z = 0; z < Width; ++z) {
			targetChunk->setBar({x, 0, z}, sourceChunk->getBar(x, z, index * Width, (index + 1) * Width));
		}
	}

	return targetChunk;
}

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::serialize(std::ostringstream &oss) const {
	for (const auto& dataChunk: dataChunks_) {
		oss << dataChunk;
	}

}

template <CoordAxis Width, CoordAxis Height>
void Chunk<Width, Height>::deserialize(std::istringstream &iss, const uint32_t size) {
	for (auto& dataChunk: dataChunks_) {
		iss >> dataChunk;
	}
}

}  // namespace pgvoxel
