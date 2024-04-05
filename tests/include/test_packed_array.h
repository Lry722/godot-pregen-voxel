#pragma once

#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "packed_array.inl"

namespace pgvoxel {

inline bool test_packed_array() {
	PackedArray<> a(10);
	a.setRange(0, 5, 1);
	a.setRange(1, 6, 2);
	std::vector<size_t> v = {1, 2, 2, 2, 2, 2, 0};
	for (size_t i = 0; i < v.size(); ++i) {
		print_line(std::format("a: {} v: {}", a.get(i), v[i]).c_str());
		if (v[i] != a[i]) {
			return false;
		}
	}
	print_line("Array content:");
	for (const auto elem : a.getRange(0, 7)) {
		print_line(Variant(elem));
	}
	return true;
}

}


