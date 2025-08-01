#include "AliasManager.h"
#include <sstream>

//bool AliasManager::registerAlias(std::string alias, std::string expansion)
//{
//	std::istringstream sstream(expansion);
//
//	std::string part;
//	std::vector<std::string> parts;
//	while (sstream >> part)
//		parts.push_back(part);
//
//	return registerAlias(alias, parts);
//}
//
//bool AliasManager::registerAlias(std::string alias, vector<std::string> expansion)
//{
//	if (_aliases.find(alias) != _aliases.end()) return false;
//
//	_aliases.insert({alias, expansion});
//
//	return true;
//}
//
//const Result<std::string> AliasManager::findAlias(std::string alias)
//{
//	auto a = findAliasParts(alias);
//	if (a.getStatus().isError()) return {a.getStatus(), nullptr};
//
//	std::string *res = new std::string( a.getValue()->begin(), a.getValue()->end() );
//	return { StatusCode::SC_OK, std::move(res) };
//}
//
//const Result<std::vector<std::string>> AliasManager::findAliasParts(std::string alias)
//{
//	if (_aliases.find(alias) == _aliases.end()) return { StatusCode::SC_ERROR_NOT_FOUND, nullptr };
//
//	return { StatusCode::SC_OK, &_aliases[alias] };
//}

bool AliasManager::save()
{
	auto s = st->deleteRecord<std::vector<std::string>>("_registered");
	if (s.isError() && s.code != StatusCode::SC_ERROR_NOT_FOUND) return false;
	s = st->writeRecord("_registered", &_registered);
	if (s.isError()) return false;

	s = st->save();
	if (s.isError()) return false;

	return true;
}

bool AliasManager::registerAlias(std::string alias, std::string expansion)
{
	if (!st->isLoaded()) return false;

	if (st->writeKVString(alias, expansion).isError()) return false;

	_registered.push_back(alias);

	return save();
}

bool AliasManager::registerAlias(std::string alias, vector<std::string> expansion)
{
	std::string s;
	for (auto& p : expansion) s += p;
	return registerAlias(alias, s);
}

bool AliasManager::unregisterAlias(std::string alias)
{
	if (!st->isLoaded()) return false;

	auto tmp = new std::string;
	if (st->readKVString(alias, tmp).isError()) return false;
	delete tmp;

	auto stat = st->deleteKVString(alias);
	assert(stat.isOk());

	auto it = std::find(_registered.begin(), _registered.end(), alias);
	assert(it != _registered.end());

	_registered.erase(it);
	
	return save();
}

const Result<std::string> AliasManager::findAlias(std::string alias) const
{
	if (!st->isLoaded()) return { Status(StatusCode::SC_ERROR_UNAVAILABLE, "AliasManager was not initialized") };

	std::string *out = new std::string;
	auto status = st->readKVString(alias, out);
	if (status.isError()) return status;

	return { Status::OK(), out };
}

const Result<std::vector<std::string>> AliasManager::findAliasParts(std::string alias) const
{
	auto r = findAlias(alias);
	if (r.getStatus().isError()) return r.getStatus();


	std::istringstream sstream(*r.getValue());

	std::string part;
	std::vector<std::string> *parts = new std::vector<std::string>();
	while (sstream >> part)
		parts->push_back(part);

	return { Status::OK(), std::move(parts) };
}


Status AliasManager::init(std::initializer_list<std::pair<std::string, std::string>> defaultAliases, bool strict)
{
	using T = std::vector<std::string>;
	st->registerTypeSerializer<T>([](const T* v, Storage::RecordBuffer* b) -> Status {
		auto status = b->writeIntField("__size__", v->size());
		if (status.isError()) return status;

		for (int i = 0; i < v->size(); i++) {
			status = b->writeStringField(std::to_string(i), (*v)[i]);
			if (status.isError()) return status;
		}

		return StatusCode::SC_OK;
	});
	st->registerTypeDeserializer<T>([](T** v, const Storage::RecordView* b) -> Status {
		*v = new T();

		int size;
		auto status = b->readIntField("__size__", &size);
		if (status.isError()) return status;

		for (int i = 0; i < size; i++) {
			std::string item;
			status = b->readStringField(std::to_string(i), &item);
			if (status.isError()) return status;
			(*v)->push_back(item);
		}

		return StatusCode::SC_OK;
	});

	auto status = st->load(true);
	if (status.isError()) return status;

	T* _reg = nullptr;
	status = st->readRecord("_registered", &_reg);
	if (status.code == StatusCode::SC_ERROR_NOT_FOUND) {
		_registered = {};
	}
	else {
		_registered = *_reg;
		delete _reg;
	}

	int skipped = 0;
	for (auto it = defaultAliases.begin(); it != defaultAliases.end(); it++) {
		if (findAlias(it->first).getStatus().isOk()) {
			// Alias already exists

			if (strict) {
				return Status(StatusCode::SC_ERROR_ALREADY_EXISTS, it->first);
			}

			skipped++;
			continue;
		}

		registerAlias(it->first, it->second);
	}

	status = std::move(st->save());
	if (status.isError()) return status;

	return Status(StatusCode::SC_OK, std::to_string(skipped));
}
