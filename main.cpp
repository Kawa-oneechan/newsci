#include "NewSCI.h"
#include "ini.h"
#include "memory.h"

SDL_Window* sdlWindow = NULL;
SDL_Texture* sdlTexture = NULL;
SDL_Renderer* sdlRenderer = NULL;

int windowWidth, windowHeight, cursorMode;

Pixels shownBuffer;
#ifdef SHADERS
Pixels windowBuffer;
#endif
extern Pixels visualBackground, priorityBackground;
extern bool soundEnabled;
extern char scan2ascii[];

/*
SizedHandle* SizedLoad(const char* file)
{
	FILE* fd;
	fopen_s(&fd, file, "rb");
	fseek(fd, 0, SEEK_END);
	auto len = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	SizedHandle* ret = (SizedHandle*)malloc(sizeof(SizedHandle) + len);
	ret->size = len;
	fread(&ret->data, sizeof(char), len, fd);
	fclose(fd);
	return ret;
}
*/

#ifdef SHADERS
void ApplyShader(Pixels sourceBuffer)
{
	for (int row = 0; row < screenHeight; row++)
	{
		for (int col = 0; col < screenWidth; col++)
		{
			auto source = (row * screenWidth) + col;
			auto target = ((row * 2) * windowWidth) + (col * 2);
			auto next = target + windowWidth;
			auto pixel = sourceBuffer[source];
			windowBuffer[target] = pixel;
			windowBuffer[target + 1] = pixel;
			auto r = (pixel & 0x00FF0000) >> 16;
			auto g = (pixel & 0x0000FF00) >> 8;
			auto b = (pixel & 0x000000FF) >> 0;
			pixel = 0xFF000000 | ((r / 2) << 16) | ((g / 2) << 8) | ((b / 2) << 0);
			windowBuffer[next] = pixel;
			windowBuffer[next + 1] = pixel;
		}
	}
}
#endif

void Flush()
{
	SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
#ifdef SHADERS
	ApplyShader(shownBuffer);
	SDL_UpdateTexture(sdlTexture, NULL, windowBuffer, windowWidth * sizeof(Color));
#else
	SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, screenPitch);
#endif
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
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

#ifdef _DEBUG
#include <tchar.h>
int _tmain(int argc, _TCHAR* argv[])
{
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
int main(int argc, char*argv[])
{
#endif
	std::string keymapFile;

	{
		auto ini = new IniFile();
		ini->Load("resource.ini");
		screenWidth = atoi(ini->Get("Video", "scrWidth", "320"));
		screenHeight = atoi(ini->Get("Video", "scrHeight", "200"));
		windowWidth = atoi(ini->Get("Video", "winWidth", "640"));
		windowHeight = atoi(ini->Get("Video", "winHeight", "480"));
		auto c = ini->Get("Video", "cursor", "auto");
		if (!_strcmpi(c, "single")) cursorMode = 0;
		else if (!_strcmpi(c, "double")) cursorMode = 1;
		else cursorMode = 2;
		c = ini->Get("Sound", "enabled", "true");
		soundEnabled = (!_strcmpi(c, "true"));
		c = ini->Get("Input", "keymap", "american");
		keymapFile = c;
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

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 0;
	if ((sdlWindow = SDL_CreateWindow("NewSCI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN)) == NULL)
	{
		SDL_Log("Could not create window: %s", SDL_GetError());
		return 0;
	}
	if ((sdlRenderer = SDL_CreateRenderer(sdlWindow, 0, SDL_RENDERER_ACCELERATED)) == NULL)
	{
		SDL_Log("Could not create renderer: %s", SDL_GetError());
		return 0;
	}
#ifdef SHADERS
	if ((sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight)) == NULL)
#else
	if ((sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight)) == NULL)
#endif
	{
		SDL_Log("Could not create texture: %s", SDL_GetError());
		return 0;
	}

	SDL_SetCursor(CreateCursor("hand.png", 0, 0)); //(screenWidth < windowWidth)));

	screenSize = screenWidth * screenHeight;
	screenPitch = screenWidth * sizeof(Color);
	visualBuffer = new Color[screenSize];
	priorityBuffer = new Color[screenSize];
#ifdef SHADERS
	windowBuffer = new Color[windowWidth * windowHeight];
#endif
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

	SDL_SetCursor(CreateCursor("pointer.png", 0, 0));
	Lua::RunFile("engine.lua");
	Lua::RunScript(R"(OpenScene("test.lua"))");

 	free(visualBuffer);
	free(priorityBuffer);
#ifdef SHADERS
	free(windowBuffer);
#endif
	free(visualBackground);
	free(priorityBackground);
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
	return 0;
}
