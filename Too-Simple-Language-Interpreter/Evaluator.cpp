#include "Evaluator.h"
#include "Parser.h"

Object::Object(): num(0), boo(false), body(nullptr), scope(nullptr)
{
	type = NOTHING;
}

Object::Object(Object_Type t): type(t), body(nullptr), scope(nullptr)
{
	num = 0;
	boo = false;
}

Object::Object(const Object& obj)
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

Object& Object::operator=(const Object& obj)
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

Object::Object(Object_Type type, int val): num(val), boo(false), type(type), body(nullptr), scope(nullptr)
{
	if (type != NUMBER)
	{
		throw RunTimeError("Wrong object type");
	}
}

Object::Object(bool b): num(0), boo(b), type(BOOL), body(nullptr), scope(nullptr)
{
}

bool Object::operator==(const Object& obj) const
{
	if (type == obj.type)
	{
		switch (type)
		{
		case NUMBER: return num == obj.num;
		case BOOL: return boo == obj.boo;
		default: return false;
		}
	}
	if (type == NUMBER && obj.type == BOOL) return num == obj.boo;
	if (obj.type == NUMBER && type == BOOL) return obj.num == boo;
	return false;
}

bool Object::strict_equal(const Object& obj) const
{
	if (type == obj.type)
	{
		switch (type)
		{
		case NUMBER: return num == obj.num;
		case BOOL: return boo == obj.boo;
		default: return false;
		}
	}
	return false;
}

Object Object::evaluate() const
{
	if (type == NUMBER || type == BOOL || type == NOTHING)
	{
		return *this;
	}
	if (parameters.size() == 0)
	{
		return body->eval(scope);
	}  
	return *this; // ?
}

Object Object::evaluate(std::vector<Object> arguments) const
{
	if (arguments.size() > parameters.size())
	{
		throw RunTimeError("Too many arguments");
	}
	return update(arguments).evaluate();
}

Object Object::update(std::vector<Object> arguments) const
{
	std::vector<Object*> values;
	std::vector<std::string> names;
	for (size_t i = 0; i < arguments.size(); i++)
	{
		values.push_back(new Object(arguments[i]));
		names.push_back(parameters[i]);
	}
	Object new_function;
	new_function.type = FUNCTION;
	new_function.body = body;
	new_function.scope = scope->update_scope(names, values);
	new_function.parameters = std::vector<std::string>(parameters.begin() + arguments.size(), parameters.end());

	return new_function;
}

template <>
std::string to_string<Object>(const Object& obj)
{
	if (obj.type == Object::NUMBER)
	{
		return to_string(obj.num);
	}
	if (obj.type == Object::BOOL)
	{
		return obj.boo ? "true" : "false";
	}
	if (obj.type == Object::NOTHING)
	{
		return "Nothing";
	}
	std::string result;
	if (obj.parameters.size() == 0)
	{
		result += "()";
	}
	else
	{
		result += "(" + obj.parameters[0];
		for (size_t i = 1; i < obj.parameters.size(); i++)
		{
			result += ", " + obj.parameters[i];
		}
		result += ")";
	}
	result += " => ";
	result += to_string(*obj.body);
	return result;
}

Scope::Scope(Scope* p)
{
	parent = p;
	if (p != nullptr)
	{
		p->children.insert(this);
	}
}

Scope::~Scope()
{
	for (auto p: children)
	{
		delete p;
	}
	for (auto p: variable_table)
	{
		delete p.second;
	}
}

Object* Scope::find(const std::string& name)
{
	Scope* current = this;
	while (current != nullptr)
	{
		if (current->variable_table.find(name) != current->variable_table.end())
		{
			return current->variable_table[name];
		}
		current = current->parent;
	}
	throw RunTimeError(name + " is not found in current scope");
}

Object* Scope::define(const std::string& name, Object* value)
{
	if (variable_table.find(name) != variable_table.end())
	{
		throw RunTimeError("Variable: " + name + " has been defined");
	}
	return variable_table[name] = value;
}

Object* Scope::modify(const std::string& name, Object* value)
{
	Scope* current = this;
	while (current != nullptr)
	{
		if (current->variable_table.find(name) != current->variable_table.end())
		{
			delete current->variable_table[name];
			return current->variable_table[name] = value;
		}
		current = current->parent;
	}
	throw RunTimeError(name + " is not found in current scope");
}

Object* Scope::find_in_top(const std::string& name)
{
	if (variable_table.find(name) != variable_table.end())
	{
		return variable_table[name];
	}
	throw RunTimeError("Can't find " + name + " in current top scope");
}

Scope* Scope::update_scope(const std::vector<std::string>& names, const std::vector<Object*>& values) const
{
	if (names.size() < values.size())
	{
		throw RunTimeError("Too many arguments");
	}
	Scope* new_scope = new Scope(const_cast<Scope*>(this));
	for (size_t i = 0; i < values.size(); i++)
	{
		new_scope->variable_table[names[i]] = values[i];
	}
	return new_scope;
}
