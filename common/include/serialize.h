#pragma once
#include <sstream>

namespace pgvoxel {
template <typename T>
requires requires(std::ostringstream &oss, T &p) { p.serialize(oss); }
std::ostringstream &operator<<(std::ostringstream &oss, const T &p) {
	size_t size{ 0 };
	oss.write(reinterpret_cast<char *>(&size), sizeof(size));
	const auto start_pos = oss.tellp();
	p.serialize(oss);
	const auto end_pos = oss.tellp();
	size = static_cast<size_t>(end_pos - start_pos);
	oss.seekp(start_pos - std::streamoff(sizeof(size)));
	oss.write(reinterpret_cast<char *>(&size), sizeof(size));
	oss.seekp(end_pos);

	return oss;
}

template <typename T>
requires requires(std::istringstream &iss, T &p, size_t size) { p.deserialize(iss, size); }
std::istringstream &operator>>(std::istringstream &iss, T &p) {
	size_t size;
	iss.read(reinterpret_cast<char *>(&size), sizeof(size_t));
	p.deserialize(iss, size);

	return iss;
}

#define SERIALIZE_WRITE(oss, data) oss.write(reinterpret_cast<const char *>(&data), sizeof(data))
#define DESERIALIZE_READ(iss, data) iss.read(reinterpret_cast<char *>(&data), sizeof(data))

} // namespace pgvoxel
