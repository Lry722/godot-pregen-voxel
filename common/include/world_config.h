#pragma once

#include <dsmap-cpp/dsmap.h>
#include <string>
#include <vector>

namespace pgvoxel {

class WorldConfig {
public:

	STRUCT_WITH_PROPERTIES(Data,
						   std::string name;
						   std::int32_t seed;
						   std::size_t width;)
	data;

public:
	static WorldConfig &singleton() {
		if (!instance_) {
			instance_ = new WorldConfig();
		}
		return *instance_;
	}

	[[nodiscard]] bool loaded() const { return successful; }

private:
	static inline WorldConfig *instance_ = nullptr;
	WorldConfig();

	bool successful{ false };
};
} // namespace pgvoxel
