#include "NewSCI.h"

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

void Serializer::StartSave(std::string filename)
{
	if (Serializer::out != NULL)
		return;
	fopen_s(&Serializer::out, filename.c_str(), "wb");
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

