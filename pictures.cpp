#include "NewSCI.h"
#include "support\sol.hpp"
#include <map>

Pixels visualBuffer = NULL, priorityBuffer = NULL;
Pixels visualBackground, priorityBackground;
int screenWidth, screenHeight;
int screenPitch, screenSize;

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
std::map<const char*, Image*> images;
std::map<const char*, int> imageRefs;

Image::Image(int width, int height)
{
	this->filename = NULL;
	this->width = width;
	this->height = height;
	this->size = width * height;
	this->pitch = width * sizeof(Pixel);
	this->pixels = (Pixels)malloc(size * sizeof(Pixel));
}

Image::Image(const char* filename)
{
	auto it = images.find(filename);
	if (it != images.end())
	{
		SDL_LogVerbose(SDL_LOG_CATEGORY_VIDEO, "Image::Image: Returning \"%s\" from cache.", filename);
		this->filename = images[filename]->filename;
		this->width = images[filename]->width;
		this->height = images[filename]->height;
		this->pixels = images[filename]->pixels;
		imageRefs[filename]++;
	}
	unsigned long size, width, height;
	Handle png = LoadFile(filename, &size);
	std::vector<unsigned char> image;
	decodePNG(image, width, height, (const unsigned char*)png, size);
	free(png);
	size = width * height * sizeof(Pixel);
	this->filename = filename;
	this->width = width;
	this->height = height;
	this->size = width * height;
	this->pitch = width * sizeof(Pixel);
	this->pixels = (Pixels)malloc(size);
	memcpy(this->pixels, (Pixels)&image[0], size);
	images[filename] = this;
	imageRefs[filename] = 1;
}

Image::~Image()
{
	if (this->filename == NULL)
	{
		free(this->pixels);
		return;
	}
	imageRefs[this->filename]--;
	if (imageRefs[this->filename] == 0)
	{
		free(this->pixels);
		images.erase(images.find(this->filename));
		imageRefs.erase(imageRefs.find(this->filename));
	}
}
