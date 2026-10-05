#ifndef __ORTIN_UTILS_HPP__
#define __ORTIN_UTILS_HPP__

void _ort_write_le64(uint8_t* dst, uint64_t data);
void _ort_write_le32(uint8_t* dst, uint32_t data);
void _ort_write_le16(uint8_t* dst, uint16_t data);

uint64_t _ort_read_le64(uint8_t* dst);
uint32_t _ort_read_le32(uint8_t* dst);
uint16_t _ort_read_le16(uint8_t* dst);

// Do this to avoid conflicts with other libraries...
#define write_le64 _ort_write_le64
#define write_le32 _ort_write_le32
#define write_le16 _ort_write_le16
#define read_le64 _ort_read_le64
#define read_le32 _ort_read_le32
#define read_le16 _ort_read_le16


#endif
