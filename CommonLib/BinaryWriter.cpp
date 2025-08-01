#include "pch.h"
#include "BinaryWriter.h"
#include <iostream>
#include <assert.h>



BinaryWriter* BinaryWriter_new(uint8_t** buff, size_t buff_size, size_t buff_at, BinaryWriterByteOrder bo) {
	BinaryWriter* bw = (BinaryWriter*)calloc(1, sizeof(BinaryWriter));
	bw->buff = buff;
	bw->size = buff_at;
	bw->cap = buff_size;
	bw->bo = bo;
	return bw;
}

void BinaryWriter_free(BinaryWriter* bw) {
	free(bw);
}

size_t BinaryWriter_available(BinaryWriter* bw) {
	return bw->cap - bw->size;
}

#define BW_CHECK(bw, v) if((bw)->size+sizeof(v) > (bw)->cap) return false

bool BinaryWriter_write_8(BinaryWriter* bw, uint8_t v) {
	BW_CHECK(bw, v);
	(*(bw->buff))[bw->size++] = v;
	return true;
}

bool BinaryWriter_write_16(BinaryWriter* bw, uint16_t v) {
	BW_CHECK(bw, v);
	if (bw->bo == BWBO_BIG) {
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 8);
		(*(bw->buff))[bw->size++] = (uint8_t)v;
	}
	else if (bw->bo == BWBO_LITTLE) {
		(*(bw->buff))[bw->size++] = (uint8_t)v;
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 8);
	} else {
		assert(false);
		return false;
	}
	return true;
}

bool BinaryWriter_write_32(BinaryWriter* bw, uint32_t v) {
	BW_CHECK(bw, v);
	if (bw->bo == BWBO_BIG) {
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 24);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 16);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 8);
		(*(bw->buff))[bw->size++] = (uint8_t)v;
	}
	else if (bw->bo == BWBO_LITTLE) {
		(*(bw->buff))[bw->size++] = (uint8_t)v;
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 8);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 16);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 24);
	}
	else {
		assert(false);
		return false;
	}

	return true;
}

bool BinaryWriter_write_64(BinaryWriter* bw, uint64_t v) {
	BW_CHECK(bw, v);
	if (bw->bo == BWBO_BIG) {
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 56);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 48);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 40);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 32);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 24);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 16);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 8);
		(*(bw->buff))[bw->size++] = (uint8_t)v;
	}
	else if (bw->bo == BWBO_LITTLE) {
		(*(bw->buff))[bw->size++] = (uint8_t)v;
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 8);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 16);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 24);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 32);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 40);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 48);
		(*(bw->buff))[bw->size++] = (uint8_t)(v >> 56);
	}
	else {
		assert(false);
		return false;
	}

	return true;
}


bool BinaryWriter_write_buff(BinaryWriter* bw, const uint8_t* buff, size_t size) {
	if (bw->size + size > bw->cap) {
		return false;
	}

	memcpy_s(((*(bw->buff)) + bw->size), bw->cap, buff, size);
	bw->size += size;

	return true;
}

