env = Environment(CPPPATH=["."], LIBS=["yaml-cpp", "spdlog", "lmdb", "dsmap", "fmt", "lz4"], CCFLAGS=["-g", "-O0", "--std=c++20"])

includes = Glob('**/include')
env['CPPPATH'] = includes

src = Glob('**/*.cpp') + ['main.cpp']

env.Program('out', src)