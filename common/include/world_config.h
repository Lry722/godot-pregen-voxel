#pragma once

#include <dsmap-cpp/dsmap.h>
#include <string>
#include <vector>
#include "../../storage/include/forward.h"

namespace pgvoxel {

class WorldConfig {
public:

	STRUCT_WITH_PROPERTIES(Data,
						   std::string name;
						   std::int32_t seed;
						   CoordAxis width;)
	data;

public:
	static WorldConfig &singleton() {
		if (!instance_) {
			instance_ = new WorldConfig();
		}
		return *instance_;
	}

	[[nodiscard]] static bool loaded() { return loaded_; }

private:
	static inline WorldConfig *instance_ = nullptr;
	WorldConfig();

	static inline bool loaded_{ false };
};

#define GET_WORLD_CONFIG(err_retval, name) \
const auto &name = WorldConfig::singleton().data;\
if (!WorldConfig::loaded()) [[unlikely]] {\
	ERR_PRINT("The world config was not loaded correctly.");\
	return err_retval;\
}

} // namespace pgvoxel
