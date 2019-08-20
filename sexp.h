#pragma once
#include <vector>

namespace SExp
{
	enum NodeType : char
	{
		Null,
		List,
		Array,
		String,
		Symbol,
		Number
	};

	struct Node
	{
		NodeType type;
		//union
		//{
			std::vector<Node*>* list;
			char* string;
			int number;
		//};

	public:
		Node();
		~Node();
		bool IsForm();
		void Collapse();
		Node* Car();
		bool Is(const char* str);
		Node* &operator[] (int);
	};

	class Expression
	{

	public:
		Node* root;
		char *str;

		Expression(const char* str);
		void Print();

		~Expression()
		{
			delete root;
		}

	private:
		void printHelper(Node* node);
		Node* parse();
		Node* parseList(bool asArray, char closeOn);
		Node* parseString(char closeOn);
		Node* parseSymbolOrNumber();
		Node* parseComment();
		Node* parseArray();
	};
}

