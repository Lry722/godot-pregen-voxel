#include <iostream>
#include "world_db.h"
#include "palette.h"
#include <sstream>

int main(int argc, char const *argv[])
{
    auto &world_db = lry::WorldDB::instance();
    auto c1{lry::Chunk::create(0, 0)};
    c1->setVoxel({0, 0, 1}, 1);
    c1->setVoxel({0, 1, 0}, 2);
    c1->setVoxel({1, 0, 0}, 3);
    world_db.saveChunk(c1.get());
    auto c2 = world_db.loadChunk(0, 0);

    return 0;
}
