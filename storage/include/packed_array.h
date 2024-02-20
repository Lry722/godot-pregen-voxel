#pragma once

#include "constants.h"

#include <climits>
#include <concepts>
#include <cstdint>
#include <string>
#include <vector>

namespace pgvoxel {
template <typename T = std::uint32_t>
	requires std::is_unsigned_v<T>
class PackedArray {
public:
	class Access;
	friend class Access;

	class iterator;
	class const_iterator;

	typedef T value_type;
	typedef Access reference;

public:
	PackedArray(const size_t size = 0, const size_t element_size = 4) noexcept :
			element_size_(element_size), size_(size), mask_((1ULL << element_size_) - 1), data_((size * element_size_ + kUnitSize - 1) / kUnitSize) {}

	size_t size() const { return size_; }
	size_t elementCapacity() const { return mask_; }
	bool empty() const { return size_ == 0; }
	size_t memory() const { return data_.size() * kUnitSize; }

	void resize(const size_t size);
	void push_back(const T value);

	T get(const size_t index) const;
	void set(const size_t index, const T value);

	void transform(const size_t element_size);
	void grow();
	void fit();

	// 序列化
	void serialize(std::ostringstream &oss);
	// 反序列化，返回值表示读取到buffer中哪个位置
	void deserialize(std::istringstream &iss, const size_t size);

	Access operator[](const size_t index) { return Access(*this, index); }
	T operator[](const size_t index) const { return get(index); }

	iterator begin() { return iterator(this, 0); }
	iterator end() { return iterator(this, size_); }
	const_iterator cbegin() const { return const_iterator(this, 0); }
	const_iterator cend() const { return const_iterator(this, size_); }

private:
	// 当UnitSize为32时，该bitarray最多支持64位的ElementSize
	// 若UnitSize为16，则最多支持32位ElementSize，以此类推
	static inline const size_t kUnitSize = sizeof(T) * CHAR_BIT;
	size_t element_size_;
	size_t size_;

	// Mask 参与的表达式都会自动转为ull，并在最后转为T时截断高位不需要的内容
	std::uint64_t mask_;

	std::vector<T> data_;
};
} // namespace pgvoxel
