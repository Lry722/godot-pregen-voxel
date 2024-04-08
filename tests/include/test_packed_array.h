#pragma once

#include "packed_array.h"
#include <format>
#include <iostream>

namespace pgvoxel {

inline bool test_packed_array() {
	PackedArray a(10);
	a.transform(2);
	std::cout << "setRange (0, 5) to 1" << std::endl;
	a.setRange(0, 5, 1);
	std::cout << "setRange (1, 9) to 2" << std::endl;
	a.setRange(1, 9, 2);
	std::vector<size_t> v = { 1, 2, 2, 2, 2, 2, 2, 2, 2, 0 };
	bool passed{ true };
	std::cout << "Normally get a in [0, 10]:" << std::endl;
	for (size_t i = 0; i < v.size(); ++i) {
		std::cout << std::format("a: {} v: {}", a.get(i), v[i]).c_str() << std::endl;
		passed &= (v[i] == a[i]);
	}
	std::cout << "getRange(0, 10)" << std::endl;
	for (const auto elem : a.getRange(0, 10)) {
		std::cout << elem << std::endl;
	}
	std::cout << "Serializing..." << std::endl;
	std::ostringstream oss;
	a.serialize(oss);
	PackedArray b;
	std::istringstream iss(oss.str());
	std::cout << "Deserializing..." << std::endl;
	b.deserialize(iss, oss.str().size());
	std::cout << b.toString().c_str() << std::endl;
	std::cout << "Normally get b in [0, 10]:" << std::endl;
	for (size_t i = 0; i < v.size(); ++i) {
		std::cout << std::format("b: {} v: {}", b.get(i), v[i]).c_str() << std::endl;
		passed &= (v[i] == b[i]);
	}

	return passed;
}

} //namespace pgvoxel
