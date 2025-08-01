#pragma once

#include "lua.hpp"

#include <iostream>
#include <assert.h>
#include "Buffer.h"


#define UNUSED(x) (void)x

#ifdef _DEBUG
	#ifdef _WIN32
	#define HELPER_WIN_DEBUG
	#endif
#endif

#ifdef HELPER_WIN_DEBUG
#include <windows.h>
static void W32printCurrectDirectory() {
	{
		char buffer[MAX_PATH];
		DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer); // For ANSI characters
		// Or use GetCurrentDirectoryW for Unicode characters
		if (length > 0 && length < MAX_PATH) {
			std::cout << "Current directory: " << buffer << std::endl;
		}
		else {
			std::cerr << "Failed to get current directory." << std::endl;
		}
	}
}
#define W32printCurrectDirectory() W32printCurrectDirectory()
#else
#define W32printCurrectDirectory()
#endif

static inline int getLuaStackSize(lua_State* L) {
	return lua_gettop(L);
}

#ifdef _DEBUG
#define DEBUG_PRINT_DIVIDER() do { std::cout << "--- ===== ===== ===== ---" << std::endl; } while(0)
#define DEBUG_printLuaStackSize(state) do { std::cout << "lua stack size: " << getLuaStackSize(state) << std::endl; } while(0)
#define DEBUG_printLuaTypename(state, idx) do { std::cout << "lua stack(" << idx << ") typename: " << luaL_typename(state, idx) << std::endl; } while(0)
#define DEBUG_inspectLuaStack(state, idx) do {\
	std::cout << "[inspect] at " << __FILE__ << ":" << __LINE__ << std::endl; \
	int __stsize = getLuaStackSize(state); \
	std::cout << "[inspect] stack size: " << __stsize << std::endl; \
	if (ABS(idx) > __stsize) { std::cout << "[inspect] " << idx << " greater than a stack size" << std::endl; break; } \
	std::cout << "[inspect] "; DEBUG_printLuaTypename(state, idx); \
	if (lua_type(state, idx) == LUA_TSTRING) { std::cout << "[inspect] " << "value is a string: ``" << lua_tostring(state, idx) << "``" << std::endl; } } while(0)
#else
#define DEBUG_PRINT_DIVIDER()
#define DEBUG_printLuaStackSize(state)
#define DEBUG_printLuaTypename(state, idx)
#define DEBUG_inspectLuaStack(state, idx)
#endif

#define ASSERT_STACK_SIZE(state, n) assert("Bad stack size" && getLuaStackSize(state) == n)


static inline const char* getPluginDirectory() {
	return ".\\";
}

void runPlugin(const char* name, const char** args, const size_t argc);

typedef struct {
	const char* name;
	const int argc;
} PluginInfo;

// plguinDirPath should be "" or end with a slash.
BinaryBuffer* readPluginFile(PluginInfo plugin, const char* plguinDirPath = getPluginDirectory());


//typedef enum {
//	PI_NEW,
//	PI_READ,
//	PI_VALIDATE,
//} PluginInfoState;
//typedef struct {
//	PluginInfoState state;
//	const char* pluginName;
//	const char* pluginDirPath;
//	BinaryBuffer* buffer;
//} PluginInfo;


lua_State* validateLoadedPlugin(PluginInfo plugin, BinaryBuffer* p);


// Feature: allow to customize what libraries are allowed for each plugin.
void addStandardLibraries(lua_State* state);


// TODO:
bool validatePluginDataTable(PluginInfo plugin, lua_State* validationState);

//typedef struct {
//	lua_State* state;
//} PluginContent;

lua_State* extractPluginContent(PluginInfo plugin, lua_State* validationState);


bool executePlugin(PluginInfo plugin, lua_State* executableState, const char** args, long long* res, char** resMsg);

//void addRuntimeInfo(PluginInfo plugin, lua_State* state);

void addStandardLibraries(lua_State* state);
void addPluginLib(lua_State* state);

void setCurrentPlugin(PluginInfo pl);
void unsetCurrentPlugin();
const PluginInfo* getCurrentPlugin(void);


// TODO: Lua Plugin library;




