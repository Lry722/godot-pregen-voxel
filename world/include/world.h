#pragma once

#include <tbb/concurrent_unordered_map.h>
#include <glm/glm.hpp>

namespace pgvoxel {


// World 其实就是管理一些 viewer 在被添加/移动/移除时触发区块的加载和卸载
class World {
public:
	static World &instance() {
		if (!instance_) [[unlikely]] {
			instance_ = new World();
		}
		return *instance_;
	}

	void addViewer();

private:
	World() {}
	static World *instance_;
};

} //namespace pgvoxel
