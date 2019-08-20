#pragma once

namespace Pack
{
	struct PackFileRecord
	{
		char filename[255];
		Uint32 offset, size;
	};

	/* DUMB PACK
	 * int32	magic	"dumb" in ASCII
	 * uint32	count	# of files
	 * 
	 * per file:
	 * uint8	len		filename length
	 * char+	filename
	 * uint32	offset
	 * uint32	length
	 */

	extern void Load();
	extern PackFileRecord* Find(std::string filename);
	extern char* Read(PackFileRecord* pfr);
	extern char* Read(std::string filename);
}

extern Handle LoadFile(std::string filename, unsigned long *size);