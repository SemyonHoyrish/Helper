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

bool AliasManager::registerAlias(std::string alias, std::string expansion)
{
	if (!st->isLoaded()) return false;

	if (st->writeKVString(alias, expansion).isError()) return false;

	st->save();

	return true;
}

bool AliasManager::registerAlias(std::string alias, vector<std::string> expansion)
{
	std::string s;
	for (auto& p : expansion) s += p;
	return registerAlias(alias, s);
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
