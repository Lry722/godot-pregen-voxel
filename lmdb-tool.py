import lmdb

# 打开环境和数据库
env = lmdb.open("world.db", readonly=True, map_size=1073741824, subdir=False, max_dbs=8)
terrain_db = env.open_db(b"terrain")
with env.begin(write=False, db=terrain_db) as txn:
    # 获取terrain数据库的句柄

    key = b"\x00\x00\x00\x00"

    value = txn.get(key)

    if value:
        print(f"Value for key {key} is: {value}")
    else:
        print(f"No value found for key {key}")
