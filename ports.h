#pragma once
#include "types.h"
#include "fonts.h"

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
};

struct Window
{
	Port port;
	Rect frame;
	Rect saveRect;
	int type;
	Handle underBits;
	std::string title;
	int visible;

public:
	Window(Rect theFrame, std::string theTitle, int theType, int vis);
	void Draw();
	void Close();
	static void Bind(lua_State* L);
};

namespace Windows
{
	void Bind(lua_State* L);
}

extern Port mainPort, currentPort;
