#pragma once
#include "App.h"
class RunApp :
    public App
{

public:
    static const std::string Name;

    explicit RunApp(std::string pluginDir) : App(), pluginDir(pluginDir) {
    }

    Status run(const char* const args[], int count) override;

private:
    const std::string pluginDir;

};

