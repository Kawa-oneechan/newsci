#pragma once

#define HEAPSTART 0x10000000
//#define HEAPSIZE (1024 * 1024)
//#define HEAPSIZE 0xFFFF

extern char* NeedPtr(unsigned int size);
extern void DisposePtr(void* ptr);
extern void* ReallocatePtr(void* ptr, int size);
//extern char void* calloc(int nelem, int elsize);
extern char* GetString(char* str);
extern void SaveHeap(char* filename);
extern void LoadHeap(char* filename);
