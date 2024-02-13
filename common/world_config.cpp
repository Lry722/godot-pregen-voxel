#include "world_config.h"
#include "constants.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/yaml.h>
#include <dsmap-cpp/yaml.h>

namespace lry
{

    WorldConfig::WorldConfig()
    {
        using namespace config;

        YAML::Node config = YAML::LoadFile(filename::kConfig);
        if (!config.IsNull())
        {
            spdlog::info("Start loading configuration from file {}", filename::kConfig);
            try
            {
                dsmap::yaml2struct(config["config"], data);
            }
            catch (const YAML::Exception &e)
            {
                spdlog::error(std::format("Failed to load configuration from file {}\n{}",
                                          filename::kConfig, e.what()));
                return;
            }
            successful = true;
            spdlog::info("Succeed loading configuration from file {}", filename::kConfig);
        }
    }

} // namespace lry