#pragma once
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "scene/main/node.h"
#include "test_packed_array.h"
#include <stdexcept>

namespace pgvoxel {

#define TEST(x)                                                     \
	if (targets.find(#x) != -1) {                                   \
		print_line("Running " #x " test...");                       \
		try {                                                       \
			if (test_##x())                                         \
				print_line("Pass " #x " test.");                    \
			else                                                    \
				print_line("Failed " #x " test!");                  \
		} catch (const std::exception &e) {                         \
			print_line("Error occured in " #x " test: ", e.what()); \
		}                                                           \
	}

class VoxelTest : public Object {
	GDCLASS(VoxelTest, Object)
public:
	static void run(const PackedStringArray &targets) {
		TEST(packed_array)
	}

private:
	static void _bind_methods() {
		ClassDB::bind_static_method("VoxelTest", D_METHOD("run", "targets"), &VoxelTest::run);
	}
};

} //namespace pgvoxel
