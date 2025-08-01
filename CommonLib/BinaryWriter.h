#pragma once
#include "CommonLib.h"
#include <stdint.h>


typedef enum {
	BWBO_LITTLE = 0x3412,
	BWBO_BIG = 0x1234
} BinaryWriterByteOrder;

typedef struct {
	uint8_t** buff;
	size_t cap;
	size_t size;
	BinaryWriterByteOrder bo;
} BinaryWriter;


extern "C" COMMONLIB_API BinaryWriter* BinaryWriter_new(uint8_t** buff,
														size_t buff_size,
														size_t buff_at = 0,
														BinaryWriterByteOrder bo = BWBO_BIG);

extern "C" COMMONLIB_API void BinaryWriter_free(BinaryWriter* bw);

extern "C" COMMONLIB_API inline size_t BinaryWriter_available(BinaryWriter* bw);

extern "C" COMMONLIB_API bool BinaryWriter_write_8(BinaryWriter* bw, uint8_t v);

extern "C" COMMONLIB_API bool BinaryWriter_write_16(BinaryWriter* bw, uint16_t v);

extern "C" COMMONLIB_API bool BinaryWriter_write_32(BinaryWriter* bw, uint32_t v);

extern "C" COMMONLIB_API bool BinaryWriter_write_64(BinaryWriter* bw, uint64_t v);

extern "C" COMMONLIB_API bool BinaryWriter_write_buff(BinaryWriter* bw, const uint8_t* buff, size_t size);

