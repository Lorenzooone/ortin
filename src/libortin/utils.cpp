#include "utils.hpp"

void _ort_write_le32(uint8_t* data, uint32_t value) {
	data[0] = value & 0xFF;
	data[1] = (value >> 8) & 0xFF;
	data[2] = (value >> 16) & 0xFF;
	data[3] = (value >> 24) & 0xFF;
}

uint32_t _ort_read_le32(uint8_t* data) {
	uint32_t out = 0;
	out |= ((uint32_t)data[0]) << 0;
	out |= ((uint32_t)data[1]) << 8;
	out |= ((uint32_t)data[2]) << 16;
	out |= ((uint32_t)data[3]) << 24;
	return out;
}

void _ort_write_le64(uint8_t* data, uint64_t value) {
	data[0] = value & 0xFF;
	data[1] = (value >> 8) & 0xFF;
	data[2] = (value >> 16) & 0xFF;
	data[3] = (value >> 24) & 0xFF;
	data[4] = (value >> 32) & 0xFF;
	data[5] = (value >> 40) & 0xFF;
	data[6] = (value >> 48) & 0xFF;
	data[7] = (value >> 56) & 0xFF;
}

uint64_t _ort_read_le64(uint8_t* data) {
	uint64_t out = 0;
	out |= ((uint64_t)data[0]) << 0;
	out |= ((uint64_t)data[1]) << 8;
	out |= ((uint64_t)data[2]) << 16;
	out |= ((uint64_t)data[3]) << 24;
	out |= ((uint64_t)data[4]) << 32;
	out |= ((uint64_t)data[5]) << 40;
	out |= ((uint64_t)data[6]) << 48;
	out |= ((uint64_t)data[7]) << 56;
	return out;
}

void _ort_write_le16(uint8_t* data, uint16_t value) {
	data[0] = value & 0xFF;
	data[1] = (value >> 8) & 0xFF;
}

uint16_t _ort_read_le16(uint8_t* data) {
	uint16_t out = 0;
	out |= ((uint16_t)data[0]) << 0;
	out |= ((uint16_t)data[1]) << 8;
	return out;
}
