#include "test_packed_array.h"

#include "constants.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "packed_array.inl"
#include <cstddef>
#include <vector>

namespace pgvoxel {

bool test_packed_array() {
	storage::PackedArray<> a(5);
	a.setRange(0, 5, 1);
	a.setRange(1, 6, 2);
	std::vector<size_t> v = {1, 2, 2, 2, 2, 2};
	for (size_t i = 0; i < a.size(); ++i) {
		print_line(std::format("a: {} v: {}", a.get(i), v[i]).c_str());
		if (v[i] != a[i]) {
			return false;
		}
	}
	return true;
}

} //namespace pgvoxel
