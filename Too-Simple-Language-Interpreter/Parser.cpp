#include "Parser.h"
#include <iostream>

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
	if (node.type == ASTNode::AST_ARITH_FACTOR)
	{
		if (node.children[0].type == ASTNode::AST_EXPR)
		{
			return "(" + to_string(node.children[0]) + ")";
		}
		return to_string(node.children[0]);
	}
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
		return "return " + to_string(node.children[0]);
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

ASTNode Parser::parse_lambda_expr()
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
		temp.children.push_back(parse_lambda_expr());
		match_token(")");
		temp.children.push_back(parse_function_call_parameters_list());
		return temp;
	}
}

ASTNode Parser::parse_arithmetic_factor()
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
			return parse_lambda_expr();
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
	temp.children.push_back(parse_arithmetic_factor());
	while (current_token() == "*" || current_token() == "/")
	{
		if (current_token() == "*")
		{
			match_token("*");
			temp.children.push_back(parse_arithmetic_factor());
			temp.calc_flag.push_back(true);
		}
		else
		{
			match_token("/");
			temp.children.push_back(parse_arithmetic_factor());
			temp.calc_flag.push_back(false);
		}
	}
	return temp;
}

ASTNode Parser::parse_expression()
{
	int back_up = pos;
	ASTNode temp;
	temp.type = ASTNode::AST_EXPR;
	if (current_token().type == Token::T_IDENTIFIER && next_token() == "=")
	{
		pos = back_up;
		temp.children.push_back(parse_assign_expression());
		return temp;
	}
	pos = back_up;
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
