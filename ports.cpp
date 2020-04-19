#include "NewSCI.h"

Port mainPort, screenPort, currentPort;
std::map<int, Font*> fonts;

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

void Port::SetFont(int fontNum)
{
	try
	{
		auto f = fonts.at(fontNum);
		SetFont(f);
	}
	catch (std::out_of_range) //Wasn't there. Try loading it.
	{
		char n[4];
		_itoa_s(fontNum, n, 4, 10);
		std::string fn = n;
		fn += ".fon";
		Font* f = Font::Load(fn);
		if (f)
		{
			fonts[fontNum] = f;
			SetFont(f);
		}
		else
			SetFont(sysFont);
	}
}
