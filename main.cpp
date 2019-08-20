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

void DoAlert(const char* text)
{
	//Handle savedBits;
	Rect rect;
	currentPort.font->MeasureString(text, &rect, 160);
	rect.Inflate(4, 4);
	rect.Center();
	//savedBits = Bits::SaveBits(&rect);
	currentPort.SetPen(WHITE);
	FillRect(&rect);
	currentPort.SetPen(BLACK);
	DrawRect(&rect);
	rect.Inflate(-4, -4);
	//currentPort.font->RenderString(text, rect.l, rect.t);
	currentPort.font->Write(text, &rect, 0);

	SDL_Event ev;
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
	//Bits::RestoreBits(savedBits);
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
	}

	Pack::Load();

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

	//auto sizedTest = Load("ilira.view");
	//char* theData = sizedTest->data;
	//delete sizedTest;

	/* auto ilira = new View("ilira.view"); */
	//auto ælÉrà = new View("ælÉrà.view");

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
	ViewObj::Initialize();

	//Lua::RunScript("local win = openwindow({8, 18, 128, 40}, \"Test\", 0, true); message(\"bottom text\", \"top text\"); win:close()");
	//RunScript("message(\"Hello from Lua! Let's put some more text up in this bitch,\\nand a forced newline because why not?\", \"TOP TEXT\") message(\"You did it again, huh?\", \"Oops!\")");
	//Message("Hello from C++!", "TOP TEXT");
	//Lua::RunScript(R"(Message("Body text", "Top text");)");

	//Window myWindow(Rect(8,18,128,40), "Test", 1, true);
	//DoAlert("ælÉrà\n??n?");
	//DoAlert("line 1\nl?nê 2\nline three");
	//DoAlert("\1 Sierra");
	//DoAlert("Ay carumba! You did something we didn't expect.");
	//Message("You did something that we weren't expecting.\nWhatever it was, you don't need to do it to finish the game.\nTry taking a different approach to the situation.", "Oops!");

	SDL_SetCursor(CreateCursor("pointer.png", 0, 0));
	//Lua::RunFile("start.lua");
	Lua::RunFile("engine.lua");
	Lua::RunScript(R"(OpenScene("test.lua"))");

	/*
	SDL_Event ev;
	auto quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&ev) != 0)
		{
			switch (ev.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYUP:
				if (ev.key.keysym.mod & KMOD_CTRL)
				{
					if (ev.key.keysym.sym == SDLK_p)
						shownBuffer = priorityBuffer;
					else if (ev.key.keysym.sym == SDLK_v)
						shownBuffer = screenBuffer;
				}
				break;
			}
		}

		ShowFrame();

		iliraObj.Move(i, 110); //32);
		auto c = iliraObj.GetCel() + 1;
		if (c >= iliraObj.GetNumCels())
			c = 0;
		iliraObj.SetCel(c);
		i += 4;
		if (i >= 340)
			i = -20;

		SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, 320 * sizeof(Pixel));
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
		SDL_RenderPresent(sdlRenderer);

		SDL_Delay(60);
	}
	*/

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
