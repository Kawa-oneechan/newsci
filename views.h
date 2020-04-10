#pragma once
#include "pictures.h"
#include "types.h"

typedef struct Cel
{
	int l, t, w, h, x, y;
} Cel;

typedef struct Loop
{
	bool mirror;
	int celCnt;
	Cel* cels;
} Loop;

struct View
{
	char* ID;
	Image* image;
	int loopCnt;
	Loop* loops;

public:
	static void Initialize();
	View(std::string filename);
	void Draw(int loop, int cel, int left, int top, int priority, bool noOffset = false, int tintR = 0, int tintG = 0, int tintB = 0, int tintA = 0);
	sol::table GetLastSeenRect(int loop, int cel, int left, int top);
	int GetNumLoops();
	int GetNumCels(int loop);
};
