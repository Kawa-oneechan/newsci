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

	///<summary>Initializes the packfile system.</summary>
	extern void Load();
	///<summary>Locates the given asset in the packfile and returns information on it.</summary>
	extern PackFileRecord* Find(std::string filename);
	///<summary>Returns the contents of the given asset.</summary>
	extern char* Read(PackFileRecord* pfr);
	///<summary>Returns the contents of the given asset.</summary>
	extern char* Read(std::string filename);
}

///<summary>Returns the contents and optionally size of the given asset.</summary>
extern Handle LoadFile(std::string filename, unsigned long *size);
