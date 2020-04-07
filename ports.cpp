#include "NewSCI.h"
#include "support/fmt/format.h"

Port mainPort, currentPort;
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
		Font* f = Font::Load(fmt::format("{0}.fon", fontNum));
		if (f)
		{
			fonts[fontNum] = f;
			SetFont(f);
		}
		else
			SetFont(sysFont);
	}
}

Window::Window(Rect theFrame, std::string theTitle, int theType, int vis)
{
	frame = theFrame;
	type = theType;
	visible = 0;
	port.font = sysFont;
	if (!(type & 2))
	{
		frame.r++;
		frame.b++;
		if (theType & 1)
			frame.t -= 10;
	}
	if (!theTitle.empty())
		title = std::string(theTitle);
	if (vis)
		Draw();
}

void Window::Draw()
{
	//Port oldPort;
	Rect r;
	Font* oldFont = port.font;
	Color oldPen = port.fgColor;
	if (!visible)
	{
		visible = true;
		//oldPort = currentPort;
		//currentPort = windowManagerPort;
		currentPort.SetPen(BLACK);
		r = Rect(frame);

		if (!(type & 2))
		{
			r.r--;
			r.b--;
			r.Offset(1, 1);
			FillRect(&r);
			r.Offset(-1, -1);
		}

		DrawRect(&r);
		r.Inflate(-1, -1);
		currentPort.SetPen(port.bkColor);
		FillRect(&r);

		if (type & 1)
		{
			r.b = r.t + 10;
			currentPort.SetPen(DKGRAY);
			FillRect(&r);
			r.Inflate(-1, -1);
			currentPort.SetPen(WHITE);
			currentPort.SetFont(sysFont);
			currentPort.font->Write(title, &r, 1);
			//currentPort.font->RenderString(title, r.l + 2, r.t + 2);
			currentPort.SetFont(oldFont);
			currentPort.SetPen(oldPen);
		}
	}
}

void Window::Close()
{
	if (!visible)
		return;
	visible = false;
}
