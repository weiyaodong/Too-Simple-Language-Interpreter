#pragma once

#include <vector>
#include <functional>
#include <map>
#include "Parser.h"
#include <memory>

class ASTNode;

class Scope;

class Object
{
public:
	enum Object_Type
	{
		NUMBER,
		BOOL,
		FUNCTION,
		NOTHING
	};

	 
	Object(): num(0), boo(false), body(nullptr)
	{
		type = NOTHING;
	}

	explicit Object(Object_Type t) : type(t), body(nullptr), scope(nullptr)
	{
		num = 0;
		boo = false;
	}

	Object(const Object& obj)
	{
		type = obj.type;
		if (type == NUMBER)
		{
			num = obj.num;
		}
		else if (type == BOOL)
		{
			boo = obj.boo;
		}
		else if (type == FUNCTION)
		{
			body = obj.body;
			scope = obj.scope;
			parameters = obj.parameters;
		}
	}
	Object& operator = (const Object& obj)
	{
		type = obj.type;
		if (type == NUMBER)
		{
			num = obj.num;
		}
		else if (type == BOOL)
		{
			boo = obj.boo;
		}
		else if (type == FUNCTION)
		{
			body = obj.body;
			scope = obj.scope;
			parameters = obj.parameters;
		}
		return *this;
	}

	Object(Object_Type type, int val): num(val), boo(false), type(type), body(nullptr), scope(nullptr)
	{
		if (type != NUMBER)
		{
			throw Exception("Wrong object type");
		}
	}

	~Object();

	int num;
	bool boo;

	Object_Type type;

	// function
	const ASTNode* body;
	std::vector<std::string> parameters;
	std::shared_ptr<Scope> scope;

	Object evaluate() const;
	Object evaluate(std::vector<Object> arguments) const
	{
		if (arguments.size() > parameters.size())
		{
			throw Exception("Too many arguments");
		}
		return update(arguments).evaluate();
	}
	Object update(std::vector<Object> arguments) const;
};

template<>
std::string to_string(const Object& obj);

class Scope
{
	std::map<std::string, Object*> variable_table;
public:
//	static std::map<std::string, std::function<Object(std::vector<ASTNode>, Scope)>> built_in_functions;
//
//	static void build_in(const std::string& name, const std::function<Object(std::vector<ASTNode>, Scope)>& fun)
//	{
//		built_in_functions[name] = fun;
//	}

	Scope(const Scope* p = nullptr)
	{
		parent = p;
	}

	~Scope();

	const Scope* parent;

	Object* find(const std::string& name)
	{
		const Scope* current = this;
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

	Object* define(const std::string& name, Object* value)
	{		
		return variable_table[name] = value;
	}

	Object* find_in_top(const std::string& name)
	{
		if (variable_table.find(name) != variable_table.end())
		{
			return variable_table[name];
		}
		throw Exception("Can't find " + name + " in current top scope");
	}

	std::shared_ptr<Scope> update_scope(const std::vector<std::string>& names, const std::vector<Object*>& values) const
	{
		if (names.size() < values.size())
		{
			throw Exception("Too many arguments");
		}
		std::shared_ptr<Scope> new_scope = std::make_shared<Scope>(this);
		for (size_t i = 0; i < values.size(); i++)
		{
			new_scope->variable_table[names[i]] = values[i];
		}
		return new_scope;
	}

};


inline Object::~Object()
{
	if (type == FUNCTION)
	{
//		delete scope;
	}
}

