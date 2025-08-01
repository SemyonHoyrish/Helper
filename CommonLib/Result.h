#pragma once
#include <assert.h>
#include "Status.h"

template <typename T>
class Result
{
public:
	Result(Status s) : s(s), val(nullptr) {
		assert(s.isError());
	}
	Result(Status s, const T* value) : s(s), val(value) {}

	Status getStatus() const
	{
		return s;
	}

	const T* getValue() const
	{
		return val;
	}

private:
	const Status s;
	const T* val;

};

