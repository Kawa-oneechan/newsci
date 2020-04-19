#include "NewSCI.h"

extern int windowWidth, windowHeight;

extern void ScaleMouse(signed int *x, signed int *y);

void EatTheMouse()
{
	auto events = Sol["events"].get<sol::table>();
	for (auto ev : events)
	{
		auto evt = ev.second.as<sol::table>();
		if (evt["type"].get<int>() == 3) //mouse up
		{
			evt["handled"] = true;
			break;
		}
	}
}

bool DrawWindowTextControl(sol::table controlDef, int leftOffset = 0, int topOffset = 0, int maxWidth = 0)
{
	//auto visible = controlDef["visible"].get<bool>();
	//if (!visible)
	//	return;
	auto left = controlDef["left"].get_or(0) + leftOffset;
	auto top = controlDef["top"].get_or(0) + topOffset;
	auto width = controlDef["width"].get_or(0);
	if (width == 0) width = maxWidth;
	if (width == 0) width = screenWidth - left;
	auto r = Rect(left, top, left + width, screenHeight - top);
	auto text = controlDef["text"].get<std::string>();
	auto mode = controlDef["mode"].get_or(0);
	currentPort.SetFont(controlDef["font"].get_or(1));
	currentPort.SetPen(controlDef["color"].get_or(0) | 0xFF000000);
	currentPort.font->Write(text, &r, mode);
	return true;
}

bool DrawWindowButtonControl(sol::table controlDef, int leftOffset = 0, int topOffset = 0, int maxWidth = 0)
{
	//auto visible = controlDef["visible"].get<bool>();
	//if (!visible)
	//	return;
	auto left = controlDef["left"].get_or(0) + leftOffset;
	auto top = controlDef["top"].get_or(0) + topOffset;
	auto width = controlDef["width"].get_or(0);
	currentPort.SetFont(controlDef["font"].get_or(0));
	auto height = controlDef["height"].get_or(currentPort.fontSize + 4);
	if (width == 0) width = maxWidth;
	if (width == 0) width = screenWidth - left;
	auto r = Rect(left, top, left + width, top + height);
	auto text = controlDef["text"].get<std::string>();
	//TODO: recognize presence of function UserDraw and call that instead.
	currentPort.SetPen(controlDef["color"].get_or(0) | 0xFF000000);
	r.Inflate(-1, -3);
	currentPort.font->Write(text, &r, 1);
	r.Inflate(1, 3);
	currentPort.SetPen(controlDef["border"].get_or(0) | 0xFF000000);
	DrawRect(&r);
	
	auto mouseX = 0;
	auto mouseY = 0;
	auto mouseB = SDL_GetMouseState(&mouseX, &mouseY);
	ScaleMouse(&mouseX, &mouseY);
	if (mouseX >= r.l && mouseX <= r.r && mouseY >= r.t && mouseY <= r.b)
	{
		r.Inflate(-1, -1);
		DrawRect(&r);
		if (mouseB == 1)
		{
			controlDef["debounce"] = true;
			InvertRect(&r);
		}
		else if (controlDef["debounce"].get_or(false))
		{
			controlDef["debounce"] = false;
			EatTheMouse();
			return false;
		}
	}
	else if (mouseB == 1)
	{
		controlDef["debounce"] = false;
	}
	return true;
}

bool DrawWindowInputControl(sol::table controlDef, int leftOffset = 0, int topOffset = 0, int maxWidth = 0)
{
	auto left = controlDef["left"].get_or(0) + leftOffset;
	auto top = controlDef["top"].get_or(0) + topOffset;
	auto width = controlDef["width"].get_or(0);
	currentPort.SetFont(controlDef["font"].get_or(0));
	auto height = controlDef["height"].get_or(currentPort.fontSize + 4);
	if (width == 0) width = maxWidth;
	if (width == 0) width = screenWidth - left;
	auto r = Rect(left, top, left + width, top + height);
	auto text = controlDef["text"].get<std::string>();
	auto caretPos = controlDef["caret"].get<unsigned int>();

	auto mouseX = 0;
	auto mouseY = 0;
	auto mouseB = SDL_GetMouseState(&mouseX, &mouseY);
	ScaleMouse(&mouseX, &mouseY);
	auto inside = (mouseX >= r.l && mouseX <= r.r && mouseY >= r.t && mouseY <= r.b);

	currentPort.SetPen(controlDef["color"].get_or(0) | 0xFF000000);
	r.Inflate(-1, -3);
	auto oldRect = currentPort.portRect;
	currentPort.portRect = r;
	auto textRect = Rect(0, 0, width, height);
	currentPort.font->Write(text, &textRect, 0);
	currentPort.portRect = oldRect;
	r.Inflate(1, 3);
	currentPort.SetPen(inside ? (controlDef["border"].get_or(0) | 0xFF000000) : LTGRAY);
	DrawRect(&r);

	if (inside)
	{
		auto upToCaret = text.substr(0, caretPos);
		Rect caretRect;
		currentPort.font->MeasureString(upToCaret, &caretRect, width);
		caretRect.Offset(left, top);
		caretRect.l = caretRect.r + 1;
		caretRect.r = caretRect.l + 1;
		caretRect.t++;
		caretRect.b = caretRect.t + height - 2;
		InvertRect(&caretRect);

		auto events = Sol["events"].get<sol::table>();
		for (auto ev : events)
		{
			auto evt = ev.second.as<sol::table>();

			if (evt["type"].get<int>() == 3) //mouse up
			{
				auto x = evt["x"].get<int>() - left;
				auto y = evt["y"].get<int>() - top;
				auto newCaretPos = 0;
				caretRect = { 0, 0, 0, 0 };
				currentPort.font->MeasureString(text, &caretRect, width);
				if (x > caretRect.r)
					newCaretPos = text.length() + 1;
				else
				{
					for (auto i = 0u; i < text.length(); i++)
					{
						upToCaret = text.substr(0, i);
						caretRect = { 0, 0, 0, 0 };
						currentPort.font->MeasureString(upToCaret, &caretRect, width);
						if (x >= caretRect.r)
							newCaretPos = i;
						else
							break;
					}
				}
				controlDef.set("caret", newCaretPos);
				evt["handled"] = true;
				continue;
			}

			if (evt["type"].get<int>() != 17) //anything but keyup
				continue;
			auto scan = (SDL_Scancode)evt["scan"].get<int>();
			auto sym = evt["sym"].get<std::string>();
			auto rawsym = (SDL_Keycode)evt["rawsym"].get<int>();
			auto c = evt["char"].get<int>();
			if (rawsym == SDLK_LEFT && caretPos > 0)
			{
				caretPos--;
				controlDef.set("caret", caretPos);
			}
			else if (rawsym == SDLK_RIGHT && caretPos < text.length())
			{
				caretPos++;
				controlDef.set("caret", caretPos);
			}
			else if (rawsym == SDLK_BACKSPACE && caretPos > 0 && text.length() > 0)
			{
				caretPos--;
				text = text.substr(0, caretPos);
				controlDef.set("caret", caretPos);
				controlDef.set("text", text);
			}
			else if (rawsym == SDLK_DELETE && caretPos < text.length() && text.length() > 0)
			{
				text = text.substr(0, caretPos) + text.substr(caretPos + 1, text.length() - caretPos - 1);
				controlDef.set("text", text);
			}
			else if (c > 0 && c < 256)
			{
				/* if (event->key.keysym.scancode != SDL_GetScancodeFromKey(event->key.keysym.sym))
					printf("Physical %s key acting as %s key",
						SDL_GetScancodeName(event->key.keysym.scancode),
						SDL_GetKeyName(event->key.keysym.sym)); */
				text = text.substr(0, caretPos) + (char)c + text.substr(caretPos, text.length() - caretPos);
				caretPos++;
				controlDef.set("caret", caretPos);
				controlDef.set("text", text);
			}
			evt["handled"] = true;
			break;
		}
	}

	return true;
}

void DrawWindow(sol::table windowDef)
{
	if (!windowDef["visible"].get<bool>())
		return;
	auto r = Rect(windowDef["box"][1].get<int>(), windowDef["box"][2].get<int>(), windowDef["box"][3].get<int>(), windowDef["box"][4].get<int>());
	auto title = windowDef["title"].get<std::string>();
	auto shadow = windowDef["shadow"].get<bool>();
	auto in = Rect(r);

	auto oldPort = currentPort;
	currentPort = screenPort;

	currentPort.SetPen(BLACK);
	if (shadow)
	{
		r.Offset(1, 1);
		FillRect(&r);
		r.Offset(-1, -1);
		in.r++;
		in.b++;
	}
	DrawRect(&r);
	r.Inflate(-1, -1);
	currentPort.SetPen(WHITE);
	FillRect(&r);
	currentPort.SetPen(BLACK);

	auto leftOffset = r.l + 2;
	auto topOffset = r.t + 2;
	auto width = r.r - r.l - 4;

	if (title.length() > 0)
	{
		r.b = r.t + 10;
		currentPort.SetPen(DKGRAY);
		FillRect(&r);
		r.Inflate(-1, -1);
		currentPort.SetPen(WHITE);
		currentPort.SetFont(sysFont);
		currentPort.font->Write(title, &r, 1);
		currentPort.SetPen(BLACK);
		topOffset += 12;
	}

	auto mouseX = 0;
	auto mouseY = 0;
	auto mouseB = SDL_GetMouseState(&mouseX, &mouseY);
	ScaleMouse(&mouseX, &mouseY);
	auto inside = (mouseX >= in.l && mouseX <= in.r && mouseY >= in.t && mouseY <= in.b);

	//Now draw the controls.
	auto controls = windowDef["controls"].get<sol::table>();
	sol::table clicked;
	for(auto control : controls)
	{
		auto c = control.second.as<sol::table>();
		auto type = c["type"].get<int>();
		auto cont = true;
		switch (type)
		{
			case 1: cont = DrawWindowTextControl(c, leftOffset, topOffset, width); break;
			case 2: cont = DrawWindowButtonControl(c, leftOffset, topOffset, width); break;
			case 3: cont = DrawWindowInputControl(c, leftOffset, topOffset, width); break;
			default: SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unsupported window control type %d.", type); break;
		}
		if (!cont)
			clicked = c;
	}
	if (clicked)
	{
		auto maybeClick = clicked["Click"].get<sol::function>();
		if (maybeClick)
			maybeClick();
	}
	else if (inside)
	{
		EatTheMouse();
	}

	currentPort = oldPort;
}
