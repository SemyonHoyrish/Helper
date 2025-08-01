#include "AliasManagerApp.h"

const std::string AliasManagerApp::Name = "AliasManager";

Status AliasManagerApp::run(const char* const args[], int count)
{
	if (count < 1) {
		return Status(StatusCode::SC_ERROR_INSUFFICIENT_DATA, "expected at least 1 argument");
	}

	auto command = std::string(args[0]);
	args++;
	count--;

	if (command == "resolve") {
		if (count < 1) {
			return Status(StatusCode::SC_ERROR_INSUFFICIENT_DATA, "[resolve] 1 argument");
		}

		auto alias = std::string(args[0]);

		auto r = Global::instance->aliasManager()->findAlias(alias);
		if (r.getStatus().isError()) {
			if (r.getStatus().code == StatusCode::SC_ERROR_NOT_FOUND) {
				return Status(StatusCode::SC_ERROR_NOT_FOUND, "alias was not found");
			}
			else {
				return r.getStatus();
			}
		}
		return Status(StatusCode::SC_OK, *r.getValue());
	}
	else if (command == "list") {
		for (const auto& alias : Global::instance->aliasManager()->registered()) {
			auto r = Global::instance->aliasManager()->findAlias(alias);
			assert(r.getStatus().isOk());
			std::cout << alias << " -> \"" << *r.getValue() << "\"" << std::endl;
		}
		return StatusCode::SC_OK;
	}
	else if (command == "add") {
		if (count < 2) {
			return Status(StatusCode::SC_ERROR_INSUFFICIENT_DATA, "[add] 2 arguments");
		}

		auto alias = std::string(args[0]);
		auto exp = std::string(args[1]);
		
		auto r = Global::instance->aliasManager()->registerAlias(alias, exp);
		if (!r) return StatusCode::SC_ERROR;

		return StatusCode::SC_OK;
	}
	else if (command == "remove") {
		if (count < 1) {
			return Status(StatusCode::SC_ERROR_INSUFFICIENT_DATA, "[remove] 1 argument");
		}

		auto alias = std::string(args[0]);
		auto r = Global::instance->aliasManager()->unregisterAlias(alias);
		if (!r) return StatusCode::SC_ERROR;

		return Status::OK();
	}
	else {
		return Status(StatusCode::SC_ERROR_NOT_FOUND, "'" + command + "' is not a valid AliasManagerApp command");
	}

	return StatusCode::SC_OK;
}
