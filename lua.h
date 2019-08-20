//LUA.H
//-------
//Everything about Lua, *but* binding what other systems add.
#pragma once
#include "support\sol.hpp"

extern sol::state Sol;
//extern lua_State* lua;

class Lua
{
public:
	static void Initialize();
	static void RunScript(std::string script);
	static void RunFile(std::string filename);
};

