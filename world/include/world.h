#pragma once

#include "chunk.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <tbb/concurrent_unordered_map.h>

namespace lry
{

    // World 其实就是管理一些 viewer 在被添加/移动/移除时触发区块的加载和卸载
    class World
    {
    public:
        static World &instance()
        {
            if (!instance_)
            {
                instance_ = new World();
            }
            return *instance_;
        }

        void addViewer();

    private:
        World() {}
        static World *instance_;

        tbb::concurrent_unordered_map<size_t, LoadedChunk> chunks;
        tbb::concurrent_unordered_map<size_t, Viewer::ViewInfo> view_infos;
    };

} // namespace lry
