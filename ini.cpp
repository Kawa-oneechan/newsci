#include "NewSCI.h"
#include <map>
#include <vector>
#include "ini.h"

char* IniFile::Get(const char* section, const char* value, char* dft)
{
	for (auto tit = sections.begin(); tit != sections.end(); tit++)
	{
		if (!_stricmp(tit->first, section))
		{
			for (auto tat = tit->second.begin(); tat != tit->second.end(); tat++)
			{
				if (!_stricmp(tat->first, value))
					return tat->second;
			}
		}
	}
	return dft;
}

void IniFile::Load(const char* filename)
{
	FILE* fd;
	fopen_s(&fd, filename, "r");
	char buffer[255];
	char* b = buffer;
	auto thisSect = new IniList();
	char* sectName;
	while(!feof(fd))
	{
		char c = fgetc(fd);
		if (c == '[')
		{
			if (thisSect->size()) sections[sectName] = *thisSect;
			thisSect->clear();
			b = buffer;
			while (!feof(fd))
			{
				c = fgetc(fd);
				if (c == ']') break;
				*b++ = c;
			}
			*b++ = 0;
			sectName = _strdup(buffer);
		}
		else if (c == ';')
			while (fgetc(fd) != '\n');
		else if (iswspace(c))
			continue;
		else
		{
			//finally, some good fucking data.
			b = buffer;
			*b++ = c;
			while (!feof(fd))
			{
				c = fgetc(fd);
				if (c == '=')
					break;
				if (iswspace(c))
					continue;
				*b++ = c;
			}
			*b++ = 0;
			char* key = _strdup(buffer);
			char* valStart = b;
			while (!feof(fd))
			{
				c = fgetc(fd);
				if (c == '\n' || c == '\r')
					break;
				if (c == ';')
				{
					while (!feof(fd) && fgetc(fd) != '\n');
					break;
				}
				if (iswspace(c))
					continue;
				*b++ = c;
			}
			*b++ = 0;
			//we now have our key and value -- add them.
			thisSect->insert(std::pair<char*, char*>(key, _strdup(valStart)));
		}
	}
	//add final section if any
	if (thisSect->size()) sections[sectName] = *thisSect;
}
