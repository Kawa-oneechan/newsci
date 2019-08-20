#pragma once
#include <vector>
#include <map>
#include "script.h"

#define	OBJID	(unsigned int)0x1234
#define	OBJNUM	(unsigned int)0xffff

#define NUMPROPERTIES 250

#define CLASSBIT	0x8000
#define CLONEBIT	0x0001
#define NODISPOSE	0x0002
#define NODISPLAY	0x0004

typedef unsigned int Selector;
typedef unsigned int ObjID;

typedef struct Property
{
	Selector selector;
	//enum { Number, String, Pointer } type;
	int value;
} Property;

//typedef std::map<Selector, char*> PropBag;
typedef std::map<Selector, char*> Selectors;

extern Selectors selectors;

typedef struct Object
{
/*
	unsigned int objID;
	unsigned int numProperties; //size, number of properties in the object
	unsigned int propertyDictOffset; //propDict, offset of property dictionary in hunk resource
	unsigned int methodDictOffset; //methDict, offset of method dictionary in hunk resource
	Script* classScript; //pointer to script node of object's class
	Script* script; //pointer to script node for the object
	Object* super; //pointer to superclass of object
	unsigned int info; //bitmapped information
*/
	unsigned int objID;
	unsigned int info;
	//unsigned int super;
	Object* superClass;
	Property properties[NUMPROPERTIES];
	int numProperties;
	//PropBag* properties;
	

public:
	Object* Clone();
	char* GetProperty(Selector selector);
	char* GetProperty(char* selector);
	char* SetProperty(Selector selector, char* value);
	char* SetProperty(char* selector, char* value);
	char* SetProperty(Selector selector, int value);
	char* SetProperty(char* selector, int value);
	void DefineProperty(Selector selector);
} Object;

extern Object* theGame;

