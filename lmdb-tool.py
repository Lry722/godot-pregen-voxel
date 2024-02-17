import lmdb

env = lmdb.open("world.db", map_size = 1000 * 1000, subdir = False)
with env.begin(write = False) as txn:
    key = b'\x00\x00\x00\x00'
    value = txn.get(key)
    if value is not None:
        print(value)