#include "NewSCI.h"

void DrawWindowTextControl(sol::table controlDef, int leftOffset = 0, int topOffset = 0, int maxWidth = 0)
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
}

void DrawWindow(sol::table windowDef)
{
	auto visible = windowDef["visible"].get<bool>();
	if (!visible)
		return;
	auto r = Rect(windowDef["box"][1].get<int>(), windowDef["box"][2].get<int>(), windowDef["box"][3].get<int>(), windowDef["box"][4].get<int>());
	auto title = windowDef["title"].get<std::string>();
	auto shadow = windowDef["shadow"].get<bool>();

	currentPort.SetPen(BLACK);
	if (shadow)
	{
		r.Offset(1, 1);
		FillRect(&r);
		r.Offset(-1, -1);
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
		topOffset += 10;
	}

	//Now draw the controls.
	auto controls = windowDef["controls"].get<sol::table>();
	for(auto control : controls)
	{
		auto c = control.second.as<sol::table>();
		auto type = c["type"].get<int>();
		switch (type)
		{
			case 1: DrawWindowTextControl(c, leftOffset, topOffset, width); break;
			default: SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unsupported window control type %d.", type); break;
		}
	}
}
