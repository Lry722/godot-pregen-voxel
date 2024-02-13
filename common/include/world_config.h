#pragma once

#include <string>
#include <vector>
#include <dsmap-cpp/dsmap.h>

namespace lry {

class WorldConfig {
public:
  STRUCT_WITH_PROPERTIES (Data, 
    std::string name{"NONE"};
    int32_t seed{722};
    size_t height{512};
    size_t width{1024};
  ) data;

public:
  static WorldConfig &instance() {
    if (!instance_) {
      instance_ = new WorldConfig();
    }
    return *instance_;
  }

  [[nodiscard]] bool loaded() const { return successful; }

private:
  static inline WorldConfig *instance_ = nullptr;
  WorldConfig();

  bool successful{false};
};
} // namespace lry