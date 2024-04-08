#include "include/packed_array.h"
#include "../common/include/serialize.h"
#include <algorithm>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace pgvoxel {

void PackedArray::resize(const int32_t size) {
	size_ = size;
	data_.resize((size_ * element_bit_width_ + kUnitBitWidth - 1) / kUnitBitWidth);
}

void PackedArray::push_back(const value_type value) {
	resize(size_ + 1);
	set(size_ - 1, value);
}

PackedArray::value_type PackedArray::get(const int32_t index) const {
	if (index >= size_) {
		throw std::out_of_range(std::format("PackedArray: index {} out of range!", index));
	}

	if (element_bit_width_ == 0) {
		return 0;
	}

	const auto [index_in_data, index_in_unit] = indexOf(index);

	// 读取当前unit中的位
	value_type result{ static_cast<value_type>((data_[index_in_data] >> index_in_unit) & element_capacity_) };
	if (index_in_unit + element_bit_width_ > kUnitBitWidth) {
		// 读取超出当前unit的位
		result |= ((data_[index_in_data + 1] << index_in_unit) & element_capacity_);
	}

	return result;
}

void PackedArray::set(const int32_t index, const value_type value) {
	if (index >= size_) {
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

void PackedArray::setRange(const int32_t begin, const int32_t end, const value_type data) {
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

void PackedArray::setRange(const int32_t begin, const int32_t end, const std::vector<value_type> &data) {
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

std::vector<PackedArray::value_type> PackedArray::getRange(const int32_t begin, const int32_t end) const {
	if (end > size_) [[unlikely]] {
		throw std::out_of_range(std::format("PackedArray: range ({}, {}) to get out of range!", begin, end));
	}

	if (element_bit_width_ == 0) {
		return std::vector<value_type>(end - begin, 0);
	}

	std::vector<value_type> result;
	result.reserve(end - begin);
	auto [index_in_data, index_in_unit] = indexOf(begin);

	for (int32_t i = begin; i < end; ++i) {
		if (index_in_unit + element_bit_width_ >= kUnitBitWidth) {
			value_type buffer = (data_[index_in_data] >> index_in_unit);
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

void PackedArray::transform(const int8_t element_bit_width) {
	if (element_bit_width == element_bit_width_) {
		return;
	}

	auto temp = PackedArray(size_, element_bit_width);
	temp.setRange(0, size_, getRange(0, size_));
	*this = std::move(temp);
}

void PackedArray::grow() {
	transform(element_bit_width_ + 1);
}

void PackedArray::fit() {
	if (empty()) {
		element_bit_width_ = 0;
		return;
	}

	const value_type maximum = *std::max_element(cbegin(), cend());
	transform(std::bit_width(maximum));
}

void PackedArray::serialize(std::ostringstream &oss) const {
	const uint32_t data_size = data_.size();
	SERIALIZE_WRITE(oss, size_);
	SERIALIZE_WRITE(oss, element_bit_width_);
	SERIALIZE_WRITE(oss, data_size);
	oss.write(reinterpret_cast<const char *>(data_.data()), data_size * sizeof(value_type));
}

void PackedArray::deserialize(std::istringstream &iss, const int32_t size) {
	DESERIALIZE_READ(iss, size_);
	DESERIALIZE_READ(iss, element_bit_width_);
	element_capacity_ = (1ULL << element_bit_width_) - 1;
	uint32_t data_size;
	DESERIALIZE_READ(iss, data_size);
	data_.resize(data_size);
	iss.read(reinterpret_cast<char *>(data_.data()), data_size * sizeof(value_type));
}

std::string PackedArray::toString() const {
	std::ostringstream oss;
	PackedArray::value_type seed = data_.size();
	for (auto &i : data_) {
		seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	oss << "PackedArray {\n"
		<< "\tsize: " << size() << "\n"
		<< "\tdata size: " << data_.size() << "\n"
		<< "\telement bit width: " << int32_t(element_bit_width_) << "\n"
		<< "\telement capacity: " << element_capacity_ << "\n"
		<< "\tdata hash: " << seed << "\n"
		<< "}";
	return oss.str();
}

} //namespace pgvoxel
