#pragma once

#include "chunk.inl"
#include "forward.h"
#include <ostream>
#include <sstream>
#include <iostream>

namespace pgvoxel {

inline bool test_chunk() {
	std::cout << "kWidthBits" << std::endl;
	std::cout << GenerationChunk::kWidthBits << std::endl;
	std::cout << "kHeightBits" << std::endl;
	std::cout << GenerationChunk::kHeightBits << std::endl;

	bool passed{ true };

	auto chunk = GenerationChunk::create({ 0, 0, 0 });
	std::cout << "Get (0, 0, 0)" << std::endl;
	std::cout << chunk->getVoxel({ 0, 0, 0 }) << std::endl;

	std::cout << "Set (0, 0, 0) to 10" << std::endl;
	chunk->setVoxel({ 0, 0, 0 }, 10);

	std::cout << "Get (0, 0, 0)" << std::endl;
	std::cout << chunk->getVoxel({ 0, 0, 0 }) << std::endl;
	passed &= chunk->getVoxel({ 0, 0, 0 }) == 10;

	std::cout << "Set bar (1, 1) to 20" << std::endl;
	chunk->setBar(1, 1, 0, 16, 20);

	std::cout << "Get (0, 0)" << std::endl;
	std::cout << chunk->getVoxel({ 0, 0, 0 }) << std::endl;
	std::cout << "Normally get bar (1, 1) content:" << std::endl;
	for (int i = 0; i < 16; i++) {
	    std::cout << chunk->getVoxel({ 1, i, 1 });
		passed &= chunk->getVoxel({ 1, i, 1 }) == 20;
	}
	std::cout << "getBar(1, 1) content:" << std::endl;
	auto bar = chunk->getBar(1, 1, 0, 16);
	for (int i = 0; i < 16; i++) {
	    std::cout << bar[i];
		passed &= bar[i] == 20;
	}
	passed &= chunk->getVoxel({ 0, 0, 0 }) == 10;
	passed &= chunk->getVoxel({ 1, 0, 1 }) == 20;
	std::cout << chunk->toString() << std::endl;

	std::cout << "Set block (0, 0, 0) to (16, 16, 16 )" << std::endl;
	chunk->setBlock({ 0, 0, 0 }, { 16, 16, 16 }, 40);

	std::cout << "Get (0, 0, 0), (1, 0, 1), (2, 0, 2)" << std::endl;
	std::cout << chunk->getVoxel({ 0, 0, 0 }) << std::endl;
	std::cout << chunk->getVoxel({ 1, 0, 1 }) << std::endl;
	std::cout << chunk->getVoxel({ 2, 0, 2 }) << std::endl;
	passed &= chunk->getVoxel({ 0, 0, 0 }) == 40;
	passed &= chunk->getVoxel({ 1, 0, 1 }) == 40;
	passed &= chunk->getVoxel({ 2, 0, 2 }) == 40;
	std::cout << chunk->toString() << std::endl;

	std::cout << "Serializing..." << std::endl;
	std::ostringstream oss;
	chunk->serialize(oss);
	std::cout << "Get (0, 0, 0), (1, 0, 1), (2, 0, 2)" << std::endl;
	std::cout << chunk->getVoxel({ 0, 0, 0 }) << std::endl;
	std::cout << chunk->getVoxel({ 1, 0, 1 }) << std::endl;
	std::cout << chunk->getVoxel({ 2, 0, 2 }) << std::endl;
	passed &= chunk->getVoxel({ 0, 0, 0 }) == 40;
	passed &= chunk->getVoxel({ 1, 0, 1 }) == 40;
	passed &= chunk->getVoxel({ 2, 0, 2 }) == 40;
	// std::cout << chunk->toString());

	std::cout << "Deserializing..." << std::endl;
	auto deserialized = GenerationChunk::create({ 0, 0, 0 });
	std::istringstream iss(oss.str());
	deserialized->deserialize(iss, iss.str().size());

	std::cout << "Get (0, 0, 0), (1, 0, 1), (2, 0, 2) in deserialized chunk" << std::endl;
	std::cout << deserialized->getVoxel({ 0, 0, 0 }) << std::endl;
	std::cout << deserialized->getVoxel({ 1, 0, 1 }) << std::endl;
	std::cout << deserialized->getVoxel({ 2, 0, 2 }) << std::endl;
	passed &= deserialized->getVoxel({ 0, 0, 0 }) == 40;
	passed &= deserialized->getVoxel({ 1, 0, 1 }) == 40;
	passed &= deserialized->getVoxel({ 2, 0, 2 }) == 40;
	auto str = deserialized->toString();
	std::cout << str << std::endl;

	return passed;
}

} //namespace pgvoxel
