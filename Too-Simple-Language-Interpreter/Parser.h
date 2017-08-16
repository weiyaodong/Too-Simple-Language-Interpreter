#pragma once
#include "Tokenizer.h"
#include "Evaluator.h"
#include <functional>
#include <vector>

class Object;
class Scope;

class ASTNode // AST for Abstract Syntax Tree Node
{
public:
	enum ASTNode_Type
	{
		AST_NUM,			// number
		AST_IDENT,			// identifier
		AST_STMT,			// statement
		AST_VAR_DEF_EXPR,		// variable definition
		AST_EXPR,			// arithmetic expression
		AST_TERM,			// arithmetic term
//		AST_ARITH_FACTOR,	// arithmetic factor
		AST_ASN_EXPR,		// assignment expression
		AST_STMTS,			// { statements }
		AST_IF_STMT,		// if statement
		AST_WHILE_STMT,		// while statement
		AST_LAMBDA_EXPR,	// lambda expression
		AST_FUN_DEF_STMT,	// function declaration expression
		AST_FUN_CALL_EXPR,	// function call expression
		AST_RET_STMT,		// return statement
		AST_FUN_CALL_PARA_LIST, // function call parameters list
		AST_FUN_DEF_PARA_LIST,	// function definition parameters list
	};

	// ReSharper disable once CppPossiblyUninitializedMember
	ASTNode() {}

	ASTNode(int val) : num(val)
	{
		type = AST_NUM;
	}
	// todo
	// ReSharper disable once CppPossiblyUninitializedMember
	ASTNode(ASTNode_Type t, std::string n) : type(t)
	{
		if (type == AST_IDENT)
		{
			name = n;
		}
	}

	ASTNode(const ASTNode& ast)
	{
		num = ast.num;
		children = ast.children;
		calc_flag = ast.calc_flag;
		name = ast.name;
		type = ast.type;
	}

	int num;
	std::vector<ASTNode> children;
	std::vector<bool> calc_flag;
	std::string name;
	ASTNode_Type type;

	Object eval(Scope* scope) const;
};

template<>
std::string to_string(const ASTNode& node);


class Parser
{
	std::vector<Token> tokens;
	size_t pos = 0;

public:

	/**
	* \brief The default constructor of Parser class
	* \param code the std::string format code data
	*/
	explicit Parser(std::string code);

	Token current_token() const;

	Token next_token();

	void match_token();

	void match_token(std::string str);

	void match_token(Token::Token_Type type);

	ASTNode parse_number();

	/// <summary>
	/// To parse identifier \n
	/// </summary>
	ASTNode parse_identifier();

	ASTNode parse_statements();

	ASTNode parse_function_definition_parameters_list();

	/// <summary>
	/// To parse function definition statement \n
	/// \n
	/// fun_def_stmt ::= "fun" identifier fun_def_para_list "=>" stmts
	/// </summary>

	ASTNode parse_function_definition_statement();

	/// <summary>
	/// To parse several statements \n
	/// BNF: \n
	/// stmts ::= {stmt}
	/// </summary>

	ASTNode parse_statement();

	/// <summary>
	/// To parse assign expression \n
	/// BNF: \n
	/// asn_expr ::= identifier "=" expr
	/// </summary>
	ASTNode parse_assign_expression();

	/// <summary>
	/// To parse if statement \n
	/// BNF: \n
	/// if_stmt ::= "if" "(" expr ")" stmt
	/// </summary>
	ASTNode parse_if_statement();

	/// <summary>
	/// To parse while statement \n
	/// BNF: \n
	/// if_stmt ::= "while" "(" expr ")" stmt
	/// </summary>
	ASTNode parse_while_statement();

	/// <summary>
	/// To parse return statement \n
	/// BNF: \n
	/// if_stmt ::= "ret" expr ";"
	/// </summary>
	ASTNode parse_return_statement();

	ASTNode parse_function_call_parameters_list();

	ASTNode parse_lambda_expression();

	ASTNode parse_function_call();

	ASTNode parse_variable_definition_statement();

	ASTNode parse_factor();

	/// <summary>
	/// To parse arithmetic term \n
	/// BNF: \n
	/// term ::= factor {"*" term} \n
	///		   | factor {"/" term}
	/// </summary>
	ASTNode parse_term();

	/// <summary>
	/// To parse expression \n
	/// BNF: \n
	/// expr ::= asn_expr\n
	///		   | term {"+" term}
	/// </summary>
	ASTNode parse_expression();

	static ASTNode parse_test(std::string str);
};

void test_for_parser();

void test_for_evaluator();
