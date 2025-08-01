#pragma once
#include "App.h"
#include "Global.h"


/*
    At leasta for now recursive alias are NOT supported;

    alias resolve <alias>
    alias list
    alias add <alias> <expansion>
    alias remove <alias>

*/

class AliasManagerApp :
    public App
{
public:
    static const std::string Name;

    virtual Status run(const char* const args[], int count) override;

private:


};

