#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <typeindex>
#include <fstream>
#include "CommonLib.h"
#include "Status.h"
#include "Value.h"
#include "BinaryWriter.h"

/*
Storage (custom binary format, json dump, with encoding)
	File (StorageEntry) : Preferences, SavelinkDB, NotesData
	HeaderBegin
		namesize name
		descrsize description
		version
	HeaderEnd
	EntryBegin type(kv, record) namesize name
		kv: valuetype size data
		record: [for each field] namesize name valuetype (size data | custom_type Entry::Record)
	EntryEnd

	IndexTable entry-name -> data position;
*/

// TODO: add support for double/float
// TODO: extend support for types: long, longlong, etc.

#define STORAGE_VERSION ((uint32_t)1)
#define STORAGE_MIN_SUPPORTED_VERSION ((uint32_t)1)
//class COMMONLIB_API Storage
class Storage
{
public:
	enum class EntryType : uint16_t {
		KV = 1,
		Record
	};

	enum class CustomType : uint16_t {
		RecordBuffer = 1,
		RecordView,
		Vector,
	};

	class RecordBuffer {
	public:
		//RecordBuffer() = default;
		RecordBuffer(const Storage* s) : s(s) {}

		Status writeUIntField(std::string name, unsigned int val)
			{ return writeField(name, Value(val)); }
		Status writeIntField(std::string name, int val)
			{ return writeField(name, Value(val)); }
		Status writeCharField(std::string name, char val)
			{ return writeField(name, Value(val)); }
		Status writeStringField(std::string name, std::string val)
			{ return writeField(name, Value(val)); }
		Status writeStringField(std::string name, const char* val)
			{ return writeField(name, Value(val)); }

		template<class T> 
		Status writeRecordField(std::string name, const T* val)
		{
			if (s->serializers.find(typeid(T)) == s->serializers.end()) {
				return Status(StatusCode::SC_ERROR_NOT_FOUND, std::string("No type serializer registered for type ") + typeid(T).name());
			}
			
			auto b = new RecordBuffer(s);
			fields.push_back({ name, {(void*)b, static_cast<uint32_t>(CustomType::RecordBuffer)}});

			//(s->serializers)[typeid(T)](val, b);
			return (s->serializers).at(typeid(T))(val, b);
		}

		const std::vector<NamedValue>& collect() const
			{ return fields; }

	private:
		Status writeField(std::string name, Value value)
		{
			fields.push_back({ name, value });
			return Status(StatusCode::SC_OK);
		}

		//	// TODO: ?do i need to do smth?
		//	~Field() {};
		//};

		std::vector<NamedValue> fields;

		const Storage* s;
	};

	class RecordView {
	public:
		RecordView(const Storage *s, const std::vector<NamedValue>& view)
			: s(s), view(view) {
		}

		Status readUIntField(std::string name, unsigned int* out) const
		{
			for (const NamedValue& entry : view) {
				if (entry.first == name) {
					*out = entry.second.getUInt();
					return Status::OK();
				}
			}

			out = NULL;
			return Status(StatusCode::SC_ERROR_NOT_FOUND);
		}
		Status readIntField(std::string name, int* out) const
		{
			for (const NamedValue& entry : view) {
				if (entry.first == name) {
					*out = entry.second.getInt();
					return Status::OK();
				}
			}

			out = NULL;
			return Status(StatusCode::SC_ERROR_NOT_FOUND);
		}
		Status readCharField(std::string name, char* out) const
		{
			for (const NamedValue& entry : view) {
				if (entry.first == name) {
					*out = entry.second.getChar();
					return Status::OK();
				}
			}

			out = NULL;
			return Status(StatusCode::SC_ERROR_NOT_FOUND);
		}
		Status readStringField(std::string name, std::string* out) const
		{
			for (const NamedValue& entry : view) {
				if (entry.first == name) {
					*out = entry.second.getString();
					return Status::OK();
				}
			}

			out = NULL;
			return Status(StatusCode::SC_ERROR_NOT_FOUND);
		}
		Status readCStringField(std::string name, const char** out) const
		{
			for (const NamedValue& entry : view) {
				if (entry.first == name) {
					*out = entry.second.getCString();
					return Status::OK();
				}
			}

			out = NULL;
			return Status(StatusCode::SC_ERROR_NOT_FOUND);
		}

		template<class T>
		Status readRecordField(std::string name, T** out) const
		{
			if (s->deserializers.find(typeid(T)) == s->deserializers.end()) {
				return Status(StatusCode::SC_ERROR_NOT_FOUND, std::string("No type deserializer registered for type ") + typeid(T).name());
			}

			for (const NamedValue& entry : view) {
				if (entry.first == name) {
					auto v = RecordView(s, extract(entry.second));
					//return s->deserializers[typeid(T)](out, &v);
					return s->deserializers.at(typeid(T))((void**)out, &v);
				}
			}

			out = NULL;
			return Status(StatusCode::SC_ERROR_NOT_FOUND);
		}

		const std::vector<NamedValue>& origin() const
			{ return view; }

	private:


	private:
		const std::vector<NamedValue>& view;
		const Storage* s;

	};

public:
	COMMONLIB_API Storage(std::string name, bool readonly = false)
		: Storage(name, "", readonly) {
	}
	COMMONLIB_API Storage(std::string name, std::string path, bool readonly = false)
		: name(name), filename(path + name + ".sdat"), readonly(readonly), serializers(), deserializers() {
	}

	COMMONLIB_API Status newFile();
	COMMONLIB_API Status load();
	COMMONLIB_API Status load(bool newIfMissing);
	COMMONLIB_API Status save();

	const std::string& getName() const
		{ return name; }
	const std::string& getDescription() const
		{ return description; }

	Status setDescription(std::string d);

	Status writeKVUInt(std::string name, unsigned int value)
		{ return writeKV(name, Value(value)); }
	Status writeKVInt(std::string name, int value)
		{ return writeKV(name, Value(value)); }
	Status writeKVChar(std::string name, char value)
		{ return writeKV(name, Value(value)); }
	Status writeKVString(std::string name, std::string value)
		{ return writeKV(name, Value(value)); }
	Status writeKVString(std::string name, const char* value)
		{ return writeKV(name, Value(value)); }
	

	Status readKVUInt(std::string name, unsigned int* out)
	{
		auto v = Value(0u);
		auto s = readKV(name, &v);
		*out = v.getUInt();
		return s;
	}
	Status readKVInt(std::string name, int* out)
	{
		auto v = Value(0);
		auto s = readKV(name, &v);
		*out = v.getInt();
		return s;
	}
	Status readKVChar(std::string name, char* out)
	{
		auto v = Value('\0');
		auto s = readKV(name, &v);
		*out = v.getChar();
		return s;
	}
	Status readKVString(std::string name, std::string* out)
	{
		auto v = Value("0");
		auto s = readKV(name, &v);
		*out = v.getString();
		return s;
	}
	Status readKVString(std::string name, const char** out)
	{
		auto v = Value("0");
		auto s = readKV(name, &v);
		*out = v.getCString();
		return s;
	}

	Status deleteKVUInt(std::string name)
	{
		return deleteKV(name, Value::Type::UINT);
	}
	Status deleteKVInt(std::string name)
	{
		return deleteKV(name, Value::Type::INT);
	}
	Status deleteKVChar(std::string name)
	{
		return deleteKV(name, Value::Type::CHAR);
	}
	Status deleteKVString(std::string name)
	{
		return deleteKV(name, Value::Type::STRING);
	}

	template<class T>
	StatusCode registerTypeSerializer(std::function<Status(const T*, RecordBuffer*)> f);
	template<class T>
	StatusCode registerTypeDeserializer(std::function<Status(T**, const RecordView*)> f);


	template<class T>
	Status writeRecord(std::string name, T* value);

	template<class T>
	Status readRecord(std::string name, T** out);

	template<class T>
	Status deleteRecord(std::string name);
	
	static const std::vector<NamedValue>& extract(const Value& v) {
		if (v.getCustomType() == static_cast<uint32_t>(CustomType::RecordBuffer)) {
			return reinterpret_cast<const RecordBuffer*>(v.getRaw())->collect();
		}
		else if (v.getCustomType() == static_cast<uint32_t>(CustomType::RecordView)) {
			return reinterpret_cast<const RecordView*>(v.getRaw())->origin();
		}
		else if (v.getCustomType() == static_cast<uint32_t>(CustomType::Vector)) {
			return *reinterpret_cast<const std::vector<NamedValue>*>(v.getRaw());
		}

		assert(false);
		return *new std::vector<NamedValue>();
	}

	bool isLoaded() const { return loaded; }
	bool isSaved() const { return saved; }

private:
	enum struct SpecialBytes : uint32_t {
		HeaderBegin = 0xFA0BFABE,
		HeaderEnd   = 0xFA0BFAED,
		EntryBegin  = 0xFA0BE0BE,
		EntryEnd    = 0xFA0BE0ED,
	};

	void resetSaved() { saved = false; }

	COMMONLIB_API inline Status checkForWritable();
	COMMONLIB_API inline Status checkForReadable();

	COMMONLIB_API Status writeKV(std::string name, Value value);

	COMMONLIB_API Status readKV(std::string name, Value* out);

	COMMONLIB_API Status deleteKV(std::string name, Value::Type t);


	void writeRecordField(BinaryWriter*, size_t&, const NamedValue&);
	Status readRecordField(struct Reader* r, std::vector<NamedValue>& out);

private:
	const std::string filename;
	const bool readonly;
	//bool loaded = false;
	bool loaded = true; // TODO: remove, DBG!
	bool saved = false; // after last edit

	std::string name;
	std::string description;

	//std::vector<EntryInfo> entries;
	std::map<std::string, Value> kv;
	std::map<std::string, std::vector<NamedValue>> records;

	std::map<std::type_index, std::function<Status(const void*, RecordBuffer*)>> serializers;
	std::map<std::type_index, std::function<Status(void**, const RecordView*)>> deserializers;
};


template<class T>
StatusCode Storage::registerTypeSerializer(std::function<Status(const T*, RecordBuffer*)> f) {
	const type_info& ti = typeid(T);

	if (serializers.find(ti) != serializers.end()) {
		return StatusCode::SC_ERROR_ALREADY_EXISTS;
	}

	serializers.insert({ ti, std::move([f](const void* val, RecordBuffer* b) -> Status { return f((const T*)val, b); }) });
	//serializers.insert({ ti, ([f](const void* val, RecordBuffer* b) -> Status { return f((const T*)val, b); }) });
	//serializers.insert({ ti, (std::function<Status(const void*, RecordBuffer*)>)f});

	return StatusCode::SC_OK;
}
template<class T>
StatusCode Storage::registerTypeDeserializer(std::function<Status(T**, const RecordView*)> f) {
	const type_info& ti = typeid(T);

	if (deserializers.find(ti) != deserializers.end()) {
		return StatusCode::SC_ERROR_ALREADY_EXISTS;
	}

	deserializers.insert({ ti, std::move([f](void** val, const RecordView* b) -> Status { return f((T**)val, b); }) });

	return StatusCode::SC_OK;
}

template<class T>
Status Storage::writeRecord(std::string name, T* value) {
	auto c = checkForWritable();
	if (c.isError()) return c;

	resetSaved();

	const type_info& ti = typeid(T);

	if (serializers.find(ti) == serializers.end()) {
		return Status(StatusCode::SC_ERROR_NOT_FOUND, std::string("No type serializers registered for type ") + ti.name());
	}

	if (records.find(name) != records.end()) {
		return StatusCode::SC_ERROR_ALREADY_EXISTS;
	}

	auto rb = new RecordBuffer(this);
	Status convst = serializers[ti](value, rb);
	if (convst.isOk())
		records[name] = rb->collect();

	return convst;
}

template<class T>
Status Storage::readRecord(std::string name, T** value) {
	if (!isLoaded()) {
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "Storage has not been loaded yet");
	}

	const type_info& ti = typeid(T);

	if (deserializers.find(ti) == deserializers.end()) {
		return Status(StatusCode::SC_ERROR_NOT_FOUND, std::string("No type deserializers registered for type ") + ti.name());
	}

	if (records.find(name) == records.end()) {
		return Status(StatusCode::SC_ERROR_NOT_FOUND);
	}

	auto e = records[name];
	auto rv = RecordView(this, e);

	return deserializers[ti]((void**)value, &rv);
}

template<class T>
Status Storage::deleteRecord(std::string name) {
	if (!isLoaded()) {
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "Storage has not been loaded yet");
	}

	const type_info& ti = typeid(T);

	if (deserializers.find(ti) == deserializers.end()) {
		return Status(StatusCode::SC_ERROR_NOT_FOUND, std::string("No type deserializers registered for type ") + ti.name());
	}

	if (records.find(name) == records.end()) {
		return Status(StatusCode::SC_ERROR_NOT_FOUND);
	}

	auto e = records[name];
	auto rv = RecordView(this, e);
	T* _val = nullptr;
	T** value = &_val;
	auto status = deserializers[ti]((void**)value, &rv);

	if (status.isError()) {
		return StatusCode::SC_ERROR_INVALID;
	}

	records.erase(name);
	resetSaved();

	return StatusCode::SC_OK;
}
