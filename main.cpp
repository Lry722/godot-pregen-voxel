#include <iostream>
#include "world_db.h"
#include "palette.h"
#include <sstream>

int main(int argc, char const *argv[])
{
    auto &world_db = lry::WorldDB::instance();
    auto c1{std::make_unique<lry::Chunk>(0, 0)};
    world_db.saveChunk(c1.get());
    auto c2 = world_db.loadChunk(0, 0);

    return 0;
}
