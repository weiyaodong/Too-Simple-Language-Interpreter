#pragma once

#include <vector>
#include <functional>
#include <map>
#include "Parser.h"
#include <set>

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


	Object();

	explicit Object(Object_Type t);

	Object(const Object& obj);

	Object& operator =(const Object& obj);

	Object(Object_Type type, int val);

	explicit Object(bool b);

	~Object();

	int num;
	bool boo;

	Object_Type type;

	// function
	const ASTNode* body;
	std::vector<std::string> parameters;
	Scope* scope;

	bool operator ==(const Object& obj) const;

	bool strict_equal(const Object& obj) const;

	Object evaluate() const;
	Object evaluate(std::vector<Object> arguments) const;
	Object update(std::vector<Object> arguments) const;
};

template<>
std::string to_string(const Object& obj);

class Scope
{
	std::map<std::string, Object*> variable_table;
	std::set<Scope*> children;
public:
//	static std::map<std::string, std::function<Object(std::vector<ASTNode>, Scope)>> built_in_functions;
//
//	static void build_in(const std::string& name, const std::function<Object(std::vector<ASTNode>, Scope)>& fun)
//	{
//		built_in_functions[name] = fun;
//	}

	Scope(Scope* p = nullptr);

	~Scope();

	Scope* parent;

	Object* find(const std::string& name);

	Object* define(const std::string& name, Object* value);

	Object* modify(const std::string& name, Object* value);

	Object* force_define(const std::string& name, Object* value);

	Object* find_in_top(const std::string& name);

	Scope* update_scope(const std::vector<std::string>& names, const std::vector<Object*>& values) const;
};


inline Object::~Object()
{
	if (type == FUNCTION)
	{
//		delete scope;
//		std::cout << scope.use_count() << std::endl;
	}
}

