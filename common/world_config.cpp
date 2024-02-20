#include "world_config.h"
#include "constants.h"
#include "core/error/error_macros.h"
#include "core/string/print_string.h"

#include <dsmap-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/yaml.h>
#include <format>

namespace pgvoxel {

WorldConfig::WorldConfig() {
	using namespace config;

	YAML::Node config = YAML::LoadFile(filename::kConfig);
	if (!config.IsNull()) {
		try {
			dsmap::yaml2struct(config["config"], data);
		} catch (const YAML::Exception &e) {
			ERR_PRINT("Failed to load configuration from file.");
			return;
		}
		successful = true;
		print_verbose("Succeed loading configuration from file.");
	}
}

} // namespace pgvoxel
