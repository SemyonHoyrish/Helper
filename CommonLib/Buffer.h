#pragma once
#include "CommonLib.h"
#include <iostream>
#include <assert.h>


typedef struct {
	char* data;
	size_t size;
	size_t cap;
} BinaryBuffer;


extern "C" COMMONLIB_API BinaryBuffer* BinaryBuffer_init(size_t initCap = 0);

extern "C" COMMONLIB_API void BinaryBuffer_free(BinaryBuffer* buff);

extern "C" COMMONLIB_API void BinaryBuffer_push(BinaryBuffer* buff, const char* data, size_t size);
extern "C" COMMONLIB_API void BinaryBuffer_push_one(BinaryBuffer* buff, char data, size_t size);

extern "C" COMMONLIB_API void BinaryBuffer_realloc(BinaryBuffer* buff, size_t newCap);
extern "C" COMMONLIB_API size_t BinaryBuffer_shrink(BinaryBuffer* buff);

