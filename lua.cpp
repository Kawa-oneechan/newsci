#include "NewSCI.h"

extern void Message(std::string text, std::string title);
extern SDL_Window* sdlWindow;
extern SDL_Texture* sdlTexture;
extern SDL_Renderer* sdlRenderer;
extern Pixels shownBuffer, visualBackground, priorityBackground, visualBuffer, priorityBuffer;

sol::state Sol;

extern int windowWidth, windowHeight;

//To go from SDL scans to regular ol' BIOS scans
int keytranslation[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 14, 15, 0, 0, 0, 28, 0, 0, 0, 0, 0, 89, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 0, 57, 2, 40, 4, 5, 6, 8, 40, 10, 11, 9, 13, 51,
	12, 52, 53, 11, 2, 3, 4, 5, 6, 7, 8, 9, 10, 39, 39, 51, 13, 52, 53, 3, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	26, 43, 27, 7, 12, 41, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38, 50,
	49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44, 0, 0, 0, 0, 211, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 82, 79, 80, 81, 75, 76, 77, 71, 72, 73, 83, 181, 55, 74, 78, 156, 0,
	200, 208, 205, 203, 210, 199, 207, 201, 209, 59, 60, 61, 62, 63, 64, 65,
	66, 67, 68, 87, 88, 0, 0, 0, 0, 0, 0, 69, 58, 70, 54, 42, 157, 29, 184, 56,
	0, 0, 219, 220, 0, 0, 0, /*-2*/0, 84, 183, 221, 0, 0, 0
};

//To go from BIOS scans to characters.
char scan2ascii[] =
{
	0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
	'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
	'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
	'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
	'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void PrepareFrame()
{
//	memcpy(visualBuffer, visualBackground, screenSize * sizeof(Color));
//	memcpy(priorityBuffer, priorityBackground, screenSize * sizeof(Color));
	auto pt = currentPort.portRect.t;
	for (auto t = 0; t < screenHeight - pt; t++)
	{
		auto td = (pt + t) * screenWidth;
		auto ld = 0;
		auto ts = t * screenWidth;
		auto ls = 0;
		auto width = screenWidth;
		memcpy(visualBuffer + td + ld, visualBackground + ts + ls, width * sizeof(Pixel));
		memcpy(priorityBuffer + td + ld, priorityBackground + ts + ls, width * sizeof(Pixel));
	}
}

extern void OpenGL_Present();
extern void DrawCursor();

void ShowFrame()
{
	DrawCursor();
	OpenGL_Present();
}

extern int scale, offsetX, offsetY;

void ScaleMouse(signed int *x, signed int *y)
{
	signed int uX = *x, uY = *y;
	signed int nX = *x, nY = *y;

	nX = (nX - offsetX) / scale;
	nY = (nY - offsetY) / scale;

	if (nX < 0) nX = 0;
	if (nY < 0) nY = 0;
	if (nX >= screenWidth) nX = screenWidth - 1;
	if (nY >= screenHeight) nY = screenHeight - 1;

	*x = nX;
	*y = nY;
}

void LocalizeEvent(sol::table ev)
{
	auto type = ev["type"].get<int>();
	if (type < 1 || type > 3)
		return;
	auto x = ev["x"].get<int>();
	auto y = ev["y"].get<int>();
	ev["x"] = x - currentPort.portRect.l;
	ev["y"] = y - currentPort.portRect.t;
}

void HandleEvents()
{
	SDL_Event ev;
	auto solEvents = Sol["events"].get<sol::table>();
	auto thisEvent = solEvents[solEvents.size()];
	thisEvent = Sol.create_table();
	while (SDL_PollEvent(&ev) != 0)
	{
		switch (ev.type)
		{
		case SDL_QUIT:
			Sol["quit"] = true;
			break;
		case SDL_MOUSEMOTION:
			ScaleMouse(&ev.motion.x, &ev.motion.y);
			thisEvent["type"] = 1;
			thisEvent["x"] = ev.motion.x;
			thisEvent["y"] = ev.motion.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			ScaleMouse(&ev.motion.x, &ev.motion.y);
			thisEvent["type"] = (ev.type == SDL_MOUSEBUTTONDOWN) ? 2 : 3;
			thisEvent["button"] = ev.button.button;
			thisEvent["x"] = ev.button.x;
			thisEvent["y"] = ev.button.y;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			int code = keytranslation[ev.key.keysym.sym] + ((ev.key.keysym.mod & KMOD_SHIFT) ? 128 : 0);
			code = scan2ascii[code];
			thisEvent["type"] = (ev.type == SDL_KEYDOWN) ? 16 : 17;
			thisEvent["sym"] = SDL_GetKeyName(ev.key.keysym.sym);
			thisEvent["mod"] = ev.key.keysym.mod & ~KMOD_NUM;
			thisEvent["scan"] = ev.key.keysym.scancode;
			thisEvent["shift"] = (ev.key.keysym.mod & KMOD_SHIFT);
			thisEvent["ctrl"] = (ev.key.keysym.mod & KMOD_CTRL);
			thisEvent["alt"] = (ev.key.keysym.mod & KMOD_ALT);
			thisEvent["rawsym"] = ev.key.keysym.sym;
			thisEvent["char"] = code;
			break;
		}
	}
}

void Display(std::string text, int x, int y)
{
	Rect rect;
	currentPort.font->MeasureString(text, &rect, screenWidth - x);
	rect.Offset(x, y);
	currentPort.font->Write(text, &rect, 0);
}

void DrawStatus(std::string text)
{
	auto oldPort = currentPort;
	currentPort = screenPort;
	currentPort.fgColor = LTGRAY;
	auto r = Rect(0, 0, screenWidth, 10);
	FillRect(&r);
	currentPort.SetFont(0);
	currentPort.fgColor = BLACK;
	currentPort.font->RenderString(text, 0, 1);
	r = Rect(0, 9, screenWidth, 10);
	DrawRect(&r);
	currentPort = oldPort;
}

// This part shamelessly stolen from SCI11

#define TrigScale 10000L
static long ATanArray[] =
{
	875, 1763, 2679, 3640, 4663, 5774, 7002, 8391, 10000
};

static int ATanProrate(long index)
{
	int i = 0;
	while (ATanArray[i] < index) ++i;
	return ((5 * i) + (int)(((5L * (index - ATanArray[i - 1])) + ((ATanArray[i] - ATanArray[i - 1]) / 2)) / (ATanArray[i] - ATanArray[i - 1])));
}

static int ATanGetMajor(int x1, int y1, int x2, int y2)
{
	long deltaX, deltaY, index;
	int major;

	deltaX = (long)(abs(x2 - x1));
	deltaY = (long)(abs(y2 - y1));

	if ((deltaX == 0) && (deltaY == 0))
		return(0);

	if (deltaY <= deltaX)
	{
		index = ((TrigScale * deltaY) / deltaX);
		if (index < 1000L)
			major = (int)((57L * deltaY + (deltaX / 2L)) / deltaX);
		else
			major = ATanProrate(index);
	}
	else
		major = 90 - ATanGetMajor(y1, x1, y2, x2);

	return(major);
}

int ATan(int x1, int y1, int x2, int y2)
{
	int major;
	major = ATanGetMajor(x1, y1, x2, y2);
	if (x2 < x1)
	{
		if (y2 <= y1)
			major += 180;
		else
			major = 180 - major;
	}
	else
	{
		if (y2 < y1)
			major = 360 - major;
		if (major == 360)
			major = 0;
	}
	return(major);
}

void InitBresen(sol::table view)
{
	int dX, dY, toX, toY, distX, distY, incr, i1, i2, di;
	int tdX, tdY, watchDog;
	bool xAxis;

	auto movePoints = view["movePoints"].get<sol::table>();
	toX = movePoints[1].get<int>();
	toY = movePoints[2].get<int>();

	tdX = 2; /* view["stepSizeX"].get<int>(); */
	tdY = 1; /* view["stepSizeY"].get<int>(); */

	watchDog = (tdX > tdY) ? tdX : tdY;
	watchDog *= 2;

	// Get distances to be moved.
	distX = toX - view["x"].get<int>();
	distY = toY - view["y"].get<int>();

	// Compute basic step sizes
	while (1)
	{
		dX = tdX;
		dY = tdY;
		if (labs(distX) >= labs(distY))
		{
			// Motion will be along the X axis.
			xAxis = true;
			if (distX < 0)
				dX = -dX;
			dY = (distX) ? dX * distY / distX : 0;
		}
		else
		{
			// Major motion is along the Y axis.
			xAxis = false;
			if (distY < 0)
				dY = -dY;
			dX = (distY) ? dY * distX / distY : 0;
		}

		// Compute increments and decision variable.
		i1 = (xAxis) ? 2 * (dX * distY - dY * distX) : 2 * (dY * distX - dX * distY);
		incr = 1;
		if ((xAxis && distY < 0) || (!xAxis && distX < 0))
		{
			i1 = -i1;
			incr = -1;
		}

		i2 = i1 - 2 * ((xAxis) ? distX : distY);
		di = i1 - ((xAxis) ? distX : distY);

		if ((xAxis && distX < 0) || (!xAxis && distY < 0))
		{
			i1 = -i1;
			i2 = -i2;
			di = -di;
		}

		// Limit X step to avoid over-stepping Y step size
		if (xAxis && tdX > tdY)
		{
			if (tdX && labs(dY + incr) > tdY)
			{
				if (!(--watchDog))
					throw new std::string("InitBresen fail.");
				--tdX;
				continue;
			}
		}

		break;
	}

	view.set("bresenDX", dX);
	view.set("bresenDY", dY);
	view.set("bresenI1", i1);
	view.set("bresenI2", i2);
	view.set("bresenDI", di);
	view.set("bresenIn", incr);
	view.set("bresenXA", xAxis);
}

void DoBresen(sol::table view)
{
	int x, y, toX, toY, i1, i2, di, si1, si2, sdi;
	int dX, dY, incr;
	bool xAxis;
	int lastX, lastY;

	lastX = x = view["x"].get<int>();
	lastY = y = view["y"].get<int>();
	
	auto movePoints = view["movePoints"].get<sol::table>();
	toX = movePoints[1].get<int>();
	toY = movePoints[2].get<int>();

	xAxis = view["bresenXA"].get<bool>();
	dX = view["bresenDX"].get<int>();
	dY = view["bresenDY"].get<int>();
	incr = view["bresenIn"].get<int>();
	si1 = i1 = view["bresenI1"].get<int>();
	si2 = i2 = view["bresenI2"].get<int>();
	sdi = di = view["bresenDI"].get<int>();

	view.set("moveLastX", x);
	view.set("moveLastX", y);

	if ((xAxis && (labs(toX - x) <= labs(dX))) || (!xAxis && (labs(toY - y) <= labs(dY))))
	{
		// We're within a step size of the destination -- set client's x & y to it.
		x = toX;
		y = toY;
	}
	else
	{
		// Move one step.
		x += dX;
		y += dY;
		if (di < 0)
			di += i1;
		else
		{
			di += i2;
			if (xAxis)
				y += incr;
			else
				x += incr;
		}
	}

	view.set("x", x);
	view.set("y", y);

	if (0) // if(CantBeHere(view))
	{
		view.set("x", lastX);
		view.set("y", lastY);

		i1 = si1;
		i2 = si2;
		di = sdi;
		// Set blocked state
	}

	view.set("bresenI1", i1);
	view.set("bresenI2", i2);
	view.set("bresenDI", di);
}

// End stolen goods

extern sol::table DoGetPath(int16_t aX, int16_t aY, int16_t bX, int16_t bY, int16_t opt);

void Lua::Initialize()
{
	Sol.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);
	Sol.set_function("dofile", RunFile);
	Sol.set_function("print", printf);
	Sol.set_function("Message", Message);
	Sol.set_function("Delay", SDL_Delay);
	Sol.set_function("PrepareFrame", PrepareFrame);
	Sol.set_function("ShowFrame", ShowFrame);
	Sol.set_function("HandleEvents", HandleEvents);
	Sol.set_function("Display", Display);
	Sol.set_function("ShowScreen", [](int screenID) { shownBuffer = (screenID == 0) ? visualBuffer : priorityBuffer; });
	Sol.set_function("DrawWindow", DrawWindow);
	Sol.set_function("ATan", ATan);
	Sol.set_function("InitBresen", InitBresen);
	Sol.set_function("DoBresen", DoBresen);
	Sol.set_function("GetPath", DoGetPath);
	Sol.set_function("DrawStatus", DrawStatus);
	Sol.set_function("LocalizeEvent", LocalizeEvent);
}

void Lua::RunScript(std::string script)
{
	try
	{
		Sol.script(script);
	}
	catch (sol::error e)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "NewSCI", e.what(), NULL);
	}
}

void Lua::RunFile(std::string filename)
{
	unsigned long size = 0;
	char* data = LoadFile(filename, &size);
	RunScript(data);
}
