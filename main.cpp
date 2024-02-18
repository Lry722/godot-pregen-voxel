#include <iostream>
#include "world_db.h"
#include "palette.h"
#include <sstream>

int main(int argc, char const *argv[])
{
    auto &world_db = lry::WorldDB::instance();
    for (int i = 0; i < 1; ++i)
        for (int j = 0; j < 1; ++j)
        {
            auto chunk{lry::Chunk::create(i, j)};
            chunk->setVoxel({0, 0, 1}, 1);
            chunk->setVoxel({0, 1, 0}, 2);
            chunk->setVoxel({1, 0, 0}, 3);
            world_db.saveChunk(chunk.get());
        }
    auto chunk = world_db.loadChunk(0, 0);

    return 0;
}
