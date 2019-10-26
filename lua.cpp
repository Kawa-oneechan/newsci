#include "NewSCI.h"
#include "support/fmt/format.h"

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
//TODO: load from file.
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
	memcpy(visualBuffer, visualBackground, screenSize * sizeof(Color));
	memcpy(priorityBuffer, priorityBackground, screenSize * sizeof(Color));
}

extern void OpenGL_Present();

void ShowFrame()
{
//	SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, screenPitch);
//	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
//	SDL_RenderPresent(sdlRenderer);
	OpenGL_Present();
}

void HandleEvents()
{
	auto mouseDivX = windowWidth / screenWidth;
	auto mouseDivY = windowHeight / screenHeight;

	SDL_Event ev;
	while (SDL_PollEvent(&ev) != 0)
	{
		switch (ev.type)
		{
		case SDL_QUIT:
			Lua::RunScript("quit = true");
			break;
		case SDL_MOUSEMOTION:
			Lua::RunScript(fmt::format("table.insert(events, {{ type = 1, x = {}, y = {} }})", ev.motion.x / mouseDivX, ev.motion.y / mouseDivY));
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			Lua::RunScript(fmt::format("table.insert(events, {{ type = {}, button = {}, x = {}, y = {} }})", (ev.type == SDL_MOUSEBUTTONDOWN) ? 2 : 3,
				ev.button.button, ev.button.x / mouseDivX, ev.button.y / mouseDivY));
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			int code = keytranslation[ev.key.keysym.sym] + ((ev.key.keysym.mod & KMOD_SHIFT) ? 128 : 0);
			code = scan2ascii[code];
			Lua::RunScript(fmt::format("table.insert(events, {{ type = {}, sym = \"{}\", mod = {}, scan = {}, shift = {}, ctrl = {}, alt = {}, rawsym = {}, char = {} }})",
				(ev.type == SDL_KEYDOWN) ? 16 : 17,
				SDL_GetKeyName(ev.key.keysym.sym),
				ev.key.keysym.mod & ~KMOD_NUM,
				ev.key.keysym.scancode,
				(ev.key.keysym.mod & KMOD_SHIFT) ? "true" : "false",
				(ev.key.keysym.mod & KMOD_CTRL) ? "true" : "false",
				(ev.key.keysym.mod & KMOD_ALT) ? "true" : "false",
				ev.key.keysym.sym,
				code));
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

// End stolen goods


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
	//Sol.set_function("OldShowFrame", OldShowFrame);
	Sol.set_function("Display", Display);
	Sol.set_function("ShowScreen", [](int screenID) { shownBuffer = (screenID == 0) ? visualBuffer : priorityBuffer; });
	Sol.set_function("DrawWindow", DrawWindow);
	Sol.set_function("ATan", ATan);
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
