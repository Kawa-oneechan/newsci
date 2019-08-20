#pragma once
#include "NewSCI.h"

class Serializer
{
private:
	static FILE* out;
public:
	static void Initialize();
	static void Load(std::string filename);
	static void StartSave(std::string filename);
	static void StartLoad(std::string filename);
	static void Finish();
	static void SetInteger(int i);
	static int GetInteger();
	static void SetString(std::string s);
	static std::string GetString();
};

