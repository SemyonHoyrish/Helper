#include "pch.h"
#include "Buffer.h"


BinaryBuffer* BinaryBuffer_init(size_t initCap) {
	return new BinaryBuffer{
		(char*) calloc(initCap, sizeof(char)),
		0,
		initCap,
	};
}

void BinaryBuffer_free(BinaryBuffer* buff) {
	delete[] buff->data;
	delete buff;
}

void BinaryBuffer_push(BinaryBuffer* buff, const char* data, size_t size) {
	if (buff->size + size > buff->cap) {
		buff->cap = (buff->cap + size) * 2;
		auto reallocation = (char*)realloc(buff->data, buff->cap);
		if (reallocation == NULL) {
			assert("Not enough memory" && false);
			delete[] buff->data;
			return;
		}
		buff->data = reallocation;
	}

	memcpy_s(buff->data + buff->size, buff->cap, data, size);
	buff->size += size;
}


void BinaryBuffer_push_one(BinaryBuffer* buff, char data, size_t size) {
	assert("Not implemented" && false);
}
