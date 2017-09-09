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
		AST_EMPTY,
		AST_NUM,			// number
		AST_CHAR,			// character todo
		AST_STRING,			// string todo
		AST_ARRAY,			// array
		
		AST_IDENT,			// identifier
		AST_STMT,			// statement
		AST_VAR_DEF_EXPR,	// variable definition
		AST_MUL_EXPR,		// multiplicative expression
		AST_ADD_EXPR,		// additive expression
		
		AST_RELAT_EXPR,		// relation expression
		AST_EQ_EXPR,		// equality expression
		AST_AND_EXPR,		// logic and expression
		AST_OR_EXPR,		// logic or expression
		AST_PRI_EXPR,		// primary expression
		
//		AST_POST_EXPR,		// postfix expression 
		AST_MEM_EXPR,		// member expression
		AST_VISIT_EXPR,		// visit expression
		AST_ASN_EXPR,		// assignment expression
		AST_EXPR,			// expression
		
		AST_BLOCK,			// { block }
		AST_IF_STMT,		// if statement
		AST_PRINT_STMT,		// print statement
		AST_READ_STMT,		// read statement
		AST_WHILE_STMT,		// while statement
		
		AST_FOR_STMT,		// for statement
		AST_BREAK_STMT,		// break statement
		AST_CONTINUE_STMT,	// continue statement
		AST_LAMBDA_EXPR,	// lambda expression
		AST_FUN_DEF_STMT,	// function declaration expression
		
		AST_FUN_CALL_EXPR,	// function call expression
		AST_RET_STMT,		// return statement
		AST_FUN_CALL_PARA_LIST, // function call parameters list
		AST_FUN_DEF_PARA_LIST,	// function definition parameters list
	};

	ASTNode();

	explicit ASTNode(int val);

	ASTNode(ASTNode_Type t, const std::string& n);

	ASTNode(const ASTNode& ast);
	ASTNode& operator =(const ASTNode& ast);

	int num;
	std::vector<ASTNode> children;
	std::vector<bool> calc_flag;
	std::string oper;
	std::string name;
	ASTNode_Type type;

	Object eval(Scope* scope, const Object* current_fun) const;
};

template<>
std::string to_string(const ASTNode& node);


class Parser
{
	std::vector<Token> tokens;
	size_t pos = 0;

public:

	explicit Parser(std::string code);

	Token current_token() const;

	Token next_token();

	void match_token();

	void match_token(std::string str);

	void match_token(Token::Token_Type type);

	ASTNode parse_number();

	ASTNode parse_identifier();

	ASTNode parse_block();

	ASTNode parse_function_definition_parameters_list();

	ASTNode parse_function_definition_statement();

	ASTNode parse_statement();

	ASTNode parse_assign_expression();

	ASTNode parse_if_statement();

	ASTNode parse_print_statement();

	ASTNode parse_read_statement();

	ASTNode parse_while_statement();

	ASTNode parse_break_statement();

	ASTNode parse_continue_statement();

	ASTNode parse_return_statement();

	ASTNode parse_function_call_parameters_list();

	ASTNode parse_lambda_expression();

	ASTNode parse_function_call();

	ASTNode parse_variable_definition_statement();

	ASTNode parse_pri_expr();

	ASTNode parse_mul_expr();

	ASTNode parse_expression();

	ASTNode parse_add_expr();

	ASTNode parse_relat_expr();

	ASTNode parse_eq_expr();

	ASTNode parse_and_expr();

	ASTNode parse_or_expr();

	ASTNode parse_postfix_expr();

	ASTNode parse_for_statement();

	ASTNode parse_array_expression();

	static ASTNode parse_test(std::string str);
};

void test_for_parser();

void repl();

void subscript_assert(int a, int b);

class ObjectIdentity
{
public:

	ObjectIdentity(const std::string& name, Scope* scope) : flag(false), scope(scope), name(name), obj(nullptr)
	{
	}

	ObjectIdentity(Object* obj) : flag(true), scope(nullptr), obj(obj)
	{
	}

	bool flag; // true for array, false for object
	Scope* scope;
	std::string name;

	Object* obj;

};