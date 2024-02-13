#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace lry {

using GlobalPosition = glm::vec<3, std::int32_t>;

class World {
  public:
    static World &getInstance() { 
        if (!instance_) {
            instance_ = new World();
        }
        return *instance_;
    }

  private:
    static World *instance_;

    World() {}
};
} // namespace lry
