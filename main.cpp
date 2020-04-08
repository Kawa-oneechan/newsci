#include "NewSCI.h"
#include "memory.h"

SDL_Window* sdlWindow = nullptr;
SDL_Texture* sdlTexture = nullptr;
SDL_Renderer* sdlRenderer = nullptr;

int windowWidth = 640, windowHeight = 400, cursorMode;

Image* cursorImg = nullptr;
int cursorHotX = 0, cursorHotY = 0;

Pixels shownBuffer;

extern Pixels visualBackground, priorityBackground;
extern bool soundEnabled;
extern char scan2ascii[];

extern void OpenGL_Present();
extern void ScaleMouse(signed int *x, signed int *y);

void DrawCursor()
{
	if (cursorImg == nullptr)
		return;
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	ScaleMouse(&mouseX, &mouseY);

	int sx = 0;
	int sy = 0;
	//get clipped
	auto w = cursorImg->width;
	auto h = cursorImg->height;
	auto l = mouseX - cursorHotX;
	auto t = mouseY - cursorHotY;
	if (l + w > screenWidth)
		w = screenWidth - l;
	if (t + h > screenHeight)
		h = screenHeight - t;
	if (l < 0)
	{
		sx += -l;
		w -= -l;
		l = 0;
	}
	if (t < 0)
	{
		sy += -t;
		h -= -t;
		t = 0;
	}

	for (auto y = 0; y < h; y++)
	{
		for (auto x = 0; x < w; x++)
		{
			auto tx = l + x;
			auto ty = t + y;
			auto pixel = cursorImg->pixels[((sy + y) * cursorImg->width) + (sx + x)];
			SetPixel(tx, ty, pixel);
		}
	}
}

void SetCursor(const char* filename, int x, int y)
{
	cursorImg = new Image(filename);
	cursorHotX = x;
	cursorHotY = y;
}

void Flush()
{
	SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
	DrawCursor();
	OpenGL_Present();
}

void Message(std::string text, std::string title)
{
	if (debugFont == NULL) debugFont = Font::Load("999.fon");
	Rect rect;
	sysFont->MeasureString(title, &rect, 200);
	int titleWidth = rect.r;
	debugFont->MeasureString(text, &rect, 200);
	if (rect.r < titleWidth)
		rect.r = titleWidth;
	rect.Center();
	rect.Inflate(4, 4);
	Window myWindow(rect, title, 1, true);
	myWindow.port.font = debugFont;
	rect.Inflate(-4, -4);
	myWindow.port.font->Write(text, &rect, 0);

	SDL_Event ev;
	Flush();
	while (true)
	{
		if (SDL_PollEvent(&ev) == 0)
			continue;
		if (ev.type == SDL_KEYUP)
		{
			if (ev.key.keysym.sym == SDLK_ESCAPE)
				break;
			else if (ev.key.keysym.sym == SDLK_RETURN)
				//ret = 1;
				break;
		}
		if (ev.type == SDL_MOUSEBUTTONUP)
			break;
		SDL_UpdateTexture(sdlTexture, NULL, visualBuffer, screenPitch);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
		SDL_RenderPresent(sdlRenderer);
	}

	myWindow.Close();
}

extern void OpenGL_Initialize();

JSONValue *settings = nullptr;

#ifdef _DEBUG
#include <tchar.h>
int _tmain(int argc, _TCHAR* argv[])
{
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
int main(int argc, char*argv[])
{
#endif
	std::string keymapFile = "american";

	char* data = LoadFile("resource.json", nullptr);
	if (data != nullptr)
	{
		settings = JSON::Parse(data);
		{
			auto root = settings->AsObject();
			if (settings->HasChild("video"))
			{
				auto video = root["video"]->AsObject();
				if (root["video"]->HasChild("screen"))
				{
					auto scrSize = video["screen"]->AsArray();
					screenWidth = (int)scrSize[0]->AsNumber();
					screenHeight = (int)scrSize[1]->AsNumber();
				}
				if (root["video"]->HasChild("window"))
				{
					if (video["window"]->IsArray())
					{
						auto winSize = video["window"]->AsArray();
						windowWidth = (int)winSize[0]->AsNumber();
						windowHeight = (int)winSize[1]->AsNumber();
					}
					else if (video["window"]->IsNumber())
					{
						auto scale = (int)video["window"]->AsNumber();
						windowWidth = screenWidth * scale;
						windowHeight = screenHeight * scale;
					}
				}
			}
			if (settings->HasChild("sound"))
			{
				auto sound = root["sound"]->AsObject();
				if (root["sound"]->HasChild("enabled"))
					soundEnabled = sound["enabled"]->AsBool();
			}
			if (settings->HasChild("input"))
			{
				auto input = root["input"]->AsObject();
				if (root["input"]->HasChild("keymap"))
					keymapFile = input["keymap"]->AsString();
			}
		}
	}

	Pack::Load();

	if (keymapFile != "american")
	{
		keymapFile += ".key";
		auto size = 0UL;
		auto ret = LoadFile(keymapFile.c_str(), &size);
		if (ret != NULL && size == 256)
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Using keymap %s.", keymapFile.c_str());
			memcpy(scan2ascii, ret, 256);
		}
		else
			SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Invalid keymap file.");
		free(ret);
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_VIDEO_OPENGL) < 0)
		return 0;
	if ((sdlWindow = SDL_CreateWindow("NewSCI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL)
	{
		SDL_Log("Could not create window: %s", SDL_GetError());
		return 0;
	}
	OpenGL_Initialize();
	if ((sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_TARGET, screenWidth, screenHeight)) == NULL)
	{
		SDL_Log("Could not create texture: %s", SDL_GetError());
		return 0;
	}

	SDL_ShowCursor(0);

	screenSize = screenWidth * screenHeight;
	screenPitch = screenWidth * sizeof(Color);
	visualBuffer = new Color[screenSize];
	priorityBuffer = new Color[screenSize];
	visualBackground = new Color[screenSize];
	priorityBackground = new Color[screenSize];
	memset(visualBackground, 0xFF000000, screenSize * sizeof(Color));
	memset(priorityBackground, 0xFF000000, screenSize * sizeof(Color));

	shownBuffer = visualBuffer;

	sysFont = Font::Load("0.fon");
	debugFont = Font::Load("999.fon");

	mainPort.portRect.t = 0;
	mainPort.portRect.l = 0;
	mainPort.portRect.b = screenHeight - 1;
	mainPort.portRect.r = screenWidth - 1;
	mainPort.SetFont(sysFont);
	currentPort = mainPort;

	Lua::Initialize();
	Serializer::Initialize();
	Audio::Initialize();
	View::Initialize();

	SetCursor("pointer.png", 0, 0);

	Lua::RunFile("engine.lua");
	Lua::RunScript(R"(OpenScene("test.lua"))");

	if (cursorImg)
		delete cursorImg;
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
	return 0;
}
