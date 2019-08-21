#include "NewSCI.h"
#include <SDL_messagebox.h>
#include "support/fmt/format.h"

namespace Pack
{
	std::string packFileName = "resource.nws";
	FILE* packFileHd = NULL;
	int packFileCt = 0;
	PackFileRecord* packFileList = NULL;

	void Load()
	{
		Uint32 signature;

		if (fopen_s(&packFileHd, packFileName.c_str(), "rb") != 0)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "NewSCI", fmt::format("Could not load resource pak \"{}\".", packFileName).c_str(), NULL);
			exit(1);
		}
		fread(&signature, sizeof(Uint32), 1, packFileHd);
		if (signature != 0x626D7564)
		{
			fclose(packFileHd);
			SDL_LogCritical(SDL_LOG_CATEGORY_ASSERT, "File \"%s\" is not a valid dumb pack.", packFileName.c_str());
			packFileCt = 0;
		}
		else
			fread(&packFileCt, sizeof(Uint32), 1, packFileHd);

		packFileList = (PackFileRecord*)malloc(sizeof(PackFileRecord) * packFileCt);

		PackFileRecord* pfr = packFileList;
		for (int i = 0; i < packFileCt; i++)
		{
			Uint8 nameLen = 0;
			fread(&nameLen, sizeof(Uint8), 1, packFileHd);
			fread(pfr->filename, sizeof(char), nameLen, packFileHd);
			pfr->filename[nameLen] = 0;
			fread(&pfr->offset, sizeof(Uint32), 1, packFileHd);
			fread(&pfr->size, sizeof(Uint32), 1, packFileHd);
			pfr++;
		}
	}

	PackFileRecord* Find(std::string filename)
	{
		PackFileRecord* pfr = packFileList;
		for (int i = 0; i < packFileCt; i++)
		{
			if (!strcmp(pfr->filename, filename.c_str()))
				return pfr;
			pfr++;
		}
		return NULL;
	}

	char* Read(PackFileRecord* pfr)
	{
		if (pfr == NULL)
			return NULL;
		char* ret = (char*)malloc(pfr->size + 1);
		memset(ret, 0, pfr->size + 1);
		fseek(packFileHd, pfr->offset, SEEK_SET);
		fread(ret, sizeof(char), pfr->size, packFileHd);
		return ret;
	}

	char* Read(std::string filename)
	{
		return Read(Find(filename));
	}
}

Handle LoadFile(std::string filename, unsigned long *size)
{
	auto looseFile = fmt::format("resource\\{}", filename);
	FILE* fd;
	if (fopen_s(&fd, looseFile.c_str(), "rb") != 0)
	{
		Pack::PackFileRecord* pfr = Pack::Find(filename);
		if (pfr)
		{
			SDL_LogVerbose(SDL_LOG_CATEGORY_SYSTEM, "LoadFile: Loading \"%s\" from pack.", filename.c_str());
			if (size) *size = pfr->size;
			return Pack::Read(pfr);
		}
		SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "LoadFile: Could not open \"%s\".", filename.c_str());
		return 0;
	}

	fseek(fd, 0, SEEK_END);
	auto len = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	char* res = (char*)malloc(len);
	fread(res, sizeof(char), len, fd);
	fclose(fd);
	if (size) *size = len;
	return res;
}

Handle LoadFile(Pack::PackFileRecord* pfr)
{
	return Pack::Read(pfr);
}

