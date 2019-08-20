#include "NewSCI.h"

//Gee, who'da thunk that Asspull would come in handy?

#ifdef WIN32

//All this just for VirtualAlloc lol.
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
//#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <Windows.h>
//At least we get the rest of Kernel32 with it. AND NOTHING ELSE.

#else
//TODO: test dis.
#include <sys/mmap.h>
#include <errno.h>
#endif

#define ALIGN4(x) (((((x)-1)>>2)<<2)+4)

#define HEAPSTART 0x20000000
#define HEAPSIZE (1024 * 1024)
char* heap;// = (char*)0x03000000; //0;
char* heapStart; //char heapStart[HEAPSIZE];
char* heapEnd;

char* sbrk(int incr)
{
	if (incr == 0) return heap;
	char* prev_heap;
	if (heap == 0)
	{
		heap = (char*)heapStart; //(char*)&__HEAP_START;
	}
	prev_heap = heap;

	if (((unsigned int)heap + incr) >= (unsigned int)heapEnd)
		return prev_heap;

	heap += incr;
	return prev_heap; //(void*)ALIGN4((unsigned int)prev_heap);
}
char* brk(char* new_heap)
{
	char* prev_heap = heap;
	heap = new_heap;
	return prev_heap;
}

typedef struct malloc_block_meta
{
	int size;
	struct malloc_block_meta* next;
	int free;
} malloc_block_meta;

#define MALLOC_META_SIZE sizeof(struct malloc_block_meta)

char* malloc_global_base = NULL;

malloc_block_meta* malloc_find_free_block(struct malloc_block_meta** last, int size)
{
	malloc_block_meta* current = (malloc_block_meta*)malloc_global_base;
	while (current && !(current->free && current->size >= size))
	{
		*last = current;
		current = current->next;
	}
	return current;
}

struct malloc_block_meta* malloc_request_space(struct malloc_block_meta* last, int size)
{
	malloc_block_meta *block;
	block = (malloc_block_meta*)sbrk(0);
	void* request = sbrk(size + MALLOC_META_SIZE);
	//assert((void*)block == request); //Not thread safe.
	if (request == (void*)-1) return NULL; //sbrk failed.
	if (last) last->next = block; //NULL on first request.
	block->size = size;
	block->next = NULL;
	block->free = 0;
	return block;
}

malloc_block_meta* malloc_get_block_ptr(void* ptr)
{
	return (malloc_block_meta*)ptr - 1;
}


//And now the public stuff.

char* NeedPtr(unsigned int size)
{
	malloc_block_meta* block;
	size = ALIGN4(size);
	if (size <= 0) return NULL;
	if (malloc_global_base == 0) //First call.
	{
#ifdef WIN32
		heapStart = (char*)VirtualAlloc((void*)HEAPSTART, HEAPSIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (heapStart == 0)
		{
			auto err = GetLastError();
#else
		heapStart = (char*)mmap((void*)HEAPSTART, HEAPSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0, 0);
		if (heapStart == -1)
		{
			auto err = 0;
#endif
			SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Could not allocate heap: 0x%X", err);
			//if (err == 0x1E7) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Could not allocate heap, invalid address.");
			//else SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Could not allocate heap for ", "NewSCI", MB_ICONERROR);
			exit(2);
		}
		heapEnd = heapStart + HEAPSIZE;

		malloc_global_base = (char*)heapStart;
		heap = malloc_global_base;
		block = malloc_request_space(NULL, size);
		if (!block) return NULL;
		malloc_global_base = (char*)block;
	}
	else
	{
		malloc_block_meta* last = (malloc_block_meta*)malloc_global_base;
		block = malloc_find_free_block(&last, size);
		if (!block) //Failed to find free block.
		{
			block = malloc_request_space(last, size);
			if (!block) return NULL;
		}
		else // Found free block
		{
			//TODO: consider splitting block here.
			block->free = 0;
		}
	}

	return (char*)(block+1);
}

void DisposePtr(void* ptr)
{
	if (!ptr) return;
	//TODO: consider merging blocks once splitting blocks is implemented.
	malloc_block_meta* block_ptr = malloc_get_block_ptr(ptr);
	//assert(block_ptr->free == 0);
	block_ptr->free = 1;
}

void* ReallocatePtr(void* ptr, int size)
{
	if (!ptr) //NULL ptr. realloc should act like malloc.
		return NeedPtr(size);

	malloc_block_meta* block_ptr = malloc_get_block_ptr(ptr);
	if (block_ptr->size >= size) //We have enough space. Could free some once we implement split.
		return ptr;

	// Need to really realloc. Malloc new space and free old space.
	// Then copy old data to new space.
	void* new_ptr;
	new_ptr = NeedPtr(size);
	if (!new_ptr) return NULL; //TODO: set errno on failure.

	memcpy(new_ptr, ptr, block_ptr->size);
	DisposePtr(ptr);
	return new_ptr;
}

/*
void* calloc(int nelem, int elsize)
{
	int size = nelem * elsize; //TODO: check for overflow.
	void* ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}
*/

char* GetString(char* str)
{
	auto len = strlen(str) + 16;
	auto r = NeedPtr(len);
	strcpy_s(r, len, str);
	return r;
}

void SaveHeap(char* filename)
{
	FILE* fd;
	unsigned long heapSize = heap - heapStart;
	fopen_s(&fd, filename, "wb");
	fwrite(&heapStart, sizeof(unsigned long), 1, fd);
	fwrite(&heapSize, sizeof(unsigned long), 1, fd);
	fwrite(&heap, sizeof(unsigned long), 1, fd);
	fwrite(heapStart, heapSize, 1, fd);
	fclose(fd);
}

void LoadHeap(char* filename)
{
	FILE* fd;
	unsigned long apparentStart;
	unsigned long apparentSize;
	unsigned long apparentSbrk;
	fopen_s(&fd, filename, "rb");
	fread(&apparentStart, sizeof(unsigned long), 1, fd);
	fread(&apparentSize, sizeof(unsigned long), 1, fd);
	fread(&apparentSbrk, sizeof(unsigned long), 1, fd);
	if (apparentStart != (unsigned long)heapStart)
	{
		printf("Can't load \"%s\": heap location is wrong -- should be $%x, but is $%x?", filename, (unsigned long)heapStart, apparentStart);
		return;
	}
	if (apparentSize > HEAPSIZE)
	{
		printf("Can't load \"%s\": heap size is wrong -- should be $%x or lower, but is $%x?", filename, HEAPSIZE, apparentSize);
		return;
	}
	if (apparentSbrk > apparentSize - apparentStart)
	{
		printf("Can't load \"%s\": heap state is wrong -- should be $%x, but is $%x?", filename, apparentSize, apparentSbrk);
		return;
	}
	fread(heapStart, apparentSize, 1, fd);
	heap = (char*)apparentSbrk;
	fclose(fd);
}
