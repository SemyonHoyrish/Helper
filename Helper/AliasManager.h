#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <initializer_list>
#include "Result.h"
#include "Status.h"
#include "Storage.h"


class AliasManager
{
	template <typename T>
	using vector = std::vector<T>;

	template <typename K, typename V>
	using map = std::unordered_map<K, V>;

public:

	AliasManager(std::string storageName, std::string storageDirPath)
	{
		st = new Storage(storageName, storageDirPath);
	}

	Status init() { return init({}); }
	Status init(std::initializer_list<std::pair<std::string, std::string>> defaultAliases, bool strict = false);

	bool registerAlias(std::string alias, std::string expansion);
	bool registerAlias(std::string alias, vector<std::string> expansion);

	bool unregisterAlias(std::string alias);

	const Result<std::string> findAlias(std::string alias) const;
	const Result<vector<std::string>> findAliasParts(std::string alias) const;

	const std::vector<std::string>& registered() const { return _registered; }

private:
	bool save();

private:
	Storage* st;

	std::vector<std::string> _registered;
	

};

