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
};

struct ViewObj
{
	View* view;
	int left, top, loop, cel, pri;
	Rect lastSeenRect;
	Handle oldBits;

	void DrawCel();

public:
	static void Initialize();
	ViewObj(View* view);
	ViewObj(View* view, int left, int top);
	void UpdateLastSeenRect();
	void Draw();
	void Move(int x, int y);
	void Set(View* view, int loop, int cel);
	void SetLoop(int loop);
	int GetLoop();
	int GetNumLoops();
	void SetCel(int cel);
	int GetCel();
	int GetNumCels();
	void SetPri(int pri);
	int GetPri();
};