#include "NewSCI.h"

View::View(std::string filename)
{
	unsigned long size = 0;
	char* data = LoadFile(filename, &size);

	auto json = JSON::Parse(data);

#if WITHSTRINGIFY
	auto stringified = JSON::Stringify(json);
	FILE* fd;
	fopen_s(&fd, "lol.txt", "wb");
	fputs(stringified.c_str(), fd);
	fclose(fd);
#endif

	auto root = json->AsObject();
	this->image = new Image(root["image"]->AsString().c_str());

	auto jLoops = root["loops"]->AsArray();
	this->loopCnt = jLoops.size();
	this->loops = (Loop*)malloc(sizeof(Loop) * this->loopCnt);
	for (auto loop = 0; loop < this->loopCnt; loop++)
	{
		auto thisLoop = &this->loops[loop];
		auto jLoop = jLoops[loop]->AsArray();
		if (jLoop[0]->IsNumber())
		{
			thisLoop->mirror = true;
			jLoop = jLoops[(int)jLoop[0]->AsNumber()]->AsArray();
		}
		else
		{
			thisLoop->mirror = false;
		}
		thisLoop->celCnt = jLoop.size();
		thisLoop->cels = (Cel*)malloc(sizeof(Cel) * thisLoop->celCnt);
		for (auto cel = 0; cel < thisLoop->celCnt; cel++)
		{
			auto jCel = jLoop[cel]->AsArray();
			auto thisCel = &thisLoop->cels[cel];
			thisCel->l = (int)jCel[0]->AsNumber();
			thisCel->t = (int)jCel[1]->AsNumber();
			thisCel->w = (int)jCel[2]->AsNumber();
			thisCel->h = (int)jCel[3]->AsNumber();
			thisCel->x = (int)jCel[4]->AsNumber();
			thisCel->y = (int)jCel[5]->AsNumber();
		}
	}

	this->ID = (char*)_strdup(filename.c_str());
}

extern Pixels visualBackground, priorityBackground;
void LoadSimpleScene(std::string vis, std::string pri)
{
	auto backdrop = new Image(vis.c_str());
	memcpy(visualBackground, backdrop->pixels, min(backdrop->size, screenSize) * sizeof(Color));
	delete backdrop;
	backdrop = new Image(pri.c_str());
	memcpy(priorityBackground, backdrop->pixels, min(backdrop->size, screenSize) * sizeof(Color));
	delete backdrop;
}

void LoadScene(std::string filename)
{
	char* data = LoadFile(filename, nullptr);
	auto json = JSON::Parse(data);
	auto root = json->AsObject();
	auto vis = root["visual"]->AsString();
	auto pri = root["priority"]->AsString();
	LoadSimpleScene(vis, pri);
	auto jsonPolys = root["walkPolys"]->AsArray();
	auto solPolys = Sol["polygons"];
	for (auto i = 0u; i < jsonPolys.size(); i++)
	{
		auto thisJP = jsonPolys[i]->AsArray();
		auto excluding = thisJP[0]->AsBool();
		auto thisSP = solPolys[i + 1];
		thisSP = Sol.create_table();
		thisSP[1] = excluding;
		for (auto j = 1u, n = 0u; j < thisJP.size(); j += 2, n++)
		{
			auto x = (int)thisJP[j + 0]->AsNumber();
			auto y = (int)thisJP[j + 1]->AsNumber();
			SDL_Log("Poly %d, point %d: %dx%d", i, n, x, y);
			thisSP[j + 1] = x;
			thisSP[j + 2] = y;
		}
	}
}

void DrawLine(int x1, int y1, int x2, int y2, int color)
{
	auto dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	auto dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	auto err = (dx > dy ? dx : -dy) / 2, e2 = 0;

	for (;;)
	{
		SetPixel(x1, y1, color);
		if (x1 == x2 && y1 == y2) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x1 += sx; }
		if (e2 <  dy) { err += dx; y1 += sy; }
	}
}

void DrawPolys()
{
	auto polys = Sol["polygons"].get<sol::table>();
	for (auto pkv : polys)
	{
		auto lastX = -1, lastY = -1;
		auto poly = pkv.second.as<sol::table>();
		auto excluding = poly[1].get<bool>();
		for (auto i = 2u, j = 0u; i < poly.size(); i += 2, j++)
		{
			auto x = poly[i + 0].get<int>();
			auto y = poly[i + 1].get<int>();
			if (j > 0)
				DrawLine(lastX, lastY, x, y, excluding ? RED : GREEN);
			lastX = x;
			lastY = y;
		}
		//Close it up
		DrawLine(lastX, lastY, poly[2].get<int>(), poly[3].get<int>(), excluding ? RED : GREEN);
	}
}

void View::Initialize()
{
	Sol.new_usertype<View>(
		"View",
		sol::constructors<View(std::string)>(),
		"Draw", &Draw,
		"GetNumLoops", &GetNumLoops,
		"GetNumCels", &GetNumCels
	);
	Sol.set_function("LoadSimpleScene", &LoadSimpleScene);
	Sol.set_function("LoadScene", &LoadScene);
	Sol.set_function("DrawLine", DrawLine);
	Sol.set_function("DrawPolys", DrawPolys);
}

void View::Draw(int loop, int cel, int left, int top, int priority, int scaleX, int scaleY, bool noOffset)
{
	auto image = this->image;
	auto pixels = image->pixels;
	if (loop >= this->loopCnt)
		loop = this->loopCnt - 1;
	auto theLoop = this->loops[loop];
	if (cel >= theLoop.celCnt)
		cel = theLoop.celCnt - 1;
	auto theCel = theLoop.cels[cel];
	auto sx = theCel.l;
	auto sy = theCel.t;

	auto ox = theCel.x;
	auto oy = theCel.y;

	auto w = theCel.w;
	auto h = theCel.h;

	auto scalingX = new uint16_t[screenWidth]();
	auto scalingY = new uint16_t[screenHeight]();
	auto scaledWidth = (w * scaleX) >> 7;
	auto scaledHeight = (h * scaleY) >> 7;
	if (scaledWidth < 0) scaledWidth = 0;
	if (scaledHeight < 0) scaledHeight = 0;
	if (scaledWidth > screenWidth) scaledWidth = screenWidth;
	if (scaledHeight > screenHeight) scaledHeight = screenHeight;
	auto scaledOX = (ox * scaleX) >> 7;
	auto scaledOY = (oy * scaleY) >> 7;

	auto l = left - (scaledWidth / 2) + scaledOX;
	auto t = top - scaledHeight;

	int pixelNo = 0;
	int scaledPixel = 0;
	int scaledPixelNo = 0;
	int prevScaledPixelNo = 0;
	while (pixelNo < h)
	{
		scaledPixelNo = scaledPixel >> 7;
		for (; prevScaledPixelNo <= scaledPixelNo; prevScaledPixelNo++)
			scalingY[prevScaledPixelNo] = pixelNo;
		pixelNo++;
		scaledPixel += scaleY;
	}
	pixelNo--;
	scaledPixelNo++;
	for (; scaledPixelNo < scaledHeight; scaledPixelNo++)
		scalingY[scaledPixelNo] = pixelNo;
	pixelNo = 0;
	scaledPixel = scaledPixelNo = prevScaledPixelNo = 0;
	while (pixelNo < w)
	{
		scaledPixelNo = scaledPixel >> 7;
		for (; prevScaledPixelNo <= scaledPixelNo; prevScaledPixelNo++)
			scalingX[prevScaledPixelNo] = pixelNo;
		pixelNo++;
		scaledPixel += scaleX;
	}
	pixelNo--;
	scaledPixelNo++;
	for (; scaledPixelNo < scaledWidth; scaledPixelNo++)
		scalingX[scaledPixelNo] = pixelNo;

	if (!theLoop.mirror)
	{
		for (auto y = 0; y < scaledHeight; y++)
		{
			for (auto x = 0; x < scaledWidth; x++)
			{
				auto tx = l + x;
				auto ty = t + y;
				auto pixel = pixels[((sy + scalingY[y]) * image->width) + sx + scalingX[x]];
				SetPriPixel(tx, ty, pixel, priority);
			}
		}
	}
	else
	{
		for (auto y = 0; y < scaledHeight; y++)
		{
			for (auto x1 = scaledWidth - 1, x2 = 0; x1 >= 0; --x1, x2++)
			{
				auto tx = l + x2;
				auto ty = t + y;
				auto pixel = pixels[((sy + scalingY[y]) * image->width) + sx + scalingX[x1]];
				SetPriPixel(tx, ty, pixel, priority);
			}
		}
	}
}

sol::table View::GetLastSeenRect(int loop, int cel, int left, int top)
{
	auto image = this->image;
	auto pixels = image->pixels;
	if (loop >= this->loopCnt)
		loop = this->loopCnt - 1;
	auto theLoop = this->loops[loop];
	if (cel >= theLoop.celCnt)
		cel = theLoop.celCnt - 1;
	auto theCel = theLoop.cels[cel];

	auto w = theCel.w;
	auto h = theCel.h;
	auto sx = left - (w / 2) + theCel.x;
	auto sy = top - (h / 1) + theCel.y;

	//get clipped
	if (left + w > screenWidth)
		w = screenWidth - left;
	if (top + h > screenHeight)
		h = screenHeight - top;
	if (left < 0)
	{
		sx += -left;
		w -= -left;
		//left = 0;
	}
	if (top < 0)
	{
		sy += -top;
		h -= -top;
		//top = 0;
	}

	auto ret = Sol.create_table_with(sx, sy, sx + w, sy + h);
	return ret;
}

int View::GetNumLoops()
{
	return this->loopCnt;
}
int View::GetNumCels(int loop)
{
	if (loop >= this->loopCnt)
		loop = this->loopCnt - 1;
	auto theLoop = this->loops[loop];
	return theLoop.celCnt;
}
