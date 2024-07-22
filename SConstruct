env = Environment(LIBS=["yaml-cpp", "lmdb", "dsmap", "lz4", "tbb"],CCFLAGS=['-fexceptions', '--std=c++20', '-g'])

includes = Glob('**/include') + ['/home/admin/Documents/C++/godot/']
sources = ['tests/test.cpp', 'storage/packed_array.cpp'] 

env.Tool('compilation_db')
env.CompilationDatabase()

env.Program(target='test.out', source=sources)
