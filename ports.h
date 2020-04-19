#pragma once
#include "types.h"
#include "fonts.h"

//TODO: rename things to be less blatantly SCI, remove anything we don't actually need.

struct Port
{
	Point origin;
	Rect portRect;
	Point pnLoc;
	int fontSize;
	Font* font;
	int txFace;
	int fgColor;
	int bkColor;
	int pnMode;

public:
	Port();
	void SetPen(Color color);
	void SetFont(Font* fontHnd);
	void SetFont(int fontNum);
};

extern Port mainPort, screenPort, currentPort;
