#pragma once

typedef char* Handle;
typedef char* Str;
typedef Uint32 Color;
typedef Uint32 Pixel;
typedef Uint32* Pixels;

#define KERNEL(X) void K##X (int* args)
#define ARG(N) *(args + (N))
#define ARGC arg(0)

struct SizedHandle
{
	unsigned long size;
	char data[1];

	operator char*() { return data; }
	char &operator[](int i) { return data[i]; }
};

struct Point
{
	int v, h;

public:
	Point();
	Point(int theH, int theV);
};

struct Rect
{
	int t, l, b, r;

public:
	Rect();
	Rect(int theL, int theT, int theR, int theB);
	void Inflate(int x, int y);
	void Center();
	void Offset(int x, int y);
};

