#pragma once
#include <vector>
#include "Status.h"
#include "CommonLib.h"



class App
{
public:
	static const std::string Name;

	explicit App() : _name(std::string(Name)) {

	}

	//virtual Status run(std::vector<std::string> args);
	virtual Status run(const char* const args[], int count) = 0;
	virtual const std::string help() const = delete;
	// TODO: remove?!
	//virtual const std::vector<std::string> aliases() const;

	std::string name() const { return _name; }

protected:
	std::string _name;

};
