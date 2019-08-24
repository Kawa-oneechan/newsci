//FONTS.H
//-------
//Decoding 
#pragma once
#include "types.h"

namespace UTF8
{
	//Returns the length of a string in characters. That is,"Renáta" is 7 bytes in UTF-8 (C3 A1), but 6 in Win-1252 (E1).
	int Length(const char* str);
	
	//Given a string with at least one full UTF-8 character, returns the codepoint value of the first character and advances the string.
	int DecodeChar(char** text, int* numBytes);
	int DecodeChar(char** text);
	
	//Decodes a string from UTF-8 to single byte values.
	//Any codepoint above 255 is replaced. If ascii is set to true, any codepoint above 127 is eaten instead.
	//Thus, this will turn a UTF-8 string into a Win-1252 or ASCII-7 string.
	char* Decode(char* dest, char* source, int charCount);

	//Decodes a string from UTF-8 to single byte values.
	//Any codepoint above 255 is replaced.
	//Thus, this will turn a UTF-8 string into a Win-1252 string.
	char* Decode(char* dest, char* source);

	//Decodes a string from UTF-8 to single byte values.
	//Any codepoint above 255 is replaced. If ascii is set to true, any codepoint above 127 is eaten instead.
	//Thus, this will turn a UTF-8 string into a Win-1252 or ASCII-7 string.	
	char* Decode(char* text, int charCount);

	//Decodes a string from UTF-8 to single byte values.
	//Any codepoint above 255 is replaced.
	//Thus, this will turn a UTF-8 string into a Win-1252 string.
	char* Decode(char* text);
}

//Base class. Don't use this, it won't render anything.
class Font
{
private:
	Uint16 lowChar, highChar;
	Uint16 lineHeight;
	Uint16* charData;
	int RenderCharacter(unsigned int c, int x, int y);
public:
	static Font* Load(std::string filename);
	Font();
	Font(std::string filename);
	virtual void RenderString(std::string text, int x, int y);
	virtual int MeasureCharacter(unsigned int ch);
	virtual void MeasureString(std::string text, Rect* rect, int maxWidth);
	void Font::Write(std::string text, Rect* rect, int mode);
	virtual int LineHeight();
	virtual int CharCount();
private:
	int GetLongest(Str* str, int max, bool* end);
	int TextWidth(Str str, int first, int count);
};

struct SCIFontChar
{
	Uint8 width, height;
	Uint8 bitmap[1];
};

class VGAFont : public Font
{
private:
	Uint16 lowChar, highChar;
	Uint16 lineHeight;
	Uint8* charData;
	int RenderCharacter(unsigned char c, int x, int y);
public:
	VGAFont();
	VGAFont(std::string filename) {};
	~VGAFont();
	void RenderString(std::string text, int x, int y);
	int MeasureCharacter(unsigned char ch);
	void MeasureString(std::string text, Rect* rect, int maxWidth);
	int LineHeight();
	int CharCount();
};

extern Font* sysFont;
extern Font* debugFont;
