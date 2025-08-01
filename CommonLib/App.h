#pragma once
#include <vector>
#include "Status.h"
#include "CommonLib.h"



class App
{
public:
	explicit App(std::string name) : name(name) {

	}

	//virtual Status run(std::vector<std::string> args);
	virtual Status run(const char* const args[], int count) = 0;
	virtual const std::string help() const = delete;
	// TODO: remove?!
	//virtual const std::vector<std::string> aliases() const;

protected:
	std::string name;

};

