// Plugins.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "Plugins.h"
#include "PluginLib.h"


//int luaFuncBuffWriter(lua_State* L, const void* p, size_t sz, void* ud) {
//	UNUSED(L);
//
//	luaL_Buffer* buff = (luaL_Buffer*)ud;
//	luaL_addstring(buff, (const char*)p);
//	
//	return 0;
//}


int luaFuncBuffWriter(lua_State* L, const void* p, size_t sz, void* ud) {
	UNUSED(L);

	BinaryBuffer* buff = (BinaryBuffer*)ud;
	BinaryBuffer_push(buff, (const char*)p, sz);

	return 0;
}


BinaryBuffer* readPluginFile(PluginInfo plugin, const char* plguinDirPath) {
	const size_t pluginName_len = strlen(plugin.name);
	const size_t plguinDirPath_len = strlen(plguinDirPath);

	const size_t filePathSize = pluginName_len + plguinDirPath_len + strlen(".lua") + 1;
	char* filePath = new char[filePathSize];

	strcpy_s(filePath, filePathSize, plguinDirPath);
	strcat_s(filePath, filePathSize, plugin.name);
	strcat_s(filePath, filePathSize, ".lua");

	auto readState = luaL_newstate();
	//auto res = luaL_loadfile(readState, filePath);
	auto res = luaL_loadfilex(readState, filePath, "t");

	if (res != LUA_OK) {
		std::cerr << "Could not load a file by path: '" << filePath << "'" << std::endl;
		std::cout << "Error message: " << lua_tostring(readState, -1) << std::endl;;
		assert(false);
		delete[] filePath;
		lua_close(readState);
		return NULL;
	}
	delete[] filePath;


	/*luaL_requiref(readState, "_G", luaopen_base, 1);
	lua_pop(readState, 1);

	auto callResult = lua_pcall(readState, 0, 0, 0);
	DEBUG_printLuaStackSize(readState);

	if (callResult != LUA_OK) {
		std::cerr << "Could not call loaded chunk" << std::endl;
		DEBUG_printLuaStackSize(readState);
		DEBUG_printLuaTypename(readState, -1);
		std::cerr << "Error: " << lua_tostring(readState, -1) << std::endl;
		assert(false);
		lua_close(readState);
		return NULL;
	}

	assert(false);
	return NULL;*/

	//const char* postfix = "_data";
	//const size_t pluginDataNameSize = pluginName_len + strlen(postfix) + 1;
	//char* pluginDataName = new char[pluginDataNameSize];
	//strcpy_s(pluginDataName, pluginDataNameSize, pluginName);
	//strcat_s(pluginDataName, pluginDataNameSize, postfix);

	//lua_getglobal(readState, pluginDataName);
	//if (!lua_istable(readState, -1)) {
	//	std::cerr << "could not find plugin table '" << pluginDataName << "'" << std::endl;
	//	assert(false);
	//	delete[] pluginDataName;
	//	return;
	//}
	//delete[] pluginDataName;

	DEBUG_printLuaStackSize(readState);
	DEBUG_printLuaTypename(readState, -1);
	

	//lua_getglobal(readState, pluginName);
	//if (!lua_isfunction(readState, -1)) {
	//	std::cerr << "could not find function '" << pluginName << "'" << std::endl;
	//	assert(false);
	//	lua_close(readState);
	//	return NULL;
	//}


	BinaryBuffer* pluginBuff = BinaryBuffer_init(500);

	if (lua_dump(readState, luaFuncBuffWriter, pluginBuff, false)) {
		std::cerr << "Error for func copy" << std::endl;
		assert(false);
		lua_close(readState);
		return NULL;
	}
	lua_pop(readState, 1);

	DEBUG_printLuaStackSize(readState);
	ASSERT_STACK_SIZE(readState, 0);

	lua_close(readState);

	return pluginBuff;
}

const char* getPluginDataName(PluginInfo plugin) {
	const char* postfix = "_data";
	const size_t pluginDataNameSize = strlen(plugin.name) + strlen(postfix) + 1;
	char* pluginDataName = new char[pluginDataNameSize];
	strcpy_s(pluginDataName, pluginDataNameSize, plugin.name);
	strcat_s(pluginDataName, pluginDataNameSize, postfix);

	return pluginDataName;
}

// TODO: check for PluginName, PluginName_data
lua_State* validateLoadedPlugin(PluginInfo plugin, BinaryBuffer* buff) {
	DEBUG_PRINT_DIVIDER();

	auto state = luaL_newstate();

	DEBUG_printLuaStackSize(state);

	luaL_loadbuffer(state, buff->data, buff->size, plugin.name);
	DEBUG_inspectLuaStack(state, -1);

	BinaryBuffer_free(buff);

	//addStandardLibraries(state);

	auto callResult = lua_pcall(state, 0, 0, 0);
	DEBUG_inspectLuaStack(state, -1);

	if (callResult == LUA_ERRSYNTAX || callResult == LUA_ERRRUN) {
		std::cout << "Error while plugin validation (" << plugin.name << "): `" << lua_tostring(state, -1) << "`" << std::endl;
		lua_close(state);
		return NULL;
	}

	if (callResult != LUA_OK) {
		std::cerr << "Could not call loaded chunk" << std::endl;
		DEBUG_printLuaStackSize(state);
		DEBUG_printLuaTypename(state, -1);
		std::cerr << "Error: " << lua_tostring(state, -1) << std::endl;
		assert(false);
		lua_close(state);
		return NULL;
	}


	lua_getglobal(state, plugin.name);
	DEBUG_inspectLuaStack(state, -1);
	if (!lua_isfunction(state, -1)) {
		std::cerr << "could not find function '" << plugin.name << "'" << std::endl;
		assert(false);
		lua_close(state);
		return NULL;
	}
	lua_pop(state, 1);

	
	const char* pluginDataName = getPluginDataName(plugin);
	lua_getglobal(state, pluginDataName);
	DEBUG_inspectLuaStack(state, -1);
	if (!lua_istable(state, -1)) {
		std::cerr << "could not find plugin table '" << pluginDataName << "'" << std::endl;
		assert(false);
		lua_close(state);
		delete[] pluginDataName;
		return NULL;
	}
	delete[] pluginDataName;
	lua_pop(state, 1);

	ASSERT_STACK_SIZE(state, 0);

	return state;
}

// ??????
void addStandardLibraries(lua_State* state) {
	luaL_requiref(state, "_G", luaopen_safe_base, 1);
	luaL_requiref(state, LUA_TABLIBNAME, luaopen_table, 1);
	luaL_requiref(state, LUA_STRLIBNAME, luaopen_string, 1);
	luaL_requiref(state, LUA_UTF8LIBNAME, luaopen_utf8, 1);
	luaL_requiref(state, LUA_MATHLIBNAME, luaopen_math, 1);
	lua_pop(state, 5);
}


void addPluginLib(lua_State* state) {
	luaL_requiref(state, "pluginLib", loadPluginLib, 1);
	lua_pop(state, 1);
}

// copies only string keys;
// values: nil, boolean, number, string, function. Unsupproted values will be added as nil;
// for now, at lest, exluded: LUA_TLIGHTUSERDATA, LUA_TTABLE, LUA_TUSERDATA, LUA_TTHREAD
void copyTable(lua_State* from, lua_State* to, const char* name) {

	const size_t fromStackSz = lua_gettop(from);
	const size_t toStackSz = lua_gettop(to);

	lua_getglobal(from, name);
	const int tableIndex = lua_gettop(from);

	lua_newtable(to);
	const int newTableIndex = lua_gettop(to);

	lua_pushnil(from);
	while (lua_next(from, tableIndex) != 0) {
		if (lua_type(from, -2) != LUA_TSTRING) {
			lua_pop(from, 1); // pop value;
			continue;
		}

		size_t keyLen;
		const char* key = lua_tolstring(from, -2, &keyLen);

		lua_pushlstring(to, key, keyLen);

		switch (lua_type(from, -1)) {
		case LUA_TBOOLEAN:
		{
			const bool v = lua_toboolean(from, -1);
			lua_pushboolean(to, v);
		}	break;

		case LUA_TNUMBER:
		{
			if (lua_isinteger(from, -1)) {
				const int64_t v = lua_tointeger(from, -1);
				lua_pushinteger(to, v);
			}
			else {
				const double v = lua_tonumber(from, -1);
				lua_pushnumber(to, v);
			}
		}	break;

		case LUA_TSTRING:
		{
			size_t len;
			const char* str = lua_tolstring(from, -1, &len);
			lua_pushlstring(to, str, len);
			delete str;
		}	break;

		case LUA_TFUNCTION:
		{
			// TODO:
		}	break;

		case LUA_TNIL:
		default:
		{
			lua_pushnil(to);
		}	break;
		}

		lua_settable(to, newTableIndex);

		lua_pop(from, 1); //value
	}
	lua_pop(from, -1); //table

	lua_setglobal(to, name);

	ASSERT_STACK_SIZE(from, fromStackSz);
	ASSERT_STACK_SIZE(to, toStackSz);
}

// TODO:
bool validatePluginDataTable(PluginInfo plugin, lua_State* validationState) {
	return true;
}

lua_State* extractPluginContent(PluginInfo plugin, lua_State* validationState) {
	lua_getglobal(validationState, plugin.name);
	BinaryBuffer* pluginFuncBuff = BinaryBuffer_init();
	if (lua_dump(validationState, luaFuncBuffWriter, pluginFuncBuff, false)) {
		std::cerr << "Error for func copy" << std::endl;
		assert(false);
		lua_close(validationState);
		return NULL;
	}
	lua_pop(validationState, 1);
	ASSERT_STACK_SIZE(validationState, 0);

	auto state = luaL_newstate();

	if (LUA_OK != luaL_loadbuffer(state, pluginFuncBuff->data, pluginFuncBuff->size, plugin.name)) {
		std::cerr << "ERROR" << std::endl;
		DEBUG_printLuaStackSize(state);
		DEBUG_printLuaTypename(state, -1);
		std::cerr << "Error: " << lua_tostring(state, -1) << std::endl;
		assert(false);
		lua_close(validationState);
		lua_close(state);
		return NULL;
	}
	lua_setglobal(state, plugin.name);
	BinaryBuffer_free(pluginFuncBuff);

	const char* pluginDataName = getPluginDataName(plugin);
	copyTable(validationState, state, pluginDataName);
	delete[] pluginDataName;
	
	ASSERT_STACK_SIZE(validationState, 0);
	ASSERT_STACK_SIZE(state, 0);

	lua_close(validationState);
	return state;
}

//void addRuntimeInfo(PluginInfo plugin, lua_State* state) {
//	lua_newtable(state);
//}

// TODO: add libs
static const size_t PLUGIN_RETURN_VALUE_COUNT = 2;
bool executePlugin(PluginInfo plugin, lua_State* executableState, const char** args, long long* res, char** resMsg) {
	auto state = executableState;

	lua_getglobal(state, plugin.name);
	
	lua_pushinteger(state, plugin.argc);
	for (size_t i = 0; i < plugin.argc; i++) {
		lua_pushstring(state, args[i]);
	}

	addStandardLibraries(state);
	addPluginLib(state);
	//addRuntimeInfo(plugin, state);
	
	auto status = lua_pcall(state, plugin.argc + 1, PLUGIN_RETURN_VALUE_COUNT, 0);
	if (status != 0) {
		// TODO:? maybe instead of printing error message, just put it in resMsg;
		std::cout << "Error from plugin (lcode=" << status << "): " << std::endl;
		std::cout << lua_tostring(state, -1) << std::endl;
		lua_pop(state, 1);
		lua_close(state);
		return false;
	}

	size_t msgLen;
	if (lua_isstring(state, -1)) {
		const char* msg = lua_tolstring(state, -1, &msgLen);
		*resMsg = new char[msgLen+1];
		strcpy_s(*resMsg, msgLen+1, msg);
	}
	else {
		*resMsg = new char[1] { '\0' };
	}
	long long code = lua_tointeger(state, -2);
	lua_pop(state, 2);
	*res = code;


	ASSERT_STACK_SIZE(state, 0);
	lua_close(state);
	return true;
}



Status runPlugin(const char* name, const char** args, const size_t argc, const char* pluginDirPath) {
	PluginInfo pl = {
		name,
		argc
	};

	setCurrentPlugin(pl);

	auto buff = readPluginFile(pl, pluginDirPath);
	auto vs = validateLoadedPlugin(pl, buff);
	if (!vs) {
		return Status(StatusCode::SC_ERROR_INVALID, "Plugin '" + std::string(pl.name) + "' has not passed validation!");
		//std::cout << "Plugin '" << pl.name << "' has not passed validation!" << std::endl;
		//return;
	}
	auto es = extractPluginContent(pl, vs);
	if (!es) {
		return Status(StatusCode::SC_ERROR, "Error while preparing for execution, plugin: '" + std::string(pl.name) + "'");
		//std::cout << "Error while preparing for execution, plugin: '" << pl.name << "'" << std::endl;
		//return;
	}

	long long res;
	char* msg;
	auto st = executePlugin(pl, es, args, &res, &msg);
	if (!st) return StatusCode::SC_ERROR;

	unsetCurrentPlugin();

	//std::cout << "Plugin finished, code=" << res << std::endl;
	//std::cout << "Message: " << msg << std::endl;

	return Status(StatusCode::SC_OK, "Plugin finished, code=" + std::to_string(res) + "\n" + "Message: " + std::string(msg));
}


static PluginInfo* currentPlugin = NULL;
void setCurrentPlugin(PluginInfo pl) {
	if (currentPlugin == NULL) {
		currentPlugin = (PluginInfo*)calloc(1, sizeof(PluginInfo));
	}
	memcpy_s(currentPlugin, sizeof(PluginInfo), &pl, sizeof(PluginInfo));
}
void unsetCurrentPlugin() {
	delete currentPlugin;
	currentPlugin = NULL;
}
const PluginInfo* getCurrentPlugin() {
	return currentPlugin;
}
