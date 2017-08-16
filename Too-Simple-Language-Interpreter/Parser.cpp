#include "Parser.h"
#include <iostream>

Object ASTNode::eval(Scope* scope) const
{
	if (type == AST_TERM)
	{
		if (children.size() == 1)
		{
			return children[0].eval(scope);
		}
		std::vector<int> num;
		for (size_t i = 0; i < children.size(); i++)
		{
			Object temp = children[i].eval(scope);
			if (temp.type != Object::NUMBER)
			{
				// todo
				// throw Exception(to_string(temp) + " is not a Number");
				throw Exception("Only Numbers can be calculated now");
			}
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
					throw Exception("Devide by zero");
				}
				ans /= num[i + 1];
			}
		}
		return Object(Object::NUMBER, ans);
	}
	if (type == AST_EXPR)
	{
		if (children.size() == 1)
		{
			return children[0].eval(scope);
		}
		std::vector<int> num;
		for (size_t i = 0; i < children.size(); i++)
		{
			Object temp = children[i].eval(scope);
			if (temp.type != Object::NUMBER)
			{
				// todo
				// throw Exception(to_string(temp) + " is not a Number");
				throw Exception("Only Numbers can be calculated now");
			}
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
	if (type == AST_NUM)
	{
		return Object(Object::NUMBER, num);
	}
	if (type == AST_IDENT)
	{
		return *scope->find(name); // caution
	}
	if (type == AST_VAR_DEF_EXPR)
	{
		if (children.size() == 1)
		{
			scope->define(children[0].name, new Object());
		}
		if (children.size() == 2)
		{
			scope->define(children[0].name, new Object(children[1].eval(scope)));
		}
		return Object();
	}
	if (type == AST_LAMBDA_EXPR)
	{
		Object temp;
		temp.type = Object::FUNCTION;
		temp.scope = std::make_shared<Scope>(scope);
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
		temp->scope = std::make_shared<Scope>(scope);
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
		Object fun = children[0].eval(scope);          
		if (fun.type != Object::FUNCTION)
		{
			throw Exception(" <" + to_string(children[0]) + "> is not a function");
		}
		if (children[1].children.size() > fun.parameters.size())
		{
			throw Exception("Too many arguments for function : " + to_string(fun));
		}
		std::vector<Object> param;
		for (size_t i = 0; i < children[1].children.size(); i++)
		{
			param.push_back(children[1].children[i].eval(scope));
		}
		if (fun.body->type != AST_STMTS)
		{
			return fun.evaluate(param);
		}
		try
		{
			return fun.evaluate(param);
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
	if (type == AST_IF_STMT) // todo
	{
		auto temp = children[0].eval(scope);
		if (temp.type == Object::NUMBER && temp.num == 0)
		{
			return Object();
		}
		return children[1].eval(scope);
	}
	if (type == AST_RET_STMT)
	{
		throw children[0].eval(scope);
	}
	if (type == AST_ASN_EXPR)
	{
		return *scope->modify(children[0].name, new Object(children[1].eval(scope)));
	}
	if (type == AST_WHILE_STMT)
	{
		auto temp = children[0].eval(scope);
		Object result;
		while (temp.type == Object::NUMBER && temp.num == 0)
		{
			try
			{
				result = children[1].eval(scope);
			}
			catch (Exception exp)
			{
				throw;
			}
			catch (...)
			{
				// todo
				// deal with "continue" and "break"
			}
		}
		return result;
	}
	if (type == AST_STMTS)
	{
		Object temp;
		for (size_t i = 0; i < children.size(); i++)
		{
			temp = children[i].eval(scope);
		}
		return temp;
	}
	if (type == AST_STMT)
	{
		return children[0].eval(scope);
	}
	else
	{
		throw Exception("what the fuck ?");
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
	if (node.type == ASTNode::AST_EXPR)
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
	if (node.type == ASTNode::AST_TERM)
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
//	if (node.type == ASTNode::AST_ARITH_FACTOR)
//	{
//		if (node.children[0].type == ASTNode::AST_EXPR)
//		{
//			return "(" + to_string(node.children[0]) + ")";
//		}
//		return to_string(node.children[0]);
//	}
	if (node.type == ASTNode::AST_FUN_CALL_EXPR)
	{
		if (node.children[0].type == ASTNode::AST_IDENT)
		{
			return to_string(node.children[0]) + to_string(node.children[1]);
		}
		return "(" + to_string(node.children[0]) + ")" + to_string(node.children[1]);
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
	if (node.type == ASTNode::AST_STMTS)
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
		return "if (" + to_string(node.children[0]) + ")" + to_string(node.children[1]);
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
	if (node.type == ASTNode::AST_STMT)
	{
		if (node.children[0].type == ASTNode::AST_EXPR)
		{
			return to_string(node.children[0]) + ";";
		}
		return to_string(node.children[0]);
	}
	if (node.type == ASTNode::AST_ASN_EXPR)
	{
		return to_string(node.children[0]) + " = " + to_string(node.children[1]);
	}
	if (node.type == ASTNode::AST_VAR_DEF_EXPR)
	{
		if (node.children.size() == 1)
		{
			return "var " + to_string(node.children[0]) + ";";
		}
		return "var " + to_string(node.children[0]) + " = " + to_string(node.children[1]) + ";";
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
	throw Exception("Can't match " + to_string(current_token()) + " with " + str);
}

void Parser::match_token(Token::Token_Type type)
{
	if (current_token().type == type)
	{
		next_token();
	}
	throw Exception("Can't match " + to_string(current_token()) + " with type: " + to_string(type));
}

ASTNode Parser::parse_number()
{
	if (current_token().type == Token::T_NUMBER)
	{
		auto temp = ASTNode(current_token().num);
		next_token();
		return temp;
	}
	throw Exception("Current token is not a number");
}

ASTNode Parser::parse_identifier()
{
	if (current_token().type == Token::T_IDENTIFIER)
	{
		auto temp = ASTNode(ASTNode::AST_IDENT, current_token().ident);
		next_token();
		return temp;
	}
	throw Exception("Current token is not an identifier");
}

ASTNode Parser::parse_statements()
{
	ASTNode temp;
	temp.type = ASTNode::AST_STMTS;
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
		throw Exception("Unexpected token:" + to_string(current_token()));
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
	temp.children.push_back(parse_statements());
	return temp;
}

ASTNode Parser::parse_statement()
{
	if (current_token() == "{")
	{
		return parse_statements();
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
	temp.children.push_back(parse_identifier());
	match_token("=");
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
		throw Exception("Unexpected token:" + to_string(current_token()));
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
		temp.children.push_back(parse_statements());
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
//		temp.children.push_back(parse_lambda_expression());
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
	match_token(";");
	return temp;
}

ASTNode Parser::parse_factor()
{
	int back_up = pos;
	try
	{
		return parse_function_call();
	}
	catch (Exception exp)
	{
		if (exp.get_message() == "No more tokens")
		{
			throw;
		}
		try
		{
			pos = back_up;
			return parse_lambda_expression();
		}
		catch (Exception exp2)
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
				auto temp = parse_expression();
				match_token(")");
				return temp;
			}
		}
	}
	throw Exception("What the fuck???");
}

ASTNode Parser::parse_term()
{
	ASTNode temp;
	temp.type = ASTNode::AST_TERM;
	temp.children.push_back(parse_factor());
	while (current_token() == "*" || current_token() == "/")
	{
		if (current_token() == "*")
		{
			match_token("*");
			temp.children.push_back(parse_factor());
			temp.calc_flag.push_back(true);
		}
		else
		{
			match_token("/");
			temp.children.push_back(parse_factor());
			temp.calc_flag.push_back(false);
		}
	}
	return temp;
}

ASTNode Parser::parse_expression()
{
	int back_up = pos;
	
	if (current_token().type == Token::T_IDENTIFIER && next_token() == "=")
	{
		pos = back_up;
		return parse_assign_expression();
	}
	pos = back_up;
	ASTNode temp;
	temp.type = ASTNode::AST_EXPR;
	temp.children.push_back(parse_term());
	while (current_token() == "+" || current_token() == "-")
	{
		if (current_token() == "+")
		{
			match_token("+");
			temp.children.push_back(parse_term());
			temp.calc_flag.push_back(true);
		}
		else
		{
			match_token("-");
			temp.children.push_back(parse_term());
			temp.calc_flag.push_back(false);
		}
	}
	return temp;
}

ASTNode Parser::parse_test(std::string str)
{
	str = "{" + str + "}";
	auto parser = Parser(str);
	return parser.parse_statements();
}

void test_for_parser()
{
	std::string str;
	while (true)
	{
		std::getline(std::cin, str);
		try
		{
			std::cout << to_string(Parser::parse_test(str)) << std::endl;
		}
		catch (Exception exp)
		{
			std::cout << exp.get_message() << std::endl;
		}
	}
}

void test_for_evaluator()
{
	std::string str;
	Scope* scope = new Scope;
	std::vector<ASTNode> nodes(1000);
	int counter = 0;
	while (true)
	{
		str.clear();
		std::cout << ">>>> ";
		std::string temp;
		std::cin >> temp;
		while (temp != "end")
		{
			str += temp+" ";
			std::cin >> temp;
		}
//		std::getline(std::cin, str);
		Parser parser(str);
		try
		{
			nodes[counter] = parser.parse_statement();
//			std::cout << "P: " << to_string(nodes[counter]) << std::endl;
			std::cout << "TSL> " << to_string(nodes[counter].eval(scope)) << std::endl;
		}
		catch(Exception exp)
		{
			std::cout << "Error : " << exp.get_message() << std::endl;
		}
		counter++;
	}
}

int main(int argc, char* argv[])
{
	test_for_evaluator();
}
