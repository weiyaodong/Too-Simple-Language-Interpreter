#include "Exception.h"
#include <vector>
#include <functional>
#include <map>

class ASTNode;

class Scope;

class Object
{
public:
	enum Object_Type
	{
		NUMBER,
		BOOL,
		FUNCTION
	};

	// ReSharper disable once CppPossiblyUninitializedMember
	Object()
	{

	}

	explicit Object(Object_Type t) : type(t), body(nullptr), scope(nullptr)
	{
		num = 0;
		boo = false;
	}

	~Object();

	int num;
	bool boo;

	Object_Type type;

	ASTNode* body;

	std::vector<std::string> parameters;

	Scope* scope;
};

class Scope
{
	std::map<std::string, Object> variable_table;
public:
	static std::map<std::string, std::function<Object(std::vector<ASTNode>, Scope)>> built_in_functions;

	static void build_in(const std::string& name, const std::function<Object(std::vector<ASTNode>, Scope)>& fun)
	{
		built_in_functions[name] = fun;
	}

	Scope* parent;

	Object find(const std::string& name)
	{
		Scope* current = this;
		while (current != nullptr)
		{
			if (variable_table.find(name) != variable_table.end())
			{
				return variable_table[name];
			}
			current = current->parent;
		}
		throw Exception(name + " is not found in current scope");
	}

	Object define(const std::string& name, const Object& value)
	{
		return variable_table[name] = value;
	}

	Object find_in_top(const std::string& name)
	{
		if (variable_table.find(name) != variable_table.end())
		{
			return variable_table[name];
		}
		throw Exception("Can't find " + name + " in current top scope");
	}

	Scope* update_scope(const std::vector<std::string>& names, const std::vector<Object>& values) const
	{
		if (names.size() < values.size())
		{
			throw Exception("Too many arguments");
		}
		Scope* new_scope = new Scope(*this);
		for (size_t i = 0; i < values.size(); i++)
		{
			new_scope->variable_table[names[i]] = values[i];
		}
		return new_scope;
	}

};


Object::~Object()
{
	if (type == FUNCTION)
	{
		delete scope;
	}
}