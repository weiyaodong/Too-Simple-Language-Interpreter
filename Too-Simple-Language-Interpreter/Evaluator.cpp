#include "Exception.h"
#include "Evaluator.h"
#include "Parser.h"

Object Object::evaluate() const
{
	if (type == NUMBER || type == BOOL || type == NOTHING)
	{
		return *this;
	}
	if (parameters.size() == 0)
	{
		return body->eval(scope.get());
	}  
	return *this; // ?
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
		return to_string(obj.boo);
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

Scope::~Scope()
{
	for (auto p: variable_table)
	{
		delete p.second;
	}
}
