env = Environment(LIBS=["yaml-cpp", "lmdb", "dsmap", "lz4"], CCFLAGS=["-g", "-O0", "--std=c++20"])

includes = Glob('**/include', exclude=['register_types.cpp'])
env['CPPPATH'] = includes

src = Glob('**/*.cpp')

env.Tool('compilation_db')
cdb = env.CompilationDatabase()
Alias('cdb', cdb)

env.Program('out', src)
