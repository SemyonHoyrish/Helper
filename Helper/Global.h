#pragma once
#include <iostream>
#include <map>

#include "AliasManager.h"

#include "App.h"
#include "RunApp.h"
#include "ConverterApp.h"
#include "AliasManagerApp.h"


static constexpr const char* STORAGE_DIR = "./storage/";
static constexpr const char* PLUGIN_DIR = "./plugins/";

static constexpr const char* ALIASMANAGER_FILENAME = "aliasmanager";

class Global
{
public:

	static Global *instance;
	int init();


	const std::map<std::string, App*>& apps() const { return _apps; }
	AliasManager* aliasManager() { return &_aliasManager; };
	
private:

	explicit Global()
		: _aliasManager(ALIASMANAGER_FILENAME, STORAGE_DIR)
	{
	}


private:
	bool _initialized = false;

	std::map<std::string, App*> _apps;
	AliasManager _aliasManager;


};
