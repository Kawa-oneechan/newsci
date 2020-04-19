//FONTS.H
//-------
//Decoding 
#pragma once
#include "types.h"

namespace UTF8
{
	///<summary>Returns the length of a string in characters.</summary>
	///<remarks>That is,"Renáta" is 7 bytes in UTF-8 (<c>C3 A1</c>), but 6 in Win-1252 (<c>E1</c>).</remarks>
	int Length(const char* str);
	
	///<summary>Given a string with at least one full UTF-8 character, returns the codepoint value of the first character and advances the string.</summary>
	int DecodeChar(char** text, int* numBytes);
	int DecodeChar(char** text);
	
	///<summary>Decodes a string from UTF-8 to single byte values.</summary>
	///<remarks>Any codepoint above <paramref name="charCount"> is replaced.
	///Thus, this will turn a UTF-8 string into a Win-1252 or ASCII-7 string.</remarks>
	char* Decode(char* dest, char* source, int charCount);

	///<summary>Decodes a string from UTF-8 to single byte values.</summary>
	///<remarks>Any codepoint above 255 is replaced.
	///Thus, this will turn a UTF-8 string into a Win-1252 string.</remarks>
	char* Decode(char* dest, char* source);

	///<summary>Decodes a string from UTF-8 to single byte values.</summary>
	///<remarks>Any codepoint above <paramref name="charCount"> is replaced.
	///Thus, this will turn a UTF-8 string into a Win-1252 or ASCII-7 string.</remarks>
	char* Decode(char* text, int charCount);

	///<summary>Decodes a string from UTF-8 to single byte values.</summary>
	///<remarks>Any codepoint above 255 is replaced.
	///Thus, this will turn a UTF-8 string into a Win-1252 string.</remarks>
	char* Decode(char* text);
}

///<summary>Base font class that can load SCI font resources.</summary>
class Font
{
private:
	Uint16 lowChar, highChar;
	Uint16 lineHeight;
	Uint16* charData;
	int RenderCharacter(unsigned int c, int x, int y);
public:
	///<summary>Load a front from the given filename.</summary>
	///<remarks>Will return a VGAFont as fallback if the file is not an SCI font.</remarks>
	static Font* Load(std::string filename);
	Font();
	Font(std::string filename);
	///<summary>Renders a string in this font to screen at the given coordinates.</summary>
	virtual void RenderString(std::string text, int x, int y);
	///<summary>Returns the width of the specified character in this font.</summary>
	virtual int MeasureCharacter(unsigned int ch);
	///<summary>Returns the size of the specified text in this font.</summary>
	virtual void MeasureString(std::string text, Rect* rect, int maxWidth);
	///<summary>Renders a string in this font to the screen inside the given bounding box.</summary>
	///<param name="mode">The alignment mode: 0 for left-justified, 1 for centered, and 2 to right-justified.</param>
	void Font::Write(std::string text, Rect* rect, int mode);
	///<summary>Returns the height of a single line in this font.</summary>
	virtual int LineHeight();
	///<summary>Returns the amount of characters available in this font.</summary>
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
