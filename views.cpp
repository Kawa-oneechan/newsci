#include "NewSCI.h"
#include "support\sol.hpp"
#include "support/json/JSON.h"

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

void View::Initialize()
{
	Sol.new_usertype<View>(
		"View",
		sol::constructors<View(std::string)>()
		);
	Sol.set_function("LoadSimpleScene", &LoadSimpleScene);
}

//----------------------------------------------------------------//

ViewObj::ViewObj(View* view)
{
	this->view = view;
	this->loop = 0;
	this->cel = 0;
	this->pri = -1;
	this->left = 0;
	this->top = 0;
}

ViewObj::ViewObj(View* view, int left, int top) : ViewObj(view)
{
	this->view = view;
	this->loop = 0;
	this->cel = 0;
	this->pri = -1;
	this->left = left;
	this->top = top;
}


void ViewObj::UpdateLastSeenRect()
{
	auto image = view->image;
	auto pixels = image->pixels;
	if (loop > view->loopCnt)
		loop = view->loopCnt - 1;
	auto theLoop = view->loops[loop];
	if (cel > theLoop.celCnt)
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

	lastSeenRect.l = sx;
	lastSeenRect.t = sy;
	lastSeenRect.r = sx + w;
	lastSeenRect.b = sy + h;
}

void ViewObj::DrawCel()
{
	auto image = view->image;
	auto pixels = image->pixels;
	if (loop > view->loopCnt)
		loop = view->loopCnt - 1;
	auto theLoop = view->loops[loop];
	if (cel > theLoop.celCnt)
		cel = theLoop.celCnt - 1;
	auto theCel = theLoop.cels[cel];
	auto sx = theCel.l;
	auto sy = theCel.t;

	//get clipped
	auto w = theCel.w;
	auto h = theCel.h;
	auto l = left - (w / 2) + theCel.x;
	auto t = top - (h / 1) + theCel.y;
	if (l + w > screenWidth)
		w = screenWidth - l;
	if (t + h > screenHeight)
		h = screenHeight - t;
	if (l < 0)
	{
		sx += -l;
		w -= -l;
		l = 0;
	}
	if (t < 0)
	{
		sy += -t;
		h -= -t;
		t = 0;
	}

	for (auto y = 0; y < h; y++)
	{
		for (auto x = 0; x < w; x++)
		{
			auto tx = theLoop.mirror ? left + theCel.w - x : left + x;
			auto ty = t + y;
			auto pixel = pixels[((sy + y) * image->width) + (sx + x)];
			SetPriPixel(tx, ty, pixel, pri);
		}
	}
}

void ViewObj::Draw()
{
	if (oldBits != NULL)
	{
		//	Bits::RestoreBits(oldBits);
	}
	UpdateLastSeenRect();
	//oldBits = Bits::SaveBits(&lastSeenRect);

	int origPri = pri;
	if (pri == -1)
		pri = lastSeenRect.b;

	DrawCel(); //(view, loop, cel, sx, sy);

	pri = origPri;
}

void ViewObj::Move(int x, int y)
{
	left = x;
	top = y;
}

void ViewObj::SetLoop(int loop)
{
	this->loop = loop;
}

int ViewObj::GetLoop()
{
	return loop;
}

int ViewObj::GetNumLoops()
{
	return view->loopCnt;
}

void ViewObj::SetCel(int cel)
{
	this->cel = cel;
}

int ViewObj::GetCel()
{
	return cel;
}

int ViewObj::GetNumCels()
{
	return view->loops[loop].celCnt;
}

void ViewObj::SetPri(int pri)
{
	if (pri > 255)
		pri = 255;
	if (pri < 0)
		pri = -1;
	this->pri = pri;
}

int ViewObj::GetPri()
{
	return pri;
}

void ViewObj::Initialize()
{
	Sol.new_usertype<ViewObj>(
		"ViewObj",
		sol::constructors<ViewObj(View*), ViewObj(View*, int, int)>(),
		"Move", &Move,
		"loop", sol::property(&GetLoop, &SetLoop),
		"numLoops", sol::property(&GetNumLoops),
		"cel", sol::property(&GetCel, &SetCel),
		"numCels", sol::property(&GetNumCels),
		"priority", sol::property(&GetPri, &SetPri)
		);
}
