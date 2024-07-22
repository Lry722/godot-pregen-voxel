#include "packed_array.h"
#include "../../common/include/serialize.h"
#include <algorithm>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace pgvoxel {

template<typename ValueType>
void PackedArray<ValueType>::resize(const size_type size) {
	size_ = size;
	data_.resize((size_ * element_bit_width_ + kUnitBitWidth - 1) / kUnitBitWidth);
}

template<typename ValueType>
void PackedArray<ValueType>::push_back(const ValueType value) {
	resize(size_ + 1);
	set(size_ - 1, value);
}

template<typename ValueType>
ValueType PackedArray<ValueType>::get(const size_type index) const {
	if (index < 0 || index >= size_) [[unlikely]] {
		throw std::out_of_range(std::format("PackedArray: index {} out of range!", index));
	}

	if (element_bit_width_ == 0) {
		return 0;
	}

	const auto [index_in_data, index_in_unit] = indexOf(index);

	// 读取当前unit中的位
	ValueType result{ static_cast<ValueType>((data_[index_in_data] >> index_in_unit) & element_capacity_) };
	if (index_in_unit + element_bit_width_ > kUnitBitWidth) {
		// 读取超出当前unit的位
		result |= ((data_[index_in_data + 1] << index_in_unit) & element_capacity_);
	}

	return result;
}

template<typename ValueType>
void PackedArray<ValueType>::set(const size_type index, const ValueType value) {
	if (index < 0 || index >= size_) [[unlikely]] {
		throw std::out_of_range(std::format("PackedArray: index {} out of range!", index));
	}

	const auto [index_in_data, index_in_unit] = indexOf(index);

	// 写入当前unit中的位
	data_[index_in_data] = (data_[index_in_data] & ~(element_capacity_ << index_in_unit)) | (value << index_in_unit);
	if (index_in_unit + element_bit_width_ > kUnitBitWidth) {
		// 写入超出当前unit的位
		data_[index_in_data + 1] = (data_[index_in_data + 1] & ~(element_capacity_ >> (kUnitBitWidth - index_in_unit))) | (value >> (kUnitBitWidth - index_in_unit));
	}
}

template<typename ValueType>
void PackedArray<ValueType>::setRange(const size_type begin, const size_type end, const ValueType data) {
	if (end > size_) [[unlikely]] {
		throw std::out_of_range(std::format("PackedArray: range ({}, {}) to set out of range!", begin, end));
	}

	auto [index_in_data, index_in_unit] = indexOf(begin);

	for (int32_t i = 0; i < end - begin; ++i) {
		// 写入当前unit中的位
		data_[index_in_data] = (data_[index_in_data] & ~(element_capacity_ << index_in_unit)) | (data << index_in_unit);
		if (index_in_unit + element_bit_width_ >= kUnitBitWidth) {
			// 写入超出当前unit的位，并前进到下一个unit
			++index_in_data;
			index_in_unit = element_bit_width_ + index_in_unit - kUnitBitWidth;
			if (index_in_unit != 0) {
				data_[index_in_data] = (data_[index_in_data] & ~(element_capacity_ >> (element_bit_width_ - index_in_unit))) | (data >> (element_bit_width_ - index_in_unit));
			}
		} else {
			index_in_unit += element_bit_width_;
		}
	}
}

template<typename ValueType>
void PackedArray<ValueType>::setRange(const size_type begin, const size_type end, const std::vector<ValueType> &data) {
	if (end > size_) [[unlikely]] {
		throw std::out_of_range(std::format("PackedArray: range ({}, {}) to set out of range!", begin, end));
	}

	auto [index_in_data, index_in_unit] = indexOf(begin);

	for (int32_t i = 0; i < end - begin; ++i) {
		data_[index_in_data] = (data_[index_in_data] & ~(element_capacity_ << index_in_unit)) | (data[i] << index_in_unit);
		if (index_in_unit + element_bit_width_ >= kUnitBitWidth) {
			++index_in_data;
			index_in_unit = element_bit_width_ + index_in_unit - kUnitBitWidth;
			if (index_in_unit != 0) {
				data_[index_in_data] = (data_[index_in_data] & ~(element_capacity_ >> (element_bit_width_ - index_in_unit))) | (data[i] >> (element_bit_width_ - index_in_unit));
			}
		} else {
			index_in_unit += element_bit_width_;
		}
	}
}

template<typename ValueType>
std::vector<ValueType> PackedArray<ValueType>::getRange(const size_type begin, const size_type end) const {
	if (end > size_) [[unlikely]] {
		throw std::out_of_range(std::format("PackedArray: range ({}, {}) to get out of range!", begin, end));
	}

	if (element_bit_width_ == 0) {
		return std::vector<ValueType>(end - begin, 0);
	}

	std::vector<ValueType> result;
	result.reserve(end - begin);
	auto [index_in_data, index_in_unit] = indexOf(begin);

	for (int32_t i = begin; i < end; ++i) {
		if (index_in_unit + element_bit_width_ >= kUnitBitWidth) {
			ValueType buffer = (data_[index_in_data] >> index_in_unit);
			++index_in_data;
			buffer |= (data_[index_in_data] << (kUnitBitWidth - index_in_unit));
			index_in_unit = element_bit_width_ + index_in_unit - kUnitBitWidth;
			result.push_back(buffer & element_capacity_);

		} else {
			result.push_back((data_[index_in_data] >> index_in_unit) & element_capacity_);
			index_in_unit = index_in_unit + element_bit_width_;
		}
	}
	return result;
}

template<typename ValueType>
void PackedArray<ValueType>::transformTo(const uint8_t element_bit_width) {
	if (element_bit_width == element_bit_width_) {
		return;
	}

	auto temp = PackedArray(size_, element_bit_width);
	temp.setRange(0, size_, getRange(0, size_));
	*this = std::move(temp);
}

template<typename ValueType>
void PackedArray<ValueType>::grow() {
	transformTo(element_bit_width_ + 1);
}

template<typename ValueType>
void PackedArray<ValueType>::fit() {
	if (empty()) {
		element_bit_width_ = 0;
		return;
	}

	const ValueType maximum = *std::max_element(cbegin(), cend());
	if (maximum == 0) {
		element_bit_width_ = 0;
		data_.clear();
		return;
	}

	transformTo(std::bit_width(maximum));
}

template<typename ValueType>
void PackedArray<ValueType>::serialize(std::ostringstream &oss) const {
	SERIALIZE_WRITE(oss, size_);
	SERIALIZE_WRITE(oss, element_bit_width_);

	const uint32_t data_size = data_.size();
	SERIALIZE_WRITE(oss, data_size);
	oss.write(reinterpret_cast<const char *>(data_.data()), data_size * sizeof(ValueType));
}

template<typename ValueType>
void PackedArray<ValueType>::deserialize(std::istringstream &iss, const uint32_t size) {
	DESERIALIZE_READ(iss, size_);
	DESERIALIZE_READ(iss, element_bit_width_);
	element_capacity_ = (1ULL << element_bit_width_) - 1;

	uint32_t data_size;
	DESERIALIZE_READ(iss, data_size);
	data_.resize(data_size);
	iss.read(reinterpret_cast<char *>(data_.data()), data_size * sizeof(ValueType));
}

template<typename ValueType>
std::string PackedArray<ValueType>::toString() const {
	std::ostringstream oss;
	ValueType seed = data_.size();
	for (auto &i : data_) {
		seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	oss << "PackedArray{ "
		<< "size: " << size()
		<< ", data size: " << data_.size()
		<< ", element bit width: " << int32_t(element_bit_width_)
		<< ", element capacity: " << element_capacity_
		<< ", data hash: " << seed
		<< " }";
	return oss.str();
}

} //namespace pgvoxel
