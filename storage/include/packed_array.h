#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace pgvoxel {

class PackedArray {
public:
	class Access;
	friend class Access;

	class iterator;
	class const_iterator;

	typedef uint32_t value_type;
	typedef Access reference;

public:
	PackedArray(const int32_t size = 0, const int8_t element_bit_width = 0) noexcept :
			element_bit_width_(element_bit_width), size_(size), element_capacity_((1ULL << element_bit_width_) - 1), data_((size * element_bit_width_ + kUnitBitWidth - 1) / kUnitBitWidth) {}
	int32_t size() const { return size_; }
	uint64_t elementCapacity() const { return element_capacity_; }
	bool empty() const { return size_ == 0; }

	void resize(const int32_t size);
	void push_back(const value_type value);

	value_type get(const int32_t index) const;
	void set(const int32_t index, const value_type value);
	void setRange(const int32_t begin, const int32_t end, const value_type value);
	void setRange(const int32_t begin, const int32_t end, const std::vector<value_type> &values);
	std::vector<value_type> getRange(const int32_t begin, const int32_t end) const;

	void transform(const int8_t element_size);
	void grow();
	void fit();

	// 序列化
	void serialize(std::ostringstream &oss) const;
	// 反序列化
	void deserialize(std::istringstream &iss, const int32_t size);
	std::string toString() const;

	Access operator[](const int32_t index);
	value_type operator[](const int32_t index) const;

	iterator begin();
	iterator end();
	const_iterator cbegin() const;
	const_iterator cend() const;

private:
	static inline const int8_t kUnitBitWidth = sizeof(value_type) * CHAR_BIT;

	int8_t element_bit_width_;
	int32_t size_;
	inline auto indexOf(const int32_t index) const {
		return std::tuple{ (index * element_bit_width_) / kUnitBitWidth, (index * element_bit_width_) % kUnitBitWidth };
	}

	std::uint64_t element_capacity_;
	std::vector<value_type> data_;
};

class PackedArray::Access {
	PackedArray &data_;
	const int32_t index_;
	const value_type value_;

public:
	Access(PackedArray &data, int32_t index) :
			data_(data), index_(index), value_(data.get(index)) {}
	Access &operator=(const value_type value) {
		if (value != value_) {
			data_.set(index_, value);
		}
		return *this;
	}
	operator value_type() const {
		return value_;
	}
};

class PackedArray::iterator {
public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = PackedArray::value_type;
	using reference = Access;

	iterator(PackedArray *array, const int32_t index) :
			array_(array), index_(index) {}

	reference operator*() const { return (*array_)[index_]; }

	iterator &operator++() {
		++index_;
		return *this;
	}
	iterator operator++(int) {
		iterator tmp(*this);
		operator++();
		return tmp;
	}

	iterator &operator--() {
		--index_;
		return *this;
	}
	iterator operator--(int) {
		iterator tmp(*this);
		operator--();
		return tmp;
	}

	iterator &operator+=(difference_type n) {
		index_ += n;
		return *this;
	}
	iterator &operator-=(difference_type n) {
		index_ -= n;
		return *this;
	}

	friend iterator operator+(iterator it, difference_type n) { return it += n; }
	friend iterator operator+(difference_type n, iterator it) { return it += n; }
	friend iterator operator-(iterator it, difference_type n) { return it -= n; }

	friend difference_type operator-(iterator lhs, iterator rhs) { return lhs.index_ - rhs.index_; }

	bool operator==(const iterator &other) const { return array_ == other.array_ && index_ == other.index_; }
	bool operator<(const iterator &other) const { return index_ < other.index_; }

private:
	PackedArray *array_;
	int32_t index_;
};

class PackedArray::const_iterator {
public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = PackedArray::value_type;
	using reference = value_type;

	const_iterator(const PackedArray *array, const int32_t index) :
			array_(array), index_(index) {}

	reference operator*() const { return (*array_)[index_]; }

	const_iterator &operator++() {
		++index_;
		return *this;
	}
	const_iterator operator++(int) {
		const_iterator tmp(*this);
		operator++();
		return tmp;
	}

	const_iterator &operator--() {
		--index_;
		return *this;
	}
	const_iterator operator--(int) {
		const_iterator tmp(*this);
		operator--();
		return tmp;
	}

	const_iterator &operator+=(difference_type n) {
		index_ += n;
		return *this;
	}
	const_iterator &operator-=(difference_type n) {
		index_ -= n;
		return *this;
	}

	friend const_iterator operator+(const_iterator it, difference_type n) { return it += n; }
	friend const_iterator operator+(difference_type n, const_iterator it) { return it += n; }
	friend const_iterator operator-(const_iterator it, difference_type n) { return it -= n; }

	friend difference_type operator-(const_iterator lhs, const_iterator rhs) { return lhs.index_ - rhs.index_; }

	bool operator==(const const_iterator &other) const { return array_ == other.array_ && index_ == other.index_; }
	bool operator<(const const_iterator &other) const { return index_ < other.index_; }

private:
	const PackedArray *array_;
	int32_t index_;
};

inline PackedArray::Access PackedArray::operator[](const int32_t index) { return Access(*this, index); }
inline PackedArray::value_type PackedArray::operator[](const int32_t index) const { return get(index); }

inline PackedArray::iterator PackedArray::begin() { return iterator(this, 0); }
inline PackedArray::iterator PackedArray::end() { return iterator(this, size_); }
inline PackedArray::const_iterator PackedArray::cbegin() const { return const_iterator(this, 0); }
inline PackedArray::const_iterator PackedArray::cend() const { return const_iterator(this, size_); }

} // namespace pgvoxel
