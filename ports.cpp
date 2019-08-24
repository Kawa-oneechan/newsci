#include "NewSCI.h"

Port mainPort, currentPort;

Port::Port()
{
	if (sysFont)
	{
		font = sysFont;
		fontSize = font->LineHeight();
	}
	fgColor = BLACK;
	bkColor = WHITE;
	portRect = Rect(0, 0, screenWidth - 1, screenHeight - 1);
}

void Port::SetPen(Color color)
{
	fgColor = color;
}

void Port::SetFont(Font* fontHnd)
{
	font = fontHnd;
	fontSize = font->LineHeight();
}

Window::Window(Rect theFrame, std::string theTitle, int theType, int vis)
{
	frame = theFrame;
	type = theType;
	visible = 0;
	port.font = sysFont;
	if (!(type & 2))
	{
		frame.r++;
		frame.b++;
		if (theType & 1)
			frame.t -= 10;
	}
	if (!theTitle.empty())
		title = std::string(theTitle);
	if (vis)
		Draw();
}

void Window::Draw()
{
	//Port oldPort;
	Rect r;
	Font* oldFont = port.font;
	Color oldPen = port.fgColor;
	if (!visible)
	{
		visible = true;
		//oldPort = currentPort;
		//currentPort = windowManagerPort;
		currentPort.SetPen(BLACK);
		r = Rect(frame);

		if (!(type & 2))
		{
			r.r--;
			r.b--;
			r.Offset(1, 1);
			FillRect(&r);
			r.Offset(-1, -1);
		}

		DrawRect(&r);
		r.Inflate(-1, -1);
		currentPort.SetPen(port.bkColor);
		FillRect(&r);

		if (type & 1)
		{
			r.b = r.t + 10;
			currentPort.SetPen(DKGRAY);
			FillRect(&r);
			r.Inflate(-1, -1);
			currentPort.SetPen(WHITE);
			currentPort.SetFont(sysFont);
			currentPort.font->Write(title, &r, 1);
			//currentPort.font->RenderString(title, r.l + 2, r.t + 2);
			currentPort.SetFont(oldFont);
			currentPort.SetPen(oldPen);
		}
	}
}

void Window::Close()
{
	if (!visible)
		return;
	visible = false;
}

//TODO - rework in Sol
/*
namespace Windows
{
	int CloseWindow(lua_State* L)
	{
		lua_pushstring(L, "handle");
		lua_gettable(L, -2);
		Window* win = (Window*)lua_topointer(L, -1);
		win->Close();
		delete(win);
		return 0;
	}

	int KOpenWindow(lua_State* L)
	{
		int args = lua_gettop(L);

		lua_geti(L, 1, 1); //left
		lua_geti(L, 1, 2); //top
		lua_geti(L, 1, 3); //right
		lua_geti(L, 1, 4); //bottom
		int l = (int)lua_tointeger(L, -4);
		int t = (int)lua_tointeger(L, -3);
		int r = (int)lua_tointeger(L, -2);
		int b = (int)lua_tointeger(L, -1);

		const char* theTitle = lua_tostring(L, 2);
		int theType = (int)lua_tointeger(L, 3);
		int vis = lua_toboolean(L, 4);

		Rect rect(l, t, r, b);
		Window* win = new Window(rect, theTitle, theType, vis);

		lua_createtable(L, 0, 2);

		lua_pushstring(L, "handle");
		lua_pushlightuserdata(L, win);
		lua_settable(L, -3);

		lua_pushstring(L, "close");
		lua_pushcfunction(L, CloseWindow);
		lua_settable(L, -3);
		return 1;
	}

	void Bind(lua_State* L)
	{
		lua_register(L, "openwindow", KOpenWindow);
	}
}
*/
