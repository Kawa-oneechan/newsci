#pragma once
#include "types.h"

extern Pixels visualBuffer, priorityBuffer;
extern int screenWidth, screenHeight;
extern int screenPitch, screenSize;

struct Image
{
	const char* filename;
	int width, height;
	int pitch, size;
	Pixels pixels;

public:
	Image(int width, int height);
	Image(const char* filename);
	~Image();
};

#define BLACK 0xFF000000
#define BLUE 0xFFAA0000
#define GREEN 0xFF00AA00
#define CYAN 0xFFAAAA00
#define RED 0xFF0000AA
#define MAGENTA 0xFFAA00AA
#define BROWN 0xFF0055AA
#define LTGRAY 0xFFAAAAAA
#define DKGRAY 0xFF555555
#define LTBLUE 0xFFFF5555
#define LTGREEN 0xFF55FF55
#define LTCYAN 0xFFFFFF55
#define LTRED 0xFF5555FF
#define LTMAGENTA 0xFFFF55FF
#define YELLOW 0xFF55FFFF
#define WHITE 0xFFFFFFFF
#define DKYELLOW 0xFF00AAAA
#define CORNFLOWER 0xFFED9564
#define ALPHA(C, A) (((A) << 24) | ((C) & 0x00FFFFFF))

#define DrawRect(R) { \
	for (int drY = (R)->t; drY <= (R)->b; drY++) { \
	SetPixel((R)->l, drY, currentPort.fgColor); \
	SetPixel((R)->r, drY, currentPort.fgColor); \
	for (int drX = (R)->l + 1; drX <= (R)->r - 1; drX++) { \
	SetPixel(drX, (R)->t, currentPort.fgColor); \
	SetPixel(drX, (R)->b, currentPort.fgColor); \
	} } }
#define FillRect(R) { \
	for (int drY = (R)->t; drY <= (R)->b; drY++) { \
	for (int drX = (R)->l; drX <= (R)->r; drX++) { \
	SetPixel(drX, drY, currentPort.fgColor); \
	} } }

inline void SetPixel(int X, int Y, Color C)
{
	if ((C) >> 24 == 0)
		return;
	const auto place = ((Y) * screenWidth) + (X);
	if ((C) >> 24 == 0xFF)
		visualBuffer[place] = (C);
	else 
	{
		auto now = visualBuffer[place];
		auto inR = ((C >> 0) & 0xFF);
		auto inG = ((C >> 8) & 0xFF);
		auto inB = ((C >> 16) & 0xFF);
		auto inA = ((C >> 24) & 0xFF);
		auto outR = ((now >> 0) & 0xFF);
		auto outG = ((now >> 8) & 0xFF);
		auto outB = ((now >> 16) & 0xFF);
		outR = ((inR * inA) + outR * (255 - inA)) >> 8;
		outG = ((inG * inA) + outG * (255 - inA)) >> 8;
		outB = ((inB * inA) + outB * (255 - inA)) >> 8;
		visualBuffer[place] = (outR) | (outG << 8) | (outB << 16);
	}
}
inline void SetPriPixel(int X, int Y, int C, int pri)
{
	auto priHere = (int)priorityBuffer[(Y * screenWidth) + X] & 0xFF;
	if (priHere > pri)
		return;
	SetPixel(X, Y, C);
	if ((C) >> 24 != 0)
		priorityBuffer[((Y) * screenWidth) + (X)] = pri;
}
