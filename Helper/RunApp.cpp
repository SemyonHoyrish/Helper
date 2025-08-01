#include "RunApp.h"
#include "Plugins.h"

const std::string RunApp::Name = "Run";

Status RunApp::run(const char* const args[], int count)
{
	std::cout << "Loading plugin" << std::endl;
	
	//todo argc > 1;
	const char* pluginName = args[0];
	const char** plargs = new const char* [count - 1];

	for (int i = 1; i < count; i++) {
		plargs[i - 1] = args[i];
	}

	return runPlugin(pluginName, plargs, count - 1, this->pluginDir.c_str());
}

