#pragma once
#include "types.h"

class IniFile
{
	typedef std::map<char*, char*> IniList;

public:
	std::map<char*, IniList> sections;

	char* Get(const char* section, const char* value, char* dft);
	void Load(const char* filename);
};
