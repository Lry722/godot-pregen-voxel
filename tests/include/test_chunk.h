#pragma once

#include "chunk.h"
#include "core/math/math_defs.h"
#include "core/string/print_string.h"
#include "forward.h"
#include <bit>

namespace pgvoxel {

inline bool test_chunk() {
	print_line("kWidthBits");
	print_line(GenerationChunk::kWidthBits);
	print_line(std::bit_width(kGeneratingChunkWidth));
	print_line("kHeightBits");
	print_line(GenerationChunk::kHeightBits);
	print_line(std::bit_width(kGeneratingChunkHeight));

	return true;
}

}

