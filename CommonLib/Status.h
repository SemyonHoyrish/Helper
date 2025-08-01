#pragma once
#include <stdint.h>
#include <iostream>

// TODO: fix naming with push_macro, pop_macro.
enum struct StatusCode : uint16_t {
	SC_OK,
	SC_ERROR,
	SC_ERROR_ALREADY_EXISTS,
	SC_ERROR_NO_PERMISSION,
	SC_ERROR_UNAVAILABLE,
	SC_ERROR_INVALID,
	SC_ERROR_NOT_FOUND,
	SC_ERROR_INSUFFICIENT_DATA,
};

struct Status
{
public:
	Status(StatusCode code, std::string msg) : code(code), msg(msg) {}
	Status(StatusCode code) : Status(code, {}) {}
	Status(const Status& other) : Status(other.code, other.msg) {}

	constexpr bool isOk() { return code == StatusCode::SC_OK; }
	constexpr bool isError() { return !isOk(); }

	static Status OK() {
		return StatusCode::SC_OK;
	}

	constexpr int intCode() { return static_cast<int>(code); }

	//Status& operator= (const Status& o) {
	//	*(const_cast<StatusCode*>(&code)) = o.code;
	//	*(const_cast<std::string*>(&msg)) = o.msg;

	//	return *this;
	//}

	Status& operator= (Status&& o) {
		*(const_cast<StatusCode*>(&code)) = std::move(o.code);
		*(const_cast<std::string*>(&msg)) = std::move(o.msg);

		return *this;
	}

public:
	const StatusCode code;
	const std::string msg;

};

