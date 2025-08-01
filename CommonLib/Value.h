#pragma once
#include <stdint.h>
#include <string>
#include <assert.h>

class Array;

// TODO: ?convert to const void * ??
class Value {
public:
	enum class Type : uint32_t {
		UNDEFINED = UINT32_MAX,
		UINT = 1,
		INT,
		CHAR,
		STRING,
		DOUBLE,
		ARRAY, // TODO
		CUSTOM = UINT32_MAX - 1,
	};
	static constexpr size_t SizeOf(Type t)
	{
		switch (t) {
		case Type::UINT:
		case Type::INT:
			return sizeof(int);
		case Type::CHAR:
			return sizeof(char);
		case Type::DOUBLE:
			return sizeof(double);
		default:
			return 0;
		}
	}

	Value() : type(Type::UNDEFINED), data(NULL) {}

	Value(const Value& other) : type(other.type), data(other.data), custom_type_id(other.custom_type_id) {

	}

	Value(unsigned int v) : type(Type::UINT) {
		data = new unsigned int{ v };
	}
	Value(int v) : type(Type::INT) {
		data = new int{ v };
	}
	Value(char v) : type(Type::CHAR) {
		data = new char{ v };
	}
	Value(std::string str) : type(Type::STRING) {
		data = new char[str.length() + 1];
		strcpy_s((char*)data, str.length() + 1, str.c_str());
	}
	Value(const char* str) : type(Type::STRING) {
		data = new char[strlen(str) + 1];
		strcpy_s((char*)data, strlen(str) + 1, str);
	}
	Value(double v) : type(Type::DOUBLE) {
		data = new double{ v };
	}

	Value(void* data, uint32_t custom_type_id) : type(Type::CUSTOM), custom_type_id(custom_type_id) {
		this->data = data;
	}

	Type getType() const
	{
		return type;
	}
	uint32_t getCustomType() const
	{
		return custom_type_id;
	}

	unsigned int getUInt() const
	{
		assert(type == Type::UINT);
		return *((unsigned int*)data);
	}
	int getInt() const
	{
		assert(type == Type::INT);
		return *((int*)data);
	}
	char getChar() const
	{
		assert(type == Type::CHAR);
		return *((char*)data);
	}
	std::string getString() const
	{
		assert(type == Type::STRING);
		const char* cstr = ((const char*)data);
		return std::string(cstr);
	}
	const char* getCString() const
	{
		assert(type == Type::STRING);
		return ((const char*)data);
	}
	const Array* getArray() const;

	double getDouble() const
	{
		assert(type == Type::DOUBLE);
		return *((double*)data);
	}

	bool isUndefined() const {
		return type == Type::UNDEFINED;
	}

	//double toDouble() const
	//{
	//	//TOOD: ?????

	//	switch (type)
	//	{
	//	case Type::UINT:
	//		return (double)getUInt();
	//	case Type::INT:
	//		return (double)getInt();
	//	case Type::DOUBLE:
	//		return getDouble();

	//	default:
	//		return 0;
	//	}

	//}

	const void* getRaw() const
	{
		assert(type == Type::CUSTOM);
		return data;
	}

	// TODO: !
	//~Value() {
	//	if (type == Type::STRING) {
	//		delete[] data;
	//	}
	//	else {
	//		delete data;
	//	}
	//}

private:
	void* data;
	uint32_t custom_type_id = 0; // for CUSTOM
	Type type;

};

typedef std::pair<std::string, Value> NamedValue;

class Array {
public:

private:
	Value* data;
	Value::Type type;
	size_t size;
};
