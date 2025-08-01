#include "pch.h"
#include "Storage.h"


Status Storage::setDescription(std::string d) {
	auto c = checkForWritable();
	if (c.isError()) return c;

	description = d;
	return Status::OK();
}


Status Storage::writeKV(std::string name, Value value) {
	auto c = checkForWritable();
	if (c.isError()) return c;

	assert(value.getType() != Value::Type::CUSTOM);
	assert(value.getType() != Value::Type::UNDEFINED);

	resetSaved();
	
	if (kv.find(name) != kv.end()) {
		return StatusCode::SC_ERROR_ALREADY_EXISTS;
	}

	kv.insert({name, value});
	return Status(StatusCode::SC_OK);
}

Status Storage::readKV(std::string name, Value* out) {
	auto c = checkForReadable();
	if (c.isError()) return c;

	assert(out->getType() != Value::Type::CUSTOM);
	assert(out->getType() != Value::Type::UNDEFINED);

	if (kv.find(name) == kv.end()) {
		return StatusCode::SC_ERROR_NOT_FOUND;
	}

	auto v = kv.at(name);
	if (v.getType() != out->getType()) {
		return Status(StatusCode::SC_ERROR_INVALID, std::string("type of requested key '") + name + "' is different");
	}

	*out = Value(v);

	return Status(StatusCode::SC_OK);
}


Status Storage::checkForWritable() {
	if (readonly) {
		return Status(StatusCode::SC_ERROR_NO_PERMISSION, "Storage is in readonly mode");
	}
	if (!isLoaded()) {
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "Storage has not been loaded yet");
	}
	return Status::OK();
}

Status Storage::checkForReadable() {
	if (!isLoaded()) {
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "Storage has not been loaded yet");
	}
	return Status::OK();
}


BinaryWriter* bwrealloc(BinaryWriter* from, size_t size) {
	if (!from) {
		uint8_t* buff = new uint8_t[size];
		uint8_t** buffptr = new uint8_t* { buff };
		return BinaryWriter_new(buffptr, size);
	}

	uint8_t** buffptr = from->buff;
	size_t sz = from->size;
	assert(size > sz);
	BinaryWriter_free(from);

	*buffptr = (uint8_t*)realloc(*buffptr, size);
	assert(*buffptr != NULL);

	return BinaryWriter_new(buffptr, size, sz);
}

inline void writeSize(BinaryWriter* w, size_t sz) {
	if (sizeof(sz) == sizeof(uint64_t)) {
		BinaryWriter_write_64(w, sz);
	}
	else if (sizeof(sz) == sizeof(uint32_t)) {
		BinaryWriter_write_32(w, static_cast<uint32_t>(sz));
	}
}


constexpr size_t BASE_SIZE = 1024 * 1024;
constexpr size_t START_SIZE = BASE_SIZE * 10;
constexpr size_t SIZE_MULT = 2;
#define ENSURE_BASE_SIZE(size, bw) if (BinaryWriter_available((bw)) < BASE_SIZE) { (size)+=BASE_SIZE; (bw) = bwrealloc((bw), (size)); }
#define EXPAND_WRITER(size, bw, expr) while (!(expr)) { (size)*=SIZE_MULT; (bw) = bwrealloc((bw), (size)); }

void Storage::writeRecordField(BinaryWriter* w, size_t& size, const NamedValue& field) {
	writeSize(w, field.first.length());
	EXPAND_WRITER(size, w, BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(field.first.c_str()), field.first.length()));
	DEBUG_LOG_INFO("written field.first (name)");
	ENSURE_BASE_SIZE(size, w);

	// valuetype
	BinaryWriter_write_32(w, static_cast<uint32_t>(field.second.getType()));
	DEBUG_LOG_INFO("written value type");
	if (field.second.getType() == Value::Type::CUSTOM) {
		DEBUG_LOG_INFO("value type is CUSTOM");
		//BinaryWriter_write_32(w, field.second.getCustomType());
		// We later use Storage::extract to convert any record field representation to a vector
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::CustomType::Vector));
		DEBUG_LOG_INFO("written custom type");
		
		ENSURE_BASE_SIZE(size, w);
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::EntryBegin));
		DEBUG_LOG_INFO("written entry begin");
		BinaryWriter_write_16(w, static_cast<uint16_t>(Storage::EntryType::Record));
		DEBUG_LOG_INFO("written entrytype RECORD");
		writeSize(w, field.first.length());
		EXPAND_WRITER(size, w, BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(field.first.c_str()), field.first.length()));
		DEBUG_LOG_INFO("written field.first (name)");


		ENSURE_BASE_SIZE(size, w);

		DEBUG_LOG_INFO("iteration over field fields begin");
		for (const NamedValue& f : Storage::extract(field.second)) {
			writeRecordField(w, size, f);
		}
		DEBUG_LOG_INFO("iteration over field fields end");


		ENSURE_BASE_SIZE(size, w);
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::EntryEnd));
		DEBUG_LOG_INFO("written entry end");

		return;
	}

	DEBUG_LOG_INFO("value type is SIMPLE TYPE");

	// size
	size_t sz;
	if (field.second.getType() == Value::Type::STRING) {
		sz = strlen(field.second.getCString());
	}
	else {
		sz = Value::SizeOf(field.second.getType());
	}
	writeSize(w, sz);

	// data
	switch (field.second.getType()) {
	case Value::Type::STRING:
		EXPAND_WRITER(size, w, BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(field.second.getCString()), sz));
		break;
	case Value::Type::UINT:
		BinaryWriter_write_32(w, field.second.getUInt());
		break;
	case Value::Type::INT:
		BinaryWriter_write_32(w, field.second.getInt());
		break;
	case Value::Type::CHAR:
		BinaryWriter_write_8(w, field.second.getChar());
		break;
	default:
		assert(false);
	}
}

Status Storage::save() {
	std::ofstream file(filename, std::ios_base::out | std::ios_base::binary);

	if (!file) {
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "Could not open file");
	}
	if (!file.good()) {
		file.close();
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "File is not good");
	}

	size_t size = MAX(1024, name.length()+description.length()+(64/8)*(2+2));
	auto w = bwrealloc(NULL, size);
	DEBUG_LOG_INFO("writer allocated");

	// header
	BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::HeaderBegin));
	DEBUG_LOG_INFO("written header begin");
	// name
	writeSize(w, name.length());
	BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(name.c_str()), name.length());
	DEBUG_LOG_INFO("writtern name");
	// description
	writeSize(w, description.length());
	BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(description.c_str()), description.length());
	DEBUG_LOG_INFO("written description");
	//
	BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::HeaderEnd));
	DEBUG_LOG_INFO("written header end");
	// end header
	
	if (BinaryWriter_available(w) < START_SIZE) {
		w = bwrealloc(w, START_SIZE);
	}

	DEBUG_LOG_INFO("Iteration over KV pairs");
	for (const NamedValue& entry : kv) {
		ENSURE_BASE_SIZE(size, w);
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::EntryBegin));
		BinaryWriter_write_16(w, static_cast<uint16_t>(Storage::EntryType::KV));
		
		writeSize(w, entry.first.length());
		EXPAND_WRITER(size, w, BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(entry.first.c_str()), entry.first.length()));
		
		ENSURE_BASE_SIZE(size, w);

		// valuetype
		BinaryWriter_write_32(w, static_cast<uint32_t>(entry.second.getType()));
		if (entry.second.getType() == Value::Type::CUSTOM) {
			assert(false);
			// KV do not support CUSTOM type
			//BinaryWriter_write_32(w, entry.second.getCustomType());
		}

		// size
		size_t sz;
		if (entry.second.getType() == Value::Type::STRING) {
			sz = strlen(entry.second.getCString());
		}
		else {
			sz = Value::SizeOf(entry.second.getType());
		}
		writeSize(w, sz);

		// data
		switch (entry.second.getType()) {
		case Value::Type::STRING:
			EXPAND_WRITER(size, w, BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(entry.second.getCString()), sz));
			break;
		case Value::Type::UINT:
			BinaryWriter_write_32(w, entry.second.getUInt());
			break;
		case Value::Type::INT:
			BinaryWriter_write_32(w, entry.second.getInt());
			break;
		case Value::Type::CHAR:
			BinaryWriter_write_8(w, entry.second.getChar());
			break;
		default:
			assert(false);
		}
		
		ENSURE_BASE_SIZE(size, w);
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::EntryEnd));
	}
	
	DEBUG_LOG_INFO("Iterating over records");
	for (const std::pair<std::string, const std::vector<NamedValue>&>& rec: records) {
		ENSURE_BASE_SIZE(size, w);
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::EntryBegin));
		DEBUG_LOG_INFO("written entry begin");
		BinaryWriter_write_16(w, static_cast<uint16_t>(Storage::EntryType::Record));
		DEBUG_LOG_INFO("written entrytype record");

		writeSize(w, rec.first.length());
		EXPAND_WRITER(size, w, BinaryWriter_write_buff(w, reinterpret_cast<const uint8_t*>(rec.first.c_str()), rec.first.length()));
		DEBUG_LOG_INFO("written rec.first (name)");

		ENSURE_BASE_SIZE(size, w);

		DEBUG_LOG_INFO("iteration over rec fields begin");
		for (const NamedValue& field : rec.second) {
			writeRecordField(w, size, field);
		}
		DEBUG_LOG_INFO("iteration over rec fields end");

		ENSURE_BASE_SIZE(size, w);
		BinaryWriter_write_32(w, static_cast<uint32_t>(Storage::SpecialBytes::EntryEnd));
		DEBUG_LOG_INFO("written entry end");
	}

	auto buff = *(w->buff);
	auto buff_size = w->size;
	BinaryWriter_free(w);

	file.write(reinterpret_cast<const char*>(buff), buff_size);
	delete[] buff;

	file.close();

	return Status::OK();
}


struct Buff {
	char** buff = NULL;
	size_t size = 0;
	size_t at = 0;
	bool _ready = 0;
};
// TODO: support for LITTLE ENDIAN, get a param
// TODO: ? convert to smth like BinaryWriter.
struct Reader {
	Buff buff;
	Buff sbuff;
	std::function<bool(Buff* sbuff)> nextBuff;
	bool good = true;

	explicit Reader(std::function<bool(Buff* sbuff)> nextBuff)
		: nextBuff(nextBuff) {
		if (!prepare(&buff)) good = false;
		prepare(&sbuff);
	}

	bool prepare(Buff* b = nullptr) {
		if(!b) {
			b = &sbuff;
		}
		auto v = nextBuff(b);
		b->_ready = v;
		return v;
	}
	bool swap() {
		if (!sbuff._ready) return false;

		auto b = buff;
		buff = sbuff;
		sbuff = b;

		prepare();

		return true;
	}

	bool isGood() const {
		return good;
	}

	void check() {
		if (!good) return;
		if (buff.at >= buff.size) {
			if (!swap()) {
				good = false;
			}
		}
	}
	
	size_t available() const {
		return (buff.size - buff.at) + (sbuff._ready ? sbuff.size : 0);
	}

	uint8_t read8() {
		uint8_t val = (*(buff.buff))[(buff.at)++];
		check();
		return val;
	}
	uint16_t read16() {
		uint16_t val1 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint8_t val2 = (*(buff.buff))[(buff.at)++];
		check();

		return (val1 << 8) | val2;
	}
	uint32_t read32() {
		uint32_t val1 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint32_t val2 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint16_t val3 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint8_t val4 = (*(buff.buff))[(buff.at)++];
		check();

		return (val1 << 24) | (val2 << 16) | (val3 << 8) | val4;
	}
	uint64_t read64() {
		uint64_t val1 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint64_t val2 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint64_t val3 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint64_t val4 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint32_t val5 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint32_t val6 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint16_t val7 = 0xff & (*(buff.buff))[(buff.at)++];
		check();
		uint8_t val8 = (*(buff.buff))[(buff.at)++];
		check();

		return (val1 << 56) | (val2 << 48) | (val3 << 40) | (val4 << 32) | (val5 << 24) | (val6 << 16) | (val7 << 8) | val8;
	}

	void read_buff(uint8_t* outbuff, size_t size) {
		size_t rest = size;
		size_t at = 0;

		while (rest > 0) {
			if (buff.at + rest >= buff.size) {
				size_t sz = buff.size - buff.at;
				memcpy_s(outbuff + at, size, (*buff.buff) + buff.at, sz);
				at += sz;
				buff.at += sz;
				rest -= sz;
				check();
			}
			else {
				memcpy_s(outbuff + at, size, (*buff.buff) + buff.at, rest);
				at += rest;
				buff.at += rest;
				rest -= rest;
			}
		}

		check();
	}


	uint32_t peek32() {
		uint32_t val1 = 0xff & (*(buff.buff))[(buff.at)];
		check();
		uint32_t val2 = 0xff & (*(buff.buff))[(buff.at)+1];
		check();
		uint16_t val3 = 0xff & (*(buff.buff))[(buff.at)+2];
		check();
		uint8_t val4 = (*(buff.buff))[(buff.at)+3];
		check();

		return (val1 << 24) | (val2 << 16) | (val3 << 8) | val4;
	}
};

inline size_t readSize(Reader* r) {
	if (sizeof(size_t) == sizeof(uint64_t)) {
		return r->read64();
	}
	else if (sizeof(size_t) == sizeof(uint32_t)) {
		return r->read32();
	}
}

inline const char* readstring(Reader* r) {
	size_t sz = readSize(r);
	char* buff = new char[sz+1];
	r->read_buff(reinterpret_cast<uint8_t*>(buff), sz);
	buff[sz] = '\0';
	return buff;
}

// TODO: extract read KV, read Record to functions.

Status Storage::readRecordField(Reader* r, std::vector<NamedValue>& out) {
	auto name = readstring(r);
	DEBUG_LOG_INFO("read name: " << name);
	auto vtype = static_cast<Value::Type>(r->read32());
	DEBUG_LOG_INFO("read vtype: " << (uint32_t)vtype);


	Value v;
	if (vtype == Value::Type::CUSTOM) {
		DEBUG_LOG_INFO("vtype is CUSTOM");

		auto vals = new std::vector<NamedValue>();

		auto custom_type = r->read32();
		assert(custom_type == (uint32_t)Storage::CustomType::Vector);
		DEBUG_LOG_INFO("read custom_type");
		
		if (r->read32() != static_cast<uint32_t>(Storage::SpecialBytes::EntryBegin)) {
			return Status(StatusCode::SC_ERROR_INVALID, "Expected EntryBegin");
		}
		DEBUG_LOG_INFO("read EntryBegin");

		auto _type = static_cast<Storage::EntryType>(r->read16());
		if (_type != Storage::EntryType::Record) {
			return Status(StatusCode::SC_ERROR_INVALID, "Expected Records type");
		}
		DEBUG_LOG_INFO("read type RECORD");
		auto _name = readstring(r);
		if (strcmp(name, _name) != 0) {
			return Status(StatusCode::SC_ERROR_INVALID, "Corrupted or invalid data");
		}

		DEBUG_LOG_INFO("reading fields of record begin");
		auto status = readRecordField(r, *vals);
		if (!status.isOk()) {
			return status;
		}
		DEBUG_LOG_INFO("reading fields of record end");

		v = Value(reinterpret_cast<void*>(vals), static_cast<uint32_t>(Storage::CustomType::Vector));
	
		if (r->read32() != static_cast<uint32_t>(Storage::SpecialBytes::EntryEnd)) {
			return Status(StatusCode::SC_ERROR_INVALID, "Expected EntryEnd");
		}
		DEBUG_LOG_INFO("read EntryEnd");
	}
	else {
		DEBUG_LOG_INFO("vtype is SIMPLE TYPE");

		auto rsize = readSize(r);
		switch (vtype) {
		case Value::Type::STRING:
		{
			char* str = new char[rsize + 1];
			r->read_buff(reinterpret_cast<uint8_t*>(str), rsize);
			str[rsize] = '\0';
			v = Value(str);
			delete[]str;
		}	break;
		case Value::Type::UINT:
			v = Value(r->read32());
			break;
		case Value::Type::INT:
			v = Value(static_cast<int32_t>(r->read32()));
			break;
		case Value::Type::CHAR:
			v = Value(r->read8());
			break;
		default:
			assert(false);
		}
	}

	out.push_back({name, v});
	if (r->peek32() == static_cast<uint32_t>(Storage::SpecialBytes::EntryEnd)) {
		DEBUG_LOG_INFO("reached EntryEnd");
		return Status::OK();
	}
	else {
		DEBUG_LOG_INFO("continue reading fields");
		return readRecordField(r, out);
	}
}

constexpr size_t BUFF_SIZE = BASE_SIZE * 10;
Status Storage::load() {
	std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);

	if (!file) {
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "Could not open file");
	}
	if (!file.good()) {
		file.close();
		return Status(StatusCode::SC_ERROR_UNAVAILABLE, "File is not good");
	}

	auto f = [&](Buff *b) -> bool {
		if (b->buff == NULL) {
			b->buff = new char* {};
		}
		if (*(b->buff) == NULL) {
			//*(b->buff) = new char[BUFF_SIZE];
			*(b->buff) = (char*)calloc(BUFF_SIZE, sizeof(char));
		}
		auto g = file.good();
		file.read(*(b->buff), BUFF_SIZE);
		size_t read = file.gcount();
		b->size = read;
		b->at = 0;
		
		return read > 0;
		};


	auto r = Reader(f);

	if (!r.isGood()) {
		std::cout << "here? " << (errno) << std::endl;
		assert(false);
		return Status(StatusCode::SC_ERROR_INVALID, "Could not read any");
	}

	if (r.read32() != static_cast<uint32_t>(Storage::SpecialBytes::HeaderBegin)) {
		return Status(StatusCode::SC_ERROR_INVALID, "Expected HeaderBegin");
	}
	DEBUG_LOG_INFO("read HeaderBegin");


	auto name = readstring(&r);
	DEBUG_LOG_INFO("read name: " << name);

	
	if (this->name != std::string(name)) {
		assert("Name in the file doesnt match Storage name, possible data corruption?" && 0);
	}

	auto descr = readstring(&r);
	DEBUG_LOG_INFO("read descr: " << descr);

	if (!this->description.empty()) {
		assert(false);
	}
	else {
		this->description = std::string(descr);
	}

	delete[]name;
	delete[]descr;

	if (r.read32() != static_cast<uint32_t>(Storage::SpecialBytes::HeaderEnd)) {
		return Status(StatusCode::SC_ERROR_INVALID, "Expected HeaderEnd");
	}
	DEBUG_LOG_INFO("read HeaderEnd");


	while (r.isGood() && r.available() > 0) {
		if (r.read32() != static_cast<uint32_t>(Storage::SpecialBytes::EntryBegin)) {
			return Status(StatusCode::SC_ERROR_INVALID, "Expected EntryBegin");
		}
		DEBUG_LOG_INFO("read EntryBegin");

		auto etype = static_cast<Storage::EntryType>(r.read16());
		DEBUG_LOG_INFO("read etype: " << (uint32_t)etype);

		auto ename = readstring(&r);
		DEBUG_LOG_INFO("read ename: " << ename);


		if (etype == Storage::EntryType::KV) {
			DEBUG_LOG_INFO("etype is KV");

			auto evt = static_cast<Value::Type>(r.read32());
			auto esz = readSize(&r);

			// TODO: check if kv alraedy exists with read key.
			Value v;
			switch (evt) {
			case Value::Type::STRING:
			{
				char* str = new char[esz+1];
				r.read_buff(reinterpret_cast<uint8_t*>(str), esz);
				str[esz] = '\0';
				v = Value(str);
				delete[]str;
			}	break;
			case Value::Type::UINT:
				v = Value(r.read32());
				break;
			case Value::Type::INT:
				v = Value(static_cast<int32_t>(r.read32()));
				break;
			case Value::Type::CHAR:
				v = Value(r.read8());
				break;
			default:
				assert(false);
			}

			auto key = std::string(ename);
			if (kv.find(key) != kv.end()) {
				return Status(StatusCode::SC_ERROR_ALREADY_EXISTS, "Key '" + key + "' already exists in KV storage");
			}
			kv[key] = v;

			delete[]ename;
		}
		else if (etype == Storage::EntryType::Record) {
			DEBUG_LOG_INFO("etype is RECORD");

			// TODO:
			std::vector<NamedValue> vals;
			DEBUG_LOG_INFO("read record fields begin");
			auto status = readRecordField(&r, vals);
			if (!status.isOk()) {
				return status;
			}
			DEBUG_LOG_INFO("read record fields end");
			records[std::string(ename)] = std::move(vals);
		}
		else {
			assert(false);
		}

		if (r.read32() != static_cast<uint32_t>(Storage::SpecialBytes::EntryEnd)) {
			return Status(StatusCode::SC_ERROR_INVALID, "Expected EntryEnd");
		}
		DEBUG_LOG_INFO("read EntryEnd");
	}

	return Status::OK();
}

// TODO: add Header -> version to save & load;
