#include "SExp.h"
#if 0
#include <SDL_log.h>
#define log(X, ...) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, X, __VA_ARGS__)
#else
#define log(X, ...)
#endif
#define wipe(L) for (unsigned int _i = 0; _i < (L)->size(); _i++) delete (L)->at(_i);

namespace SExp
{
	Node::Node()
	{
		type = Null;
	}

	Node::~Node()
	{
		switch (type)
		{
		case List:
		case Array:
			log("Killing list\n");
			wipe(list);
			delete list;
			break;
		case String:
		case Symbol:
			log("Killing string/symbol \"%s\"\n", string);
			delete string;
			break;
		case Number:
			log("Killing number %d\n", number);
			break; //Nothing to do for a number.
		}
	}

	bool Node::IsForm()
	{
		return (type == List && list->size() >= 1 && list->at(0)->type == Symbol);
	}

	void Node::Collapse()
	{
		if (!IsForm())
			return;
		if (list->size() == 1)
			return;
		auto car = this[0].string;
		auto n = this[1];
		if (n.IsForm())
			n.Collapse();
		if (n.type != Number)
			return;
		long res = n.number;

#define SETUP \
	n = this[i]; \
	if (n.IsForm()) n.Collapse(); \
	if (n.type != Number) return;

		if (!strcmp(car, "+"))
		{
			for (unsigned int i = 2; i < list->size(); i++)
			{
				SETUP;
				res += n.number;
			}
		}
		else if (!strcmp(car, "-"))
		{
			for (unsigned int i = 2; i < list->size(); i++)
			{
				SETUP;
				res -= n.number;
			}
		}
		else if (!strcmp(car, "*"))
		{
			for (unsigned int i = 2; i < list->size(); i++)
			{
				SETUP;
				res *= n.number;
			}
		}
		else if (!strcmp(car, "/"))
		{
			for (unsigned int i = 1; i < list->size(); i++)
			{
				SETUP;
				res /= n.number;
			}
		}

#undef SETUP

		wipe(list);
		delete list;
		type = Number;
		number = res;
	}

	Node* Node::Car()
	{
		return &this[0];
	}

	bool Node::Is(const char* str)
	{
		return (this->type == String || this->type == Symbol) && !strcmp(str, this->string);
	}

	Node* &Node::operator[] (int index)
	{
		if (type != List && type != Array)
			throw 42;
		return list->at(index);
	}

	Expression::Expression(const char* str)
	{
		this->str = (char*)str;
		this->root = parse();
	}

	void Expression::Print()
	{
		printHelper(this->root);
		printf("\n");
	}

	void Expression::printHelper(Node* node)
	{
		switch(node->type)
		{
		case Null:
			printf("null");
		case List:
		case Array:
			{
				auto list = node->list;
				printf(node->type == List ? "(" : "[");
				for (unsigned int i = 0; i < list->size(); i++)
				{
					if (i)
						printf(" ");
					printHelper((Node*)(list->at(i)));
				}
				printf(node->type == List ? ")" : "]");
				break;
			}
		case Symbol:
			printf(node->string);
			break;
		case String:
			printf("\"%s\"", node->string);
			break;
		case Number:
			printf("%d", node->number);
			break;
		}
	}

	int depth = 0;
	Node* Expression::parse()
	{
		char c;
		bool stop = false;
		while (*this->str || *this->str != 0xFF)
		{
			c = *str;
			if (iswspace(c))
			{
				str++;
				continue;
			}
			else if (c == '(')
			{
				auto ret = parseList(false, ')');
				return ret;
			}
			else if (c == '[')
				return parseList(true, ']');
			else if (c == '"')
				return parseString('"');
			else if (c == '{')
				return parseString('}');
			else if (c == ';')
				parseComment();
			else
				return parseSymbolOrNumber();
			str++;
		}

		return NULL;
	}

	Node* Expression::parseList(bool asArray, char closeOn)
	{
		Node* node = (Node*)malloc(sizeof(Node));
		Node* thing = { 0 };
		node->type = asArray ? Array : List;
		node->list = new std::vector<Node*>();
		char c;
		str++;
		while (*str)
		{
			c = *str;
			if (c == closeOn)
			{
				str++;
				return node;
			}
			else if (iswspace(c))
			{
				str++;
				continue;
			}
			else
			{
				thing = parse();
				if (thing->type == List && asArray)
					throw 42;
				node->list->push_back(thing);
			}
			c = *str;
			if (c == closeOn)
			{
				str++;
				return node;
			}
			str++;
		}
		return node;
	}

	Node* Expression::parseString(char closeOn)
	{
		Node* node = (Node*)malloc(sizeof(Node));
		Node* thing = NULL;
		node->type = String;
		str++;
		char* strBeg = str++;
		unsigned int len = 0;
		char c;
		while (*str)
		{
			c = *str;
			if (c == closeOn)
				break;
			len++;
			str++;
		}
		char* buffer = (char*)malloc(len);
		char* b = buffer;
		len++;
		while (len--)
			*b++ = *strBeg++;
		*b = '\0';
		node->string = _strdup(buffer);
		return node;
	}

	Node* Expression::parseSymbolOrNumber()
	{
		Node* node = (Node*)malloc(sizeof(Node));
		char buffer[256] = { 0 };
		char* b = buffer;
		char c;
		while (*str)
		{
			c = *str;
			if (iswspace(c))
				break;
			if (c == ')')
				break;
			*b++ = c;
			str++;
		}
		*b = '\0';
		b = buffer;
		int allNum = true;
		while (*b)
		{
			if (!iswdigit(*b))
			{
				allNum = false;
				break;
			}
			b++;
		}
		if (allNum)
		{
			node->type = Number;
			node->number = atoi(buffer);
		}
		else
		{
			node->type = Symbol;
			node->string = _strdup(buffer);
		}
		return node;
	}

	Node* Expression::parseComment()
	{
		while (*str)
		{
			if (*str == '\r')
				break;
			str++;
		}
		return NULL;
	}
}
