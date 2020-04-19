#include "NewSCI.h"

//TODO: add more types?

FILE* Serializer::out = NULL;

void Serializer::Initialize()
{
	auto s = Sol.create_table();
	s.set_function("Load", &Serializer::Load);
	s.set_function("StartSave", &Serializer::StartSave);
	s.set_function("StartLoad", &Serializer::StartLoad);
	s.set_function("Finish", &Serializer::Finish);
	s.set_function("SetInteger", Serializer::SetInteger);
	s.set_function("GetInteger", Serializer::GetInteger);
	s.set_function("SetString", Serializer::SetString);
	s.set_function("GetString", Serializer::GetString);
	Sol["Serializer"] = s;
}

void Serializer::Load(std::string filename)
{
	Audio::CloseAll();
	Lua::Initialize();
	Serializer::Initialize();
	Audio::Initialize();
	View::Initialize();
	Lua::RunFile("engine.lua");
	Sol["restarting"] = true;
	Sol["Deserialize"](filename);
}

void serializeThing(std::pair<sol::object, sol::object> thing, int indent);

void serializeTable(sol::table table, int indent)
{
	Serializer::SetInteger(5);
	Serializer::SetInteger(table.size());
	for (auto thing : table)
		serializeThing(thing, indent);
}

void serializeThing(std::pair<sol::object, sol::object> thing, int indent)
{
	auto keyType = thing.first.get_type();
	auto valType = thing.second.get_type();
	std::string key;
	if (keyType == sol::type::string)
	{
		key = thing.first.as<std::string>();
		if (key == "base")
			return;
		if (key == "math")
			return;
		if (key[0] == '_')
			return;
		if (key.substr(0, 4) == "sol.")
			return;
		printf("\n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("\"%s\", %d", key.c_str(), valType);
	}
	else if (keyType == sol::type::number)
	{
		char lol[16];
		sprintf_s(lol, 16, "#%d", thing.first.as<int>());
		key = std::string(lol);
	}

	if (valType == sol::type::number)
	{
		Serializer::SetString(key);
		Serializer::SetInteger(3);
		Serializer::SetInteger(thing.second.as<int>());
		printf(" -> %f", thing.second.as<float>());
	}
	else if (valType == sol::type::boolean)
	{
		Serializer::SetString(key);
		Serializer::SetInteger(1);
		Serializer::SetInteger((int)thing.second.as<bool>());
		printf(" -> %s", thing.second.as<bool>() ? "true" : "false");
	}
	else if (valType == sol::type::string)
	{
		Serializer::SetString(key);
		Serializer::SetInteger(4);
		Serializer::SetString(thing.second.as<std::string>());
		printf(" -> \"%s\"", thing.second.as<std::string>().c_str());
	}
	else if (valType == sol::type::table)
	{
		Serializer::SetString(key);
		serializeTable(thing.second.as<sol::table>(), indent + 1);
	}
	else if (valType == sol::type::function)
		printf(" -> function");
	else if (valType == sol::type::thread)
		printf(" -> thread");
	else if (valType == sol::type::userdata)
		printf(" -> user data");
	else if (valType == sol::type::lightuserdata)
		printf(" -> light user data");
}

void Serializer::StartSave(std::string filename)
{
	if (Serializer::out != NULL)
		return;
	fopen_s(&Serializer::out, filename.c_str(), "wb");

	for (auto thing : Sol)
		serializeThing(thing, 0);
	Serializer::SetInteger(0x99999999);
	Serializer::Finish();
}

void Serializer::StartLoad(std::string filename)
{
	if (Serializer::out != NULL)
		return;
	fopen_s(&Serializer::out, filename.c_str(), "rb");
}

void Serializer::Finish()
{
	if (Serializer::out == NULL)
		return;
	fflush(out);
	fclose(out);
	out = NULL;
}

void Serializer::SetInteger(int i)
{
	if (Serializer::out == NULL)
		return;
	fwrite(&i, sizeof(int), 1, Serializer::out);
}

int Serializer::GetInteger()
{
	if (Serializer::out == NULL)
		return 0;
	int i = 0;
	fread(&i, sizeof(int), 1, Serializer::out);
	return i;
}

void Serializer::SetString(std::string s)
{
	if (Serializer::out == NULL)
		return;
	SetInteger(s.length());
	fwrite(s.c_str(), sizeof(char), s.length(), Serializer::out);
}

std::string Serializer::GetString()
{
	if (Serializer::out == NULL)
		return "NULL";
	auto len = GetInteger() + 1;
	char* str = (char*)malloc(sizeof(char) * len);
	memset(str, 0, sizeof(char) * len);
	fread(str, sizeof(char), len - 1, Serializer::out);
	auto r = std::string(str);
	free(str);
	return r;
}

