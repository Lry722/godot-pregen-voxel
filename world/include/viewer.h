#pragma once

#include "constants.h"

namespace lry
{
    // 用于触发区块加载/卸载
    struct Viewer
    {
        // 加载等级，高级的加载等级会包含低级等级的内容
        enum class LoadLevel
        {
            kData,
            kTerrain,
            kCollision,
            kStaticEntity,
            kDynamicEntity,
            kFull
        };
        // 用于记录一个区块被view的状态
        struct ViewInfo
        {
            std::uint8_t data{0};
            std::uint8_t terrain{0};
            std::uint8_t collision{0};
            std::uint8_t static_entity{0};
            std::uint8_t dynamic_entity{0};
        };

        void view(ViewInfo &info) const
        {
            switch (load_level)
            {
            case LoadLevel::kFull:
            case LoadLevel::kDynamicEntity:
                ++info.dynamic_entity;
            case LoadLevel::kStaticEntity:
                ++info.static_entity;
            case LoadLevel::kCollision:
                ++info.collision;
            case LoadLevel::kTerrain:
                ++info.terrain;
            case LoadLevel::kData:
                ++info.data;
            }
        }

        void unview(ViewInfo &info) const
        {
            switch (load_level)
            {
            case LoadLevel::kFull:
            case LoadLevel::kDynamicEntity:
                --info.dynamic_entity;
            case LoadLevel::kStaticEntity:
                --info.static_entity;
            case LoadLevel::kCollision:
                --info.collision;
            case LoadLevel::kTerrain:
                --info.terrain;
            case LoadLevel::kData:
                --info.data;
            }
        }

        size_t x_, z_;
        size_t view_distance_;
        LoadLevel load_level;
    };

} // namespace lry
