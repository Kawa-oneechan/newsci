#include "NewSCI.h"
#include "memory.h"

SDL_Window* sdlWindow = NULL;
SDL_Texture* sdlTexture = NULL;
SDL_Renderer* sdlRenderer = NULL;

int windowWidth = 640, windowHeight = 400, cursorMode;

Pixels shownBuffer;

extern Pixels visualBackground, priorityBackground;
extern bool soundEnabled;
extern char scan2ascii[];

extern void OpenGL_Present();

void Flush()
{
	SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
	//SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, screenPitch);
	//SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	//SDL_RenderPresent(sdlRenderer);
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

SDL_Cursor* CreateCursor(const char* filename, int x, int y)
{
	/*
	unsigned long size, width, height;
	Handle png = LoadFromFile(filename, &size);
	std::vector<unsigned char> image;
	decodePNG(image, width, height, (const unsigned char*)png, size);
	free(png);
	*/
	auto image = new Image(filename);

	SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(image->pixels, image->width, image->height, 32, 4 * image->width, SDL_PIXELFORMAT_ABGR8888);

	bool doubleUp = cursorMode > 0;
	if (cursorMode == 2)
		doubleUp = (screenWidth < windowWidth);

	if (doubleUp)
	{
		SDL_Surface* newSurf = SDL_CreateRGBSurfaceWithFormat(0, image->width * 2, image->height * 2, 32, SDL_PIXELFORMAT_ABGR8888);
		SDL_UpperBlitScaled(surf, NULL, newSurf, NULL);
		SDL_FreeSurface(surf);
		surf = newSurf;
	}

	SDL_Cursor* cur = SDL_CreateColorCursor(surf, x, y);
	return cur;
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
			if (root["video"]->HasChild("sound"))
			{
				auto sound = root["sound"]->AsObject();
				soundEnabled = sound["enabled"]->AsBool();
			}
			if (root["video"]->HasChild("input"))
			{
				auto input = root["input"]->AsObject();
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

	Lua::RunFile("engine.lua");
	Lua::RunScript(R"(OpenScene("test.lua"))");

 	free(visualBuffer);
	free(priorityBuffer);
	free(visualBackground);
	free(priorityBackground);
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
	return 0;
}
