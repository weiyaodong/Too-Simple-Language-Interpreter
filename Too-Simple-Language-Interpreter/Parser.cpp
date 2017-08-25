#include "Parser.h"
#include <iostream>
#include <fstream>

template<>
std::string to_string<Object::Object_Type>(const Object::Object_Type& type)
{
	switch (type)
	{
	case Object::NUMBER: return "Number";
	case Object::BOOL: return "Bool";
	case Object::FUNCTION: return "Function";
	case Object::ARRAY: return "Array";
	case Object::NOTHING: return "Nothing";
	default: return "What the fuck";
	}
}


void object_type_assert(Object::Object_Type t1, Object::Object_Type t2)
{
	if (t1 != t2)
	{
		throw TypeError(to_string(t2), to_string(t1));
	}
}

void subscript_assert(int a, int b)
{
	if (a >= b)
	{
		throw SubscriptOutOfRange(to_string(a), to_string(b));
	}
}



std::pair<std::string, Scope*> find_member(ASTNode ast, Scope* scope)
{
	if (ast.children[0].type == ASTNode::AST_MEM_EXPR)
	{
		auto temp = find_member(ast.children[0], scope);
		auto temp2 = temp.second->find(temp.first);
		if (temp2->type != Object::FUNCTION)
		{
			throw TypeError(to_string(Object::FUNCTION), to_string(temp2->type));
		}
		auto scope2 = temp2->scope;
		return std::make_pair(ast.children[1].name, scope2);
	}
	if (ast.type == ASTNode::AST_MEM_EXPR)
	{
		auto temp = scope->find(ast.children[0].name);
		if (temp->type != Object::FUNCTION)
		{
			throw TypeError(to_string(Object::FUNCTION), to_string(temp->type));
		}
		auto scope2 = temp->scope;
		return std::make_pair(ast.children[1].name, scope2);
	}

	else
	{
		throw RunTimeError("Unrecogenized postfix expression");
	}
}

ObjectIdentity find_object(ASTNode ast, Scope* scope, const Object* cur_fun)
{
	if (ast.children.empty())
	{
		if (ast.type == ASTNode::AST_IDENT)
		{
			return ObjectIdentity(scope->find(ast.name));
		}
	}
	else if (ast.children[0].children.empty()) // end
	{
		if (ast.type == ASTNode::AST_MEM_EXPR)
		{
			auto temp = scope->find(ast.children[0].name);
			object_type_assert(temp->type, Object::FUNCTION);
			return ObjectIdentity(ast.children[1].name, temp->scope);
		}
		if (ast.type == ASTNode::AST_VISIT_EXPR)
		{
			auto temp = scope->find(ast.children[0].name);
			object_type_assert(temp->type, Object::ARRAY);
			auto temp2 = ast.children[1].eval(scope, cur_fun);
			object_type_assert(temp2.type, Object::NUMBER);
			subscript_assert(temp2.num, temp->array.size());
			return ObjectIdentity(temp->array[temp2.num]);
		}
	}
	else // not end
	{
		auto temp = find_object(ast.children[0], scope, cur_fun);
		if (temp.flag) // last is array and only obj is working
		{
			if (ast.type == ASTNode::AST_MEM_EXPR)
			{
				auto temp2 = *temp.obj;
				object_type_assert(temp2.type, Object::FUNCTION);
				return ObjectIdentity(ast.children[1].name, temp2.scope);
			}
			if (ast.type == ASTNode::AST_VISIT_EXPR)
			{
				auto temp2 = temp.obj;
				object_type_assert(temp2->type, Object::ARRAY);
				auto temp3 = ast.children[1].eval(scope, cur_fun);
				object_type_assert(temp3.type, Object::NUMBER);
				subscript_assert(temp3.num, temp2->array.size());
				return ObjectIdentity(temp2->array[temp3.num]);
			}
		}
		else // last is object and scope and name is working
		{
			if (ast.type == ASTNode::AST_MEM_EXPR)
			{
				auto temp2 = temp.scope->find(temp.name);
				object_type_assert(temp2->type, Object::FUNCTION);
				return ObjectIdentity(ast.children[1].name, temp2->scope);
			}
			if (ast.type == ASTNode::AST_VISIT_EXPR)
			{
				auto temp2 = temp.scope->find(temp.name);
				object_type_assert(temp2->type, Object::ARRAY);
				auto temp3 = ast.children[1].eval(scope, cur_fun); // calculate the subscript
				object_type_assert(temp3.type, Object::NUMBER);
				subscript_assert(temp3.num, temp2->array.size());
				return ObjectIdentity(temp2->array[temp3.num]);
			}
		}
	}
	throw RunTimeError("Can't resolved the expression as lvalue :" + to_string(ast));
}


ASTNode::ASTNode(): num(0)
{
	type = AST_EMPTY;
}

ASTNode::ASTNode(int val): num(val)
{
	type = AST_NUM;
}

ASTNode::ASTNode(ASTNode_Type t, std::string n): num(0), type(t)
{
	if (type == AST_IDENT)
	{
		name = n;
	}
}

ASTNode::ASTNode(const ASTNode& ast)
{
	num = ast.num;
	children = ast.children;
	calc_flag = ast.calc_flag;
	name = ast.name;
	type = ast.type;
	oper = ast.oper;
}

Object ASTNode::eval(Scope* scope,const Object* current_fun) const
{
	if (type == AST_MUL_EXPR)
	{
		std::vector<int> num;
		for (size_t i = 0; i < children.size(); i++)
		{
			Object temp = children[i].eval(scope, current_fun);
			if (temp.type == Object::BOOL)
			{
				num.push_back(temp.boo);
				continue;
			}
			object_type_assert(temp.type, Object::NUMBER);

			num.push_back(temp.num);
		}
		int ans = num[0];
		for (size_t i = 0; i < children.size() - 1; i++)
		{
			if (calc_flag[i])
			{
				ans *= num[i + 1];
			}
			else
			{
				if (num[i + 1] == 0)
				{
					throw RunTimeError("Devide by zero");
				}
				ans /= num[i + 1];
			}
		}
		return Object(Object::NUMBER, ans);
	}
	if (type == AST_ADD_EXPR)
	{
		std::vector<int> num;
		Object res = children[0].eval(scope, current_fun);
		if (res.type == Object::ARRAY)
		{
			for (size_t i = 1; i < children.size(); i++)
			{
				if (!calc_flag[i - 1])
				{
					throw RunTimeError(" Operator - can't apply to arrays");
				}
				auto ary = children[i].eval(scope, current_fun);
				for (size_t j = 0; j < ary.array.size(); j++)
				{
					res.array.push_back(new Object(*ary.array[j]));
				}
			}
			return res;
		}
		else
		{
			for (size_t i = 0; i < children.size(); i++)
			{
				Object temp = children[i].eval(scope, current_fun);
				if (temp.type == Object::BOOL)
				{
					num.push_back(temp.boo);
					continue;
				}
				object_type_assert(temp.type, Object::NUMBER);

				num.push_back(temp.num);
			}
			int ans = num[0];
			for (size_t i = 0; i < children.size() - 1; i++)
			{
				if (calc_flag[i])
				{
					ans += num[i + 1];
				}
				else
				{
					ans -= num[i + 1];
				}
			}
			return Object(Object::NUMBER, ans);
		}
	}
	if (type == AST_NUM)
	{
		return Object(Object::NUMBER, num);
	}
	if (type == AST_IDENT)
	{
		if (name == "this")
		{
			if (current_fun == nullptr)
			{
				throw RunTimeError("No function call in this scope.");
			}
			return *current_fun;
		}
		return *scope->find(name); // Error here
	}
	if (type == AST_ARRAY)
	{
		auto temp = Object();
		temp.type = Object::ARRAY;
		for (size_t i = 0; i < children.size(); i++)
		{
			temp.array.push_back(new Object(children[i].eval(scope, current_fun)));
		}
		return temp;
	}
	if (type == AST_VAR_DEF_EXPR)
	{
		for (size_t i = 0; i < children.size(); i += 2)
		{
			if (children[i + 1].type == AST_EMPTY)
			{
				scope->define(children[i].name, new Object());
			}
			else
			{
				scope->define(children[i].name, new Object(children[i + 1].eval(scope, current_fun)));
			}
		}
		return Object();
	}
	
	if (type == AST_LAMBDA_EXPR)
	{
		Object temp;
		temp.type = Object::FUNCTION;
		temp.scope = new Scope(scope);
		std::vector<std::string> param;
		for (size_t i = 0; i < children[0].children.size(); i++)
		{
			param.push_back(children[0].children[i].name);
		}
		temp.parameters = param;
		temp.body = &children[1];
		return temp;
	}
	if (type == AST_FUN_DEF_STMT)
	{
		Object* temp = new Object();
		temp->type = Object::FUNCTION;
		temp->scope = new Scope(scope);
		std::vector<std::string> param;
		for (size_t i = 0; i < children[1].children.size(); i++)
		{
			param.push_back(children[1].children[i].name);
		}
		temp->parameters = param;
		temp->body = &children[2];
		scope->define(children[0].name, temp);
		return Object();
	}
	if (type == AST_FUN_CALL_EXPR)
	{
		Object* fun;
		if (children[0].type == AST_IDENT || children[0].type == AST_MEM_EXPR || children[0].type == AST_VISIT_EXPR)
		{
			auto temp = find_object(children[0], scope, current_fun);
			
			if (temp.flag) // function in array
			{
				object_type_assert(temp.obj->type, Object::FUNCTION);
				fun = temp.obj;
			}
			else
			{
				fun = temp.scope->find(temp.name);
				object_type_assert(fun->type, Object::FUNCTION);
			}
		}
		else 
		{
			// ReSharper disable once CppMsExtAddressOfClassRValue
			fun = &children[0].eval(scope, current_fun);
			object_type_assert(fun->type, Object::FUNCTION);
		}
		object_type_assert(fun->type, Object::FUNCTION);
		if (children[1].children.size() > fun->parameters.size())
		{
			throw RunTimeError("Too many arguments for function : " + to_string(fun));
		}
		std::vector<Object> param;
		for (size_t i = 0; i < children[1].children.size(); i++)
		{
			param.push_back(children[1].children[i].eval(scope, current_fun));
		}
		if (fun->body->type != AST_BLOCK)
		{
			return fun->evaluate(param);
		}
		try
		{
			return fun->evaluate(param);
		}
		catch (const Object& obj)
		{
			return obj;
		}
		catch (Exception exp)
		{
			throw;
		}
	}
	if (type == AST_IF_STMT)
	{
		auto temp = children[0].eval(scope, current_fun);
		if (!(temp.type == Object::NUMBER && temp.num == 0 || temp.type == Object::BOOL && temp.boo == false))
		{
			return children[1].eval(scope, current_fun);
		}
		if (children.size() == 3)
		{
			return children[2].eval(scope, current_fun);
		}
		return Object();
	}
	if (type == AST_RET_STMT)
	{
		throw children[0].eval(scope, current_fun);
	}
	
	if (type == AST_ASN_EXPR)
	{
		if (children[0].type == AST_IDENT)
		{
			if (oper == "=") return *scope->modify(children[0].name, new Object(children[1].eval(scope, current_fun)));
			auto temp = *scope->find(children[0].name);
			auto temp2 = children[1].eval(scope, current_fun);
			if (temp.type == Object::ARRAY && temp2.type == Object::ARRAY && oper == "+=") // concat two arrays
			{
				for (size_t i = 0; i < temp2.array.size(); i++)
				{
					temp.array.push_back(new Object(*temp2.array[i]));
				}
				return *scope->modify(children[0].name, new Object(temp));
			}
			if (temp.type != Object::NUMBER || temp2.type != Object::NUMBER)
			{
				throw RunTimeError(oper + " operator must apply to two Numbers or two Arrays");
			}
			if (oper == "+=") return *scope->modify(children[0].name, new Object(Object::NUMBER, temp.num + temp2.num));
			if (oper == "-=") return *scope->modify(children[0].name, new Object(Object::NUMBER, temp.num - temp2.num));
			if (oper == "*=") return *scope->modify(children[0].name, new Object(Object::NUMBER, temp.num * temp2.num));
			if (temp2.num == 0)
			{
				throw RunTimeError("Divide by zero");
			}
			if (oper == "/=") return *scope->modify(children[0].name, new Object(Object::NUMBER, temp.num / temp2.num));
		}
		else
		{
			auto temp = find_object(children[0], scope, current_fun);
			if (!temp.flag) 
			{
				auto scope2 = temp.scope;
				std::string vname = temp.name;
				if (oper == "=") return *scope2->force_define(vname, new Object(children[1].eval(scope, current_fun)));
				auto temp3 = *scope2->find(vname);
				auto temp2 = children[1].eval(scope, current_fun);
				if (temp3.type == Object::ARRAY && temp2.type == Object::ARRAY && oper == "+=")
				{
					for (size_t i = 0; i < temp2.array.size(); i++)
					{
						temp3.array.push_back(new Object(*temp2.array[i]));
					}
					return *scope2->modify(vname, new Object(temp3));
				}
				if (temp3.type != Object::NUMBER || temp2.type != Object::NUMBER)
				{
					throw RunTimeError(oper + " operator must apply to two Numbers");
				}
				if (oper == "+=") return *scope2->modify(vname, new Object(Object::NUMBER, temp3.num + temp2.num));
				if (oper == "-=") return *scope2->modify(vname, new Object(Object::NUMBER, temp3.num - temp2.num));
				if (oper == "*=") return *scope2->modify(vname, new Object(Object::NUMBER, temp3.num * temp2.num));
				if (temp2.num == 0)
				{
					throw RunTimeError("Divide by zero");
				}
				if (oper == "/=") return *scope2->force_define(vname, new Object(Object::NUMBER, temp3.num / temp2.num));
			}
			else
			{
				Object* cur = temp.obj;
				
				if (oper == "=")
				{
					return (*cur = children[1].eval(scope, current_fun));
				}
				auto temp2 = children[1].eval(scope, current_fun);

				if (cur->type == Object::ARRAY && temp2.type == Object::ARRAY && oper == "+=")
				{
					for (size_t i = 0; i < temp2.array.size(); i++)
					{
						cur->array.push_back(new Object(*temp2.array[i]));
					}
					return *cur;
				}
				if (cur->type != Object::NUMBER || temp2.type != Object::NUMBER)
				{
					throw RunTimeError(oper + " operator must apply to two Numbers");
				}
				
				if (oper == "+=") return (*cur = Object(Object::NUMBER, cur->num + temp2.num));
				if (oper == "-=") return (*cur = Object(Object::NUMBER, cur->num - temp2.num));
				if (oper == "*=") return (*cur = Object(Object::NUMBER, cur->num * temp2.num));
				if (temp2.num == 0)
				{
					throw RunTimeError("Divide by zero");
				}
				if (oper == "/=") return (*cur = Object(Object::NUMBER, cur->num / temp2.num));
			}
		}
	}
	if (type == AST_WHILE_STMT)
	{
		auto temp = children[0].eval(scope, current_fun);
		Object result;
		while (!((temp.type == Object::NUMBER && temp.num == 0) || (temp.type == Object::BOOL && temp.boo == false)))
		{
			try
			{
				result = children[1].eval(scope, current_fun);
			}
			catch (const Exception& )
			{
				throw;
			}
			catch (const LoopInterrupt& interrupt)
			{
				if (interrupt.get_flag() == 1) break;
				if (interrupt.get_flag() == 0)
				{
					temp = children[0].eval(scope, current_fun);
					continue;
				}
			}
			temp = children[0].eval(scope, current_fun);
		}
		return result;
	}
	if (type == AST_PRINT_STMT)
	{
		std::string result = to_string(children[0].eval(scope, current_fun));
//		std::cout << to_string(children[0].eval(scope)) << std::endl;
		for (size_t i = 1; i < children.size(); i++)
		{
			result += ", " + to_string(children[i].eval(scope, current_fun));
		}
		std::cout << result << std::endl;
		return Object();
	}
	if (type == AST_READ_STMT)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			std::string str;
			std::cin >> str;
			scope->modify(children[0].name, new Object(Parser(str).parse_expression().eval(scope, current_fun)));
		}
		return Object();
	}
	if (type == AST_BLOCK)
	{
		Object temp;
		for (size_t i = 0; i < children.size(); i++)
		{
			temp = children[i].eval(scope, current_fun);
		}
		return temp;
	}
	
	if (type == AST_STMT)
	{
		return children[0].eval(scope, current_fun);
	}
	if (type == AST_AND_EXPR)
	{
		bool result = true;
		for (size_t i = 0; i < children.size(); i++)
		{
			auto temp = children[i].eval(scope, current_fun);
			if (temp.type != Object::NUMBER && temp.type != Object::BOOL)
			{
				throw TypeError(to_string(Object::BOOL), to_string(temp.type));
			}
			if (temp.type == Object::NUMBER)
			{
				result = result && bool(temp.num);
			}
			else
			{
				result = result && temp.boo;
			}
		}
		return Object(result);
	}
	if (type == AST_OR_EXPR)
	{
		bool result = false;
		for (size_t i = 0; i < children.size(); i++)
		{
			auto temp = children[i].eval(scope, current_fun);
			if (temp.type != Object::NUMBER && temp.type != Object::BOOL)
			{
				throw TypeError(to_string(Object::BOOL), to_string(temp.type));
			}
			if (temp.type == Object::NUMBER)
			{
				result = result || bool(temp.num);
			}
			else
			{
				result = result || temp.boo;
			}
		}
		return Object(result);
	}
	if (type == AST_RELAT_EXPR)
	{
		int a1, a2;
		auto temp1 = children[0].eval(scope, current_fun);
		auto temp2 = children[1].eval(scope, current_fun);
		switch (temp1.type)
		{
		case Object::BOOL: a1 = temp1.boo; break;
		case Object::NUMBER: a1 = temp1.num; break;
		default: throw TypeError(to_string(Object::NUMBER), to_string(temp1.type));
		}
		switch (temp2.type)
		{
		case Object::BOOL: a2 = temp2.boo; break;
		case Object::NUMBER: a2 = temp2.num; break;
		default: throw TypeError(to_string(Object::NUMBER), to_string(temp2.type));
		}
		
		if (oper == "<")return Object(a1 < a2);
		if (oper == ">")return Object(a1 > a2);
		if (oper == "<=")return Object(a1 <= a2);
		if (oper == ">=")return Object(a1 >= a2);	
		throw RunTimeError("Unknown operater : " + oper);
	}
	if (type == AST_EQ_EXPR)
	{
		auto temp1 = children[0].eval(scope, current_fun);
		auto temp2 = children[1].eval(scope, current_fun);
		if (oper == "==") return Object(temp1 == temp2);
		if (oper == "!=") return Object(!(temp1 == temp2));
		if (oper == "===") return Object(temp1.strict_equal(temp2));
		if (oper == "!==") return Object(!temp1.strict_equal(temp2));
		throw RunTimeError("Unknown operater : " + oper);
	}
	
	if (type == AST_MEM_EXPR)
	{
		// array todo
		auto temp = children[0].eval(scope, current_fun);
		if (children[1].type != AST_IDENT)
		{
			throw RunTimeError("Unrecognized member " + to_string(children[1]));
		}
		if (temp.type == Object::ARRAY)
		{			
			auto temp2 = children[1].name;
			if (temp2 == "length")
			{
				return Object(Object::NUMBER, temp.array.size());
			}
			throw RunTimeError("Unrecognized array method " + temp2);
		}
		object_type_assert(temp.type, Object::FUNCTION);
		return *temp.scope->find(children[1].name);
	}
	if (type == AST_BREAK_STMT)
	{
		throw LoopInterrupt(1);
	}
	if (type == AST_CONTINUE_STMT)
	{
		throw LoopInterrupt(0);
	}
	if (type == AST_FOR_STMT)
	{
		children[0].eval(scope, current_fun);
		Object result;
		auto temp = children[1].eval(scope, current_fun);
		while (!((temp.type == Object::NUMBER && temp.num == 0) || (temp.type == Object::BOOL && temp.boo == false)))
		{
			try
			{
				result = children[3].eval(scope, current_fun);
			}
			catch (const Exception&)
			{
				throw;
			}
			catch (const LoopInterrupt& interrupt)
			{
				if (interrupt.get_flag() == 1) break;
				if (interrupt.get_flag() == 0)
				{
					children[2].eval(scope, current_fun);
					temp = children[1].eval(scope, current_fun);
					continue;
				}
			}
			children[2].eval(scope, current_fun);
			temp = children[1].eval(scope, current_fun);
		}
		return result;
	}
	if (type == AST_PRI_EXPR)
	{
		return children[0].eval(scope, current_fun);
	}

	if (type == AST_VISIT_EXPR)
	{
		auto ary = children[0].eval(scope, current_fun);
		object_type_assert(ary.type, Object::ARRAY);
		auto subscript = children[1].eval(scope, current_fun);
		object_type_assert(subscript.type, Object::NUMBER);
		subscript_assert(subscript.num, ary.array.size());
		return *ary.array[subscript.num]; // visit the array by subscript
	}
	
	else
	{
		throw RunTimeError("what the fuck ?");
	}
}

template <>
std::string to_string<ASTNode>(const ASTNode& node)
{
	if (node.type == ASTNode::AST_NUM)
	{
		return to_string(node.num);
	}
	if (node.type == ASTNode::AST_IDENT)
	{
		return node.name;
	}
	if (node.type == ASTNode::AST_ADD_EXPR)
	{
		if (node.children.size() == 1)
		{
			return to_string(node.children[0]);
		}
		std::string result = "(" + to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			if (node.calc_flag[i - 1])
			{
				result += " + ";
			}
			else
			{
				result += " - ";
			}
			result += to_string(node.children[i]);
		}
		return result + ")";
	}
	if (node.type == ASTNode::AST_MUL_EXPR)
	{
		if (node.children.size() == 1)
		{
			return to_string(node.children[0]);
		}
		std::string result = "(" + to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			if (node.calc_flag[i - 1])
			{
				result += " * ";
			}
			else
			{
				result += " / ";
			}
			result += to_string(node.children[i]);
		}
		return result + ")";
	}
	if (node.type == ASTNode::AST_RELAT_EXPR)
	{
		return to_string(node.children[0]) + " " + node.oper + " " + to_string(node.children[1]);
	}
	
	if (node.type == ASTNode::AST_EQ_EXPR)
	{
		return to_string(node.children[0]) + " " + node.oper + " " + to_string(node.children[1]);
	}
	if (node.type == ASTNode::AST_AND_EXPR)
	{
		std::string result = to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += " && " + to_string(node.children[i]);
		}
		return result;
	}
	if (node.type == ASTNode::AST_OR_EXPR)
	{
		std::string result = to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += " || " + to_string(node.children[i]);
		}
		return result;
	}
	if (node.type == ASTNode::AST_PRI_EXPR)
	{
		return "(" + to_string(node.children[0]) + ")";
	}
	if (node.type == ASTNode::AST_MEM_EXPR)
	{
		return to_string(node.children[0]) + "." + to_string(node.children[1]);
	}
	
	if (node.type == ASTNode::AST_FUN_CALL_EXPR)
	{
		if (node.children[0].type != ASTNode::AST_LAMBDA_EXPR)
		{
			return to_string(node.children[0]) + to_string(node.children[1]);
		}
		return "(" + to_string(node.children[0]) + ")" + to_string(node.children[1]);
	}
	if (node.type == ASTNode::AST_EMPTY)
	{
		return "";
	}
	if (node.type == ASTNode::AST_FOR_STMT)
	{
		std::string result = "for (";
		if (node.children[0].type == ASTNode::AST_VAR_DEF_EXPR)
		{
			result += to_string(node.children[0]);
		}
		else
		{
			result += to_string(node.children[0]) + ";";
		}
		result += to_string(node.children[1]) + ";";
		result += to_string(node.children[2]) + ")";
		return result + to_string(node.children[3]);
	}
	if (node.type == ASTNode::AST_FUN_CALL_PARA_LIST)
	{
		std::string result = "(";
		if (node.children.size() == 0)
		{
			result += ")";
			return result;
		}
		result += to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += ", " + to_string(node.children[i]);
		}
		result += ")";
		return result;
	}
	if (node.type == ASTNode::AST_LAMBDA_EXPR)
	{
		return to_string(node.children[0]) + " => " + to_string(node.children[1]);
	}
	
	if (node.type == ASTNode::AST_PRINT_STMT)
	{
		std::string result = "print " + to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += ", " + to_string(node.children[i]);
		}
		return result + ";";
	}
	if (node.type == ASTNode::AST_READ_STMT)
	{
		std::string result = "read " + to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += ", " + to_string(node.children[i]);
		}
		return result + ";";
	}
	if (node.type == ASTNode::AST_BLOCK)
	{
		if (node.children.size() == 0)
		{
			return "{}";
		}
		std::string result = "{\n" + to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += "\n" + to_string(node.children[i]);
		}
		return result + "\n}";
	}
	if (node.type == ASTNode::AST_IF_STMT)
	{
		if (node.children.size() == 2)
		{
			return "if (" + to_string(node.children[0]) + ")" + to_string(node.children[1]);
		}
		else
		{
			return "if (" + to_string(node.children[0]) + ")" + to_string(node.children[1]) + "\nelse " + to_string(node.children[2]);
		}
	}
	if (node.type == ASTNode::AST_WHILE_STMT)
	{
		return "while (" + to_string(node.children[0]) + ")" + to_string(node.children[1]);
	}
	
	if (node.type == ASTNode::AST_RET_STMT)
	{
		return "ret " + to_string(node.children[0]) + ";";
	}
	if (node.type == ASTNode::AST_FUN_DEF_PARA_LIST)
	{
		std::string result = "(";
		if (node.children.size() == 0)
		{
			result += ")";
			return result;
		}
		result += to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += ", " + to_string(node.children[i]);
		}
		result += ")";
		return result;
	}
	if (node.type == ASTNode::AST_FUN_DEF_STMT)
	{
		return "fun " + to_string(node.children[0]) + to_string(node.children[1]) + " => " + to_string(node.children[2]);
	}
	if (node.type == ASTNode::AST_BREAK_STMT)
	{
		return "break;";
	}
	if (node.type == ASTNode::AST_CONTINUE_STMT)
	{
		return "continue;";
	}
	
	if (node.type == ASTNode::AST_STMT)
	{
//		if (node.children[0].type == ASTNode::AST_EXPR || node.children[0].type == ASTNode::AST_ASN_EXPR || node.type)
//		{
//			return to_string(node.children[0]) + ";";
//		}
		return to_string(node.children[0]) + ";";
	}
	if (node.type == ASTNode::AST_ASN_EXPR)
	{
		return to_string(node.children[0]) + " = " + to_string(node.children[1]);
	}
	if (node.type == ASTNode::AST_VAR_DEF_EXPR)
	{
		std::string result = "var ";
		if (node.children[1].type == ASTNode::AST_EMPTY)
		{
			result += node.children[0].name;
		}
		else
		{
			result += node.children[0].name + " = " + to_string(node.children[1]);
		}
		for (size_t i = 2; i < node.children.size(); i+=2)
		{
			if (node.children[i + 1].type == ASTNode::AST_EMPTY)
			{
				result += ", " + node.children[i].name;
			}
			else
			{
				result += ", " + node.children[i].name + " = " + to_string(node.children[i + 1]);
			}
		}
		return result + ";";
	}
	if (node.type == ASTNode::AST_VISIT_EXPR)
	{
		return to_string(node.children[0]) + "[" + to_string(node.children[1]) + "]";
	}
	if (node.type == ASTNode::AST_ARRAY)
	{
		std::string result = "[";
		if (node.children.empty())
		{
			return "[]";
		}
		result += to_string(node.children[0]);
		for (size_t i = 1; i < node.children.size(); i++)
		{
			result += ", " + to_string(node.children[i]);
		}
		return result + "]";
	}
	return {};
}

Parser::Parser(std::string code)
{
	tokens = TokenStream(code).retrieve_all_tokens();
}

Token Parser::current_token() const
{
	if (pos < tokens.size())
	{
		return tokens[pos];
	}
	//		throw Exception("No more tokens");
	return Token::empty_token();
}

Token Parser::next_token()
{
	if (pos < tokens.size())
	{
		pos++;
	}
	else
	{
		return Token::empty_token();
		//			throw Exception("No more tokens");
	}
	return current_token();
}

void Parser::match_token()
{
	next_token();
}

void Parser::match_token(std::string str)
{
	if (current_token() == str)
	{
		next_token();
		return;
	}
	throw ParseError("Can't match " + to_string(current_token()) + " with " + "'" + str + "'");
}

void Parser::match_token(Token::Token_Type type)
{
	if (current_token().type == type)
	{
		next_token();
	}
	throw ParseError("Can't match " + to_string(current_token()) + " with type: " + to_string(type));
}

ASTNode Parser::parse_number()
{
	if (current_token().type == Token::T_NUMBER)
	{
		auto temp = ASTNode(current_token().num);
		next_token();
		return temp;
	}
	throw ParseError("Current token is not a number");
}

ASTNode Parser::parse_identifier()
{
	if (current_token().type == Token::T_IDENTIFIER)
	{
		auto temp = ASTNode(ASTNode::AST_IDENT, current_token().ident);
		next_token();
		return temp;
	}
	throw ParseError("Current token is not an identifier");
}

ASTNode Parser::parse_block()
{
	ASTNode temp;
	temp.type = ASTNode::AST_BLOCK;
	match_token("{");
	while (!(current_token() == "}"))
	{
		temp.children.push_back(parse_statement());
	}
	match_token("}");
	return temp;
}

ASTNode Parser::parse_function_definition_parameters_list()
{
	ASTNode temp;
	temp.type = ASTNode::AST_FUN_DEF_PARA_LIST;
	match_token("(");
	if (current_token() == ")")
	{
		match_token(")");
		return temp;
	}
	temp.children.push_back(parse_identifier());
	ASTNode temp2;
	while (true)
	{
		if (current_token() == ",")
		{
			match_token(",");
			temp.children.push_back(parse_identifier());
			continue;
		}
		if (current_token() == ")")
		{
			match_token(")");
			return temp;
		}
		throw ParseError("Unexpected token:" + to_string(current_token()));
	}
}

ASTNode Parser::parse_function_definition_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_FUN_DEF_STMT;
	match_token("fun");
	temp.children.push_back(parse_identifier());
	temp.children.push_back(parse_function_definition_parameters_list());
	match_token("=>");
	temp.children.push_back(parse_block());
	return temp;
}

ASTNode Parser::parse_statement()
{
	if (current_token() == ";")
	{
		ASTNode temp;
		temp.type = ASTNode::AST_EMPTY;
		return temp;
	}
	if (current_token() == "{")
	{
		return parse_block();
	}
	if (current_token() == "if")
	{
		return parse_if_statement();
	}
	if (current_token() == "while")
	{
		return parse_while_statement();
	}
	if (current_token() == "fun")
	{
		return parse_function_definition_statement();
	}
	if (current_token() == "ret")
	{
		return parse_return_statement();
	}
	if (current_token() == "var")
	{
		return parse_variable_definition_statement();
	}
	if (current_token() == "print")
	{
		return parse_print_statement();
	}
	if (current_token() == "read")
	{
		return parse_read_statement();
	}
	if (current_token() == "for")
	{
		return parse_for_statement();
	}
	if (current_token() == "break")
	{
		return parse_break_statement();
	}
	if (current_token() == "continue")
	{
		return parse_continue_statement();
	}
	ASTNode temp;
	temp.type = ASTNode::AST_STMT;
	temp.children.push_back(parse_expression());
	match_token(";");

	return temp;
}

ASTNode Parser::parse_assign_expression()
{
	ASTNode temp;
	temp.type = ASTNode::AST_ASN_EXPR;
	temp.children.push_back(parse_postfix_expr());
	if (current_token() == "=" || current_token() == "+=" || current_token() == "-=" || current_token() == "*=" || current_token() == "/=")
	{		
		temp.oper = current_token().oper;
		match_token();
	}
	else
	{
		throw ParseError("Expected eq_oper , get : " + current_token().oper);
	}
	temp.children.push_back(parse_expression());
	return temp;
}

ASTNode Parser::parse_if_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_IF_STMT;
	match_token("if");
	match_token("(");
	temp.children.push_back(parse_expression());
	match_token(")");
	temp.children.push_back(parse_statement());
	if (current_token() == "else")
	{
		match_token();
		temp.children.push_back(parse_statement());
	}
	return temp;
}

ASTNode Parser::parse_print_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_PRINT_STMT;
	match_token("print");
	temp.children.push_back(parse_expression()); while (current_token() == ",")
	{
		match_token(",");
		temp.children.push_back(parse_expression());
	}
	match_token(";");
	return temp; 
}

ASTNode Parser::parse_read_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_READ_STMT;
	match_token("read");
	temp.children.push_back(parse_identifier());
	while (current_token() == ",")
	{
		match_token(",");
		temp.children.push_back(parse_identifier());
	}
	match_token(";");
	return temp;
}

ASTNode Parser::parse_while_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_WHILE_STMT;
	match_token("while");
	match_token("(");
	temp.children.push_back(parse_expression());
	match_token(")");
	temp.children.push_back(parse_statement());
	return temp;
}

ASTNode Parser::parse_break_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_BREAK_STMT;
	match_token("break");
	match_token(";");
	return temp;
}

ASTNode Parser::parse_continue_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_CONTINUE_STMT;
	match_token("continue");
	match_token(";");
	return temp;
}

ASTNode Parser::parse_return_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_RET_STMT;
	match_token("ret");
	temp.children.push_back(parse_expression());
	match_token(";");
	return temp;
}

ASTNode Parser::parse_function_call_parameters_list()
{
	ASTNode temp;
	temp.type = ASTNode::AST_FUN_CALL_PARA_LIST;
	match_token("(");
	if (current_token() == ")")
	{
		match_token(")");
		return temp;
	}
	ASTNode temp2;
	while (true)
	{
		temp.children.push_back(parse_expression());
		if (current_token() == ",")
		{
			match_token(",");
			continue;
		}
		if (current_token() == ")")
		{
			match_token(")");
			return temp;
		}
		throw ParseError("Unexpected token:" + to_string(current_token()));
	}
}

ASTNode Parser::parse_lambda_expression()
{
	ASTNode temp;
	temp.type = ASTNode::AST_LAMBDA_EXPR;
	temp.children.push_back(parse_function_definition_parameters_list());
	match_token("=>");
	if (current_token() == "{")
	{
		temp.children.push_back(parse_block());
		return temp;
	}
	temp.children.push_back(parse_expression());
	return temp;
}

ASTNode Parser::parse_function_call()
{
	ASTNode temp;
	temp.type = ASTNode::AST_FUN_CALL_EXPR;
	if (current_token().type == Token::T_IDENTIFIER)
	{
		temp.children.push_back(parse_identifier());
		temp.children.push_back(parse_function_call_parameters_list());
		return temp;
	}
	else
	{
		match_token("(");
		temp.children.push_back(parse_expression());
		match_token(")");
		temp.children.push_back(parse_function_call_parameters_list());
		return temp;
	}
}

ASTNode Parser::parse_variable_definition_statement()
{
	ASTNode temp;
	temp.type = ASTNode::AST_VAR_DEF_EXPR;
	match_token("var");
	temp.children.push_back(parse_identifier());
	if (current_token() == "=")
	{
		match_token("=");
		temp.children.push_back(parse_expression());
	}
	else
	{
		temp.children.push_back(ASTNode());
	}
	while ((current_token() == ","))
	{
		match_token();
		temp.children.push_back(parse_identifier());
		if (current_token() == "=")
		{
			match_token("=");
			temp.children.push_back(parse_expression());
		}
		else
		{
			temp.children.push_back(ASTNode());
		}
	}
	match_token(";");
	return temp;
}

ASTNode Parser::parse_pri_expr()
{
	int back_up = pos;

	if (current_token() == "[")
	{
		return parse_array_expression();
	}

	try
	{
		pos = back_up;
		return parse_lambda_expression();
	}
	catch (const ParseError& exp2)
	{
		if (exp2.get_message() == "No more tokens")
		{
			throw;
		}
		pos = back_up;
		if (current_token().type == Token::T_NUMBER)
		{
			return parse_number();
		}
		if (current_token().type == Token::T_IDENTIFIER)
		{
			return parse_identifier();
		}
		if (current_token() == "(")
		{
			match_token("(");
			ASTNode temp;
			temp.type = ASTNode::AST_PRI_EXPR;
			temp.children.push_back(parse_expression());
			match_token(")");
			return temp;
		}
	}

	throw ParseError("Unrecognized primary expression");
}

ASTNode Parser::parse_mul_expr()
{
	ASTNode temp;
	temp.type = ASTNode::AST_MUL_EXPR;
	ASTNode temp2 = parse_postfix_expr();
	temp.children.push_back(temp2);
	while (current_token() == "*" || current_token() == "/")
	{
		if (current_token() == "*")
		{
			match_token("*");
			temp.children.push_back(parse_pri_expr());
			temp.calc_flag.push_back(true);
		}
		else
		{
			match_token("/");
			temp.children.push_back(parse_pri_expr());
			temp.calc_flag.push_back(false);
		}
	}
	if (temp.children.size() == 1)
	{
		return temp2;
	}
	return temp;
}

ASTNode Parser::parse_expression()
{
	int back_up = pos;
	
	if (current_token().type == Token::T_IDENTIFIER)
	{
//		match_token();
		parse_postfix_expr();
		if (current_token() == "=" || current_token() == "+=" || current_token() == "-=" || current_token() == "*=" || current_token() == "/=")
		{
			pos = back_up;
			return parse_assign_expression();
		}		
	}
	pos = back_up;

	return parse_or_expr();
}

ASTNode Parser::parse_add_expr()
{
	ASTNode temp;
	temp.type = ASTNode::AST_ADD_EXPR;
	ASTNode temp2 = parse_mul_expr();
	temp.children.push_back(temp2);
	while (current_token() == "+" || current_token() == "-")
	{
		if (current_token() == "+")
		{
			match_token("+");
			temp.children.push_back(parse_mul_expr());
			temp.calc_flag.push_back(true);
		}
		else
		{
			match_token("-");
			temp.children.push_back(parse_mul_expr());
			temp.calc_flag.push_back(false);
		}
	}
	if (temp.children.size() == 1)
	{
		return temp2;
	}
	return temp;
}

ASTNode Parser::parse_relat_expr()
{
	ASTNode temp;
	temp.type = ASTNode::AST_RELAT_EXPR;
	ASTNode temp2 = parse_add_expr();
	temp.children.push_back(temp2);
	if (current_token() == ">" || current_token() == "<" || current_token() == ">=" || current_token() == "<=")
	{
		temp.oper = current_token().oper;
		match_token();
		temp.children.push_back(parse_add_expr());
		return temp;
	}
	return temp2;
}

ASTNode Parser::parse_eq_expr()
{
	ASTNode temp;
	temp.type = ASTNode::AST_EQ_EXPR;
	ASTNode temp2 = parse_relat_expr();
	temp.children.push_back(temp2);
	if (current_token() == "==" || current_token() == "!==" || current_token() == "!=" || current_token() == "===")
	{
		temp.oper = current_token().oper;
		match_token();
		temp.children.push_back(parse_relat_expr());
		return temp;
	}
	return temp2;
}

ASTNode Parser::parse_and_expr()
{
	ASTNode temp;
	temp.type = ASTNode::AST_AND_EXPR;
	ASTNode temp2 = parse_eq_expr();
	temp.children.push_back(temp2);
	while (current_token() == "&&")
	{
		match_token();
		temp.children.push_back(parse_eq_expr());
	}
	if (temp.children.size() == 1)
	{
		return temp2;
	}
	return temp;
}

ASTNode Parser::parse_or_expr()
{
	ASTNode temp;
	temp.type = ASTNode::AST_OR_EXPR;
	ASTNode temp2 = parse_and_expr();
	temp.children.push_back(temp2);
	while (current_token() == "||")
	{
		match_token();
		temp.children.push_back(parse_and_expr());
	}
	if (temp.children.size() == 1)
	{
		return temp2;
	}
	return temp;
}

ASTNode Parser::parse_postfix_expr()
{
	ASTNode temp = parse_pri_expr();
	while (current_token() == "(" || current_token() == "." || current_token() == "[")
	{
		if (current_token() == "(")
		{
			ASTNode temp2;
			temp2.type = ASTNode::AST_FUN_CALL_EXPR;
			temp2.children.push_back(temp);
			temp2.children.push_back(parse_function_call_parameters_list());
			temp = temp2;
		}
		else if (current_token() == ".")
		{
			match_token();
			ASTNode temp2;
			temp2.type = ASTNode::AST_MEM_EXPR;
			temp2.children.push_back(temp);
			temp2.children.push_back(parse_identifier());
			temp = temp2;
		}
		else if (current_token() == "[")
		{
			match_token();
			ASTNode temp2;
			temp2.type = ASTNode::AST_VISIT_EXPR;
			temp2.children.push_back(temp);
			temp2.children.push_back(parse_expression());
			match_token("]");
			temp = temp2;
		}
	}
	return temp;
}

ASTNode Parser::parse_for_statement()
{
	match_token("for");
	match_token("(");
	ASTNode temp;
	temp.type = ASTNode::AST_FOR_STMT;
	if (current_token() == ";")
	{
		ASTNode temp2; temp2.type = ASTNode::AST_EMPTY;
		temp.children.push_back(temp2);
		match_token(";");
	}
	else 
	{
		if (current_token() == "var")
		{
			temp.children.push_back(parse_variable_definition_statement());
		}
		else
		{
			temp.children.push_back(parse_expression());
			match_token(";");
		}
	}

	if (current_token() == ";")
	{
		ASTNode temp2; temp2.type = ASTNode::AST_EMPTY;
		temp.children.push_back(temp2);
	}
	else
	{
		temp.children.push_back(parse_expression());
	}
	match_token(";");
	if (current_token() == ")")
	{
		ASTNode temp2; temp2.type = ASTNode::AST_EMPTY;
		temp.children.push_back(temp2);
	}
	else
	{
		temp.children.push_back(parse_expression());
	}
	match_token(")");
	temp.children.push_back(parse_statement());
	return temp;
}

ASTNode Parser::parse_array_expression()
{
	ASTNode temp;
	temp.type = ASTNode::AST_ARRAY;
	match_token("[");
	if (current_token() == "]")
	{
		match_token();
		return temp;
	}
	temp.children.push_back(parse_expression());
	while ((current_token() == ","))
	{
		match_token();
		temp.children.push_back(parse_expression());
	}
	match_token("]");
	return temp;
}

ASTNode Parser::parse_test(std::string str)
{
	str = "{" + str + "}";
	auto parser = Parser(str);
	return parser.parse_block();
}

void test_for_parser()
{
	std::string str;
	while (true)
	{
		std::cout << ">>>> ";
		std::getline(std::cin, str);
		try
		{
			std::cout <<"Par> " + to_string(Parser::parse_test(str)) << std::endl;
		}
		catch (const Exception& exp)
		{
			std::cout << exp.get_message() << std::endl;
		}
	}
}

void test_for_evaluator2()
{
	std::string str;
	Scope* scope = new Scope;
	scope->define("true", new Object(true));
	scope->define("false", new Object(false));
	scope->define("null", new Object());
	std::vector<ASTNode> nodes(1000);
	int counter = 0;
	while (true)
	{
		try
		{
			str.clear();
			std::cout << ">>>> ";
			std::getline(std::cin, str);
			if (str[0] == ':')
			{
				if (str[1] == 'l')
				{					
					std::string filename = str.substr(3, str.length() - 3);
					std::ifstream in(filename);
					std::string base;
					std::string temp;
					while (std::getline(in, temp))
					{
						base += temp;
					}
					str = "{" + base + "}";
					try {
						Parser parser(str);
						nodes[counter] = parser.parse_statement();
						nodes[counter].eval(scope, nullptr);
						std::cout << "Load " + filename + " successful!" << std::endl;
					}
					catch (const Exception& exp)
					{
						std::cout << exp.get_message() << std::endl;
					}
				}
				else if (str[1] == 'q')
				{
					delete scope;
					std::cout << "Quit Successful" << std::endl;
					return;
				}
				else if (str[1] == 's')
				{
					std::cout << "Names in global scope : " << std::endl;
					for (auto var : scope->get_map())
					{
						std::cout << var.first << std::endl;
					}
				}
			}
			else 
			{
				Parser parser(str);
				nodes[counter] = parser.parse_statement();
				std::cout << "TSL> " << to_string(nodes[counter].eval(scope, nullptr)) << std::endl;
			}
		}
		catch (const Exception& exp)
		{
			std::cout << exp.get_message() << std::endl;
		}
		catch (const std::exception& exp)
		{
			std::cout << "std::exception : " << exp.what() << std::endl;
		}
		counter++;
	}
}

int main(int argc, char* argv[])
{

	test_for_evaluator2();
//	test_for_parser();
}
