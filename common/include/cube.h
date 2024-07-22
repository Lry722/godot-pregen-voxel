#pragma once

#include "forward.h"
#include "modules/pgvoxel/storage/include/forward.h"
#include <array>
#include <glm/ext/vector_float3.hpp>
#include <utility>

namespace pgvoxel {

enum Side {
	SIDE_LEFT = 0,
	SIDE_RIGHT,
	SIDE_BOTTOM,
	SIDE_TOP,
	SIDE_BACK,
	SIDE_FRONT,
	// SIDE_COUNT 也可用于表示内部
	SIDE_COUNT
};

static const std::array<glm::vec3, SIDE_COUNT> kSideNormalLut {
	glm::vec3{-1 ,0, 0},
	glm::vec3{1 ,0, 0},
	glm::vec3{0 ,-1, 0},
	glm::vec3{0 ,1, 0},
	glm::vec3{0 ,0, -1},
	glm::vec3{0 ,0, 1}
};

static const std::array<Side, SIDE_COUNT> kOppositeSideLut {
    Side::SIDE_RIGHT,
	Side::SIDE_LEFT,
	Side::SIDE_TOP,
	Side::SIDE_BOTTOM,
	Side::SIDE_FRONT,
	Side::SIDE_BACK
};

enum SideAxis {
	SIDE_NEGATIVE_X = 0,
	SIDE_POSITIVE_X,
	SIDE_NEGATIVE_Y,
	SIDE_POSITIVE_Y,
	SIDE_NEGATIVE_Z,
	SIDE_POSITIVE_Z,

	SIDE_AXIS_COUNT
};

enum Edge {
	EDGE_BOTTOM_BACK = 0,
	EDGE_BOTTOM_RIGHT,
	EDGE_BOTTOM_FRONT,
	EDGE_BOTTOM_LEFT,
	EDGE_BACK_LEFT,
	EDGE_BACK_RIGHT,
	EDGE_FRONT_RIGHT,
	EDGE_FRONT_LEFT,
	EDGE_TOP_BACK,
	EDGE_TOP_RIGHT,
	EDGE_TOP_FRONT,
	EDGE_TOP_LEFT,

	EDGE_COUNT
};

enum Corner {
	CORNER_BOTTOM_BACK_LEFT = 0,
	CORNER_BOTTOM_BACK_RIGHT,
	CORNER_BOTTOM_FRONT_RIGHT,
	CORNER_BOTTOM_FRONT_LEFT,
	CORNER_TOP_BACK_LEFT,
	CORNER_TOP_BACK_RIGHT,
	CORNER_TOP_FRONT_RIGHT,
	CORNER_TOP_FRONT_LEFT,

	CORNER_COUNT
};



} //namespace pgvoxel
