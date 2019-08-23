#include "NewSCI.h"
#include "support/fmt/format.h"

extern void Message(std::string text, std::string title);
extern SDL_Window* sdlWindow;
extern SDL_Texture* sdlTexture;
extern SDL_Renderer* sdlRenderer;
extern Pixels shownBuffer, visualBackground, priorityBackground, visualBuffer, priorityBuffer;

sol::state Sol;

extern int windowWidth, windowHeight;
#ifdef SHADERS
extern Pixels windowBuffer;
extern void ApplyShader(Pixels sourceBuffer);
#endif

void PrepareFrame()
{
	memcpy(visualBuffer, visualBackground, screenSize * sizeof(Color));
	memcpy(priorityBuffer, priorityBackground, screenSize * sizeof(Color));
}

void ShowFrame()
{
	#ifdef SHADERS
	ApplyShader(shownBuffer);
	SDL_UpdateTexture(sdlTexture, NULL, windowBuffer, windowWidth * sizeof(Color));
#else
	SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, screenPitch);
#endif
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
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
			Lua::RunScript(fmt::format("table.insert(events, {{ type = {}, sym = \"{}\", mod = {}, scan = {}, shift = {}, ctrl = {}, alt = {} }})", (ev.type == SDL_KEYDOWN) ? 16 : 17,
				SDL_GetKeyName(ev.key.keysym.sym),
				ev.key.keysym.mod & ~KMOD_NUM, ev.key.keysym.scancode,
				(ev.key.keysym.mod & KMOD_SHIFT) ? "true" : "false",
				(ev.key.keysym.mod & KMOD_CTRL) ? "true" : "false",
				(ev.key.keysym.mod & KMOD_ALT) ? "true" : "false"));
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

void Lua::Initialize()
{
	Sol.open_libraries(sol::lib::base, sol::lib::table);
	Sol.set_function("dofile", RunFile);
	Sol.set_function("Message", Message);
	Sol.set_function("Delay", SDL_Delay);
	Sol.set_function("PrepareFrame", PrepareFrame);
	Sol.set_function("ShowFrame", ShowFrame);
	Sol.set_function("HandleEvents", HandleEvents);
	//Sol.set_function("OldShowFrame", OldShowFrame);
	Sol.set_function("Display", Display);
	Sol.set_function("ShowScreen", [](int screenID) { shownBuffer = (screenID == 0) ? visualBuffer : priorityBuffer; });
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
