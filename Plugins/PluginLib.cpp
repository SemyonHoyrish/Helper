#include "pch.h"
#include "PluginLib.h"

int pl_print(lua_State* state) {
	const int n = lua_gettop(state);

	std::cout << "[" << getCurrentPlugin()->name << ":: prints] ";
	for (int i = 1; i <= n; ++i) {
		const char* v = lua_tostring(state, i);
		std::cout << v << (i < n ? "\t" : "");
	}
	std::cout << std::endl;

	return 0;
}

int pl_input(lua_State* state) {
	if (lua_gettop(state) != 0) {
		lua_pushliteral(state, "Input does not expect any arguments"); // TODO: add usefil arguments
		lua_error(state);
		return 0;
	}

	char* buff = new char[1024];
	std::cout << "[" << getCurrentPlugin()->name << ":: requires your input] ";
	std::cin.getline(buff, 1024);
	auto read = std::cin.gcount();

	lua_pushlstring(state, buff, read);
	delete[] buff;
	return 1;
}

const char* newString(const char* str) {
	char* nstr = new char[strlen(str) + 1];
	strcpy_s(nstr, strlen(str) + 1, str);
	return nstr;
}

static const luaL_Reg funcs[] = {
		{newString("print"), pl_print},
		{newString("input"), pl_input},
};

int loadPluginLib(lua_State* state) {
	luaL_newlib(state, funcs);
	return 1;
}
