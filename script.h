#pragma once

typedef struct
{
	//Node link;		//list header (list node key is script number)
	//HeapRes* heap;	//pointer to data in heap for script
	//Handle hunk;	//handle pointer to hunkptr for data in hunk for script
	//uword* vars;	//pointer to variables for this script
	int clones;		//number of clones spawned from this script
} Script;
