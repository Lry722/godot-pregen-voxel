env = Environment(CPPPATH=["."], LIBS=["yaml-cpp", "spdlog", "lmdb", "dsmap", "fmt"], CCFLAGS=["-g", "-O0", "--std=c++20"])

includes = Glob('**/include')
print(includes)
env['CPPPATH'] = includes

src = Glob('**/*.cpp') + ['main.cpp']
print(src)

env.Program('out', src)