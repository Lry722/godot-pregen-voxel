#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace pgvoxel {

// ValueType 应当为无符号整数
template <typename ValueType = uint32_t>
class PackedArray {
   public:
    class Access;
    friend class Access;

    class iterator;
    class const_iterator;

    typedef Access reference;
	typedef ValueType value_type;

    typedef int32_t size_type;

   public:
    PackedArray(const size_type size = 0, const uint8_t element_bit_width = 0) noexcept
        : element_bit_width_(element_bit_width),
          size_(size),
          element_capacity_((1ULL << element_bit_width_) - 1),
          data_((size * element_bit_width_ + kUnitBitWidth - 1) / kUnitBitWidth) {}
    size_type size() const { return size_; }
    ValueType elementCapacity() const { return element_capacity_; }
    bool empty() const { return size_ == 0; }

    void resize(const size_type size);
    void push_back(const ValueType value);

    ValueType get(const size_type index) const;
    void set(const size_type index, const ValueType value);
    void setRange(const size_type begin, const size_type end, const ValueType value);
    void setRange(const size_type begin, const size_type end, const std::vector<ValueType> &values);
    std::vector<ValueType> getRange(const size_type begin, const size_type end) const;

    void transformTo(const uint8_t element_size);
    void grow();
    void fit();

    // 序列化/反序列化
    void serialize(std::ostringstream &oss) const;
    void deserialize(std::istringstream &iss, const uint32_t size);

    std::string toString() const;

    Access operator[](const int32_t index);
    ValueType operator[](const int32_t index) const;

    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;

   private:
    static inline const int8_t kUnitBitWidth = sizeof(ValueType) * CHAR_BIT;

    uint8_t element_bit_width_;
    size_type size_;
    inline auto indexOf(const size_type index) const { return std::tuple{(index * element_bit_width_) / kUnitBitWidth, (index * element_bit_width_) % kUnitBitWidth}; }

    ValueType element_capacity_;
    std::vector<ValueType> data_;
};

template <typename ValueType>
class PackedArray<ValueType>::Access {
    PackedArray &data_;
    const PackedArray<ValueType>::size_type index_;

   public:
    Access(PackedArray &data, decltype(index_) index) : data_(data), index_(index) {}
    Access &operator=(const ValueType value) {
        data_.set(index_, value);
        return *this;
    }
    operator ValueType() const { return data_.get(index_); }
};

template <typename ValueType>
class PackedArray<ValueType>::iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = PackedArray<ValueType>::size_type;
    using reference = Access;

    iterator(PackedArray& data, const PackedArray<ValueType>::size_type index) : data_(data), index_(index) {}

    reference operator*() const { return data_[index_]; }

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

    bool operator==(const iterator &other) const { return index_ == other.index_; }
    bool operator<(const iterator &other) const { return index_ < other.index_; }

   private:
    PackedArray& data_;
    PackedArray<ValueType>::size_type index_;
};

template <typename ValueType>
class PackedArray<ValueType>::const_iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using reference = ValueType;

    const_iterator(const PackedArray& data, const PackedArray<ValueType>::size_type index) : data_(data), index_(index) {}

    reference operator*() const { return data_[index_]; }

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

    bool operator==(const const_iterator &other) const { return index_ == other.index_; }
    bool operator<(const const_iterator &other) const { return index_ < other.index_; }

   private:
    const PackedArray& data_;
    PackedArray<ValueType>::size_type index_;
};

template <typename ValueType>
inline PackedArray<ValueType>::reference PackedArray<ValueType>::operator[](const PackedArray<ValueType>::size_type index) {
    return Access(*this, index);
}
template <typename ValueType>
inline ValueType PackedArray<ValueType>::operator[](const PackedArray<ValueType>::size_type index) const {
    return get(index);
}

template <typename ValueType>
inline PackedArray<ValueType>::iterator PackedArray<ValueType>::begin() {
    return iterator(this, 0);
}
template <typename ValueType>
inline PackedArray<ValueType>::iterator PackedArray<ValueType>::end() {
    return iterator(this, size_);
}
template <typename ValueType>
inline PackedArray<ValueType>::const_iterator PackedArray<ValueType>::cbegin() const {
    return const_iterator(this, 0);
}
template <typename ValueType>
inline PackedArray<ValueType>::const_iterator PackedArray<ValueType>::cend() const {
    return const_iterator(this, size_);
}

}  // namespace pgvoxel
