#include "Global.h"


Global *Global::instance = new Global();

int Global::init()
{
	if (_initialized) return 0;


	// Apps
	_apps = {
		{ std::string(ConverterApp::Name), new ConverterApp() },
		{ std::string(RunApp::Name), new RunApp(PLUGIN_DIR) },
		{ std::string(AliasManagerApp::Name), new AliasManagerApp() },
	};


	// AliasManager
	auto status = _aliasManager.init({
			{"converter", ConverterApp::Name},
			{"conv", ConverterApp::Name},
			{"run", RunApp::Name},
			{"alias", AliasManagerApp::Name},
		});

	if (status.isError()) {
		std::cout << "Could not initialize AliasManager: " << status.msg << std::endl;
		return status.intCode();
	}


	_initialized = true;

	return 0;
}
