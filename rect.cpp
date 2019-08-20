#include "NewSCI.h"

Point::Point()
{
	v = h = 0;
}

Point::Point(int theH, int theV)
{
	v = theV;
	h = theH;
}

Rect::Rect()
{
	l = t = r = b = 0;
}

Rect::Rect(int theL, int theT, int theR, int theB)
{
	l = theL;
	t = theT;
	r = theR;
	b = theB;
}

void Rect::Inflate(int x, int y)
{
	t -= y;
	l -= x;
	b += y;
	r += x;
}

void Rect::Center()
{
	int w = r - l;
	int h = b - t;
	l = (screenWidth / 2) - (w / 2);
	t = (screenHeight / 2) - (h / 2);
	r = l + w;
	b = t + h;
}

void Rect::Offset(int x, int y)
{
	l += x;
	t += y;
	r += x;
	b += y;
}
