#include "NewSCI.h"
#include "object.h"

extern void Message(const char* text, const char* title);

Selectors selectors;


extern char* NeedPtr(unsigned int size);

Object* Object::Clone()
{
	//int size = this->numProperties * 2;
	//Object* newObj = new Object();
	auto newObj = (Object*)NeedPtr(sizeof(Object));
	#pragma warning(suppress:4345)
	newObj = new(newObj) Object();
	newObj->objID = this->objID;
	newObj->info = this->info;
	newObj->superClass = this->superClass;
	//newObj->properties = this->properties;
	newObj->numProperties = this->numProperties;
	for (auto i = 0; i < newObj->numProperties; i++)
	{
		newObj->properties[i].selector = this->properties[i].selector;
		newObj->properties[i].value = this->properties[i].value;
	}

	if (this->info & CLASSBIT)
	{
		newObj->superClass = this;
		newObj->info &= ~CLASSBIT;
	}

	newObj->info |= CLONEBIT;

	//increment script's reference count
	//auto sp = newObj->script;
	//sp->clones++;

	return newObj;
}

void Object::DefineProperty(Selector selector)
{
	this->properties[this->numProperties].selector = selector;
	this->properties[this->numProperties].value = NULL;
	this->numProperties++;
}

char* Object::GetProperty(Selector selector)
{
	for (auto i = 0; i < numProperties; i++)
	{
		if (properties[i].selector == selector)
			return (char*)properties[i].value;
	}
	char str[256];
	sprintf_s(str, 255, "Unknown selector \"%s\" for object \"%s\".", selectors.at(selector), (char*)GetProperty(20));
	Message(str, "PMachine");
	exit(1);
}

char* Object::SetProperty(Selector selector, char* value)
{
	for (auto i = 0; i < numProperties; i++)
	{
		if (properties[i].selector == selector)
			return (char*)(properties[i].value = (int)value);
	}
	char str[256];
	sprintf_s(str, 255, "Unknown selector \"%s\" for object \"%s\".", selectors.at(selector), (char*)GetProperty(20));
	Message(str, "PMachine");
	exit(1);
}

char* Object::SetProperty(Selector selector, int value)
{
	return SetProperty(selector, (char*)value);
}

char* Object::GetProperty(char* selector)
{
	SDL_LogVerbose(SDL_LOG_CATEGORY_SYSTEM, "Looking up selector \"%s\".", selector);
	auto it = selectors.begin();
	while (it != selectors.end())
	{
		if (!strcmp(it->second, selector))
			break;
		it++;
	}
	if (it == selectors.end())
	{
		char str[256];
		sprintf_s(str, 255, "Unknown selector \"%s\".", selector);
		Message(str, "PMachine");
		exit(1);
	}
	auto num = it->first;
	return GetProperty(num);
}

char* Object::SetProperty(char* selector, char* value)
{
	SDL_LogVerbose(SDL_LOG_CATEGORY_SYSTEM, "Looking up selector \"%s\".", selector);
	auto it = selectors.begin();
	while (it != selectors.end())
	{
		if (!strcmp(it->second, selector))
			break;
		it++;
	}
	if (it == selectors.end())
	{
		char str[256];
		sprintf_s(str, 255, "Unknown selector \"%s\".", selector);
		Message(str, "PMachine");
		exit(1);
	}
	auto num = it->first;
	return SetProperty(num, value);
}

char* Object::SetProperty(char* selector, int value)
{
	return SetProperty(selector, (char*)value);
}

