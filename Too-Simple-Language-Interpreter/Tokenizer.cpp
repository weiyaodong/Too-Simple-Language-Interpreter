#include "Tokenizer.h"
#include <iostream>

template <>
std::string to_string<Exception>(const Exception& exp)
{
	return exp.get_message();
}

int to_int(const std::string str)
{
	std::stringstream ss(str);
	int val;
	if (ss >> val)
	{
		return val;
	}
	throw TokenizeError("Can't convert " + str + " to int!");
}

Token::Token(): type(T_EMPTY), num(0)
{
}

Token::Token(int val): type(T_NUMBER), num(val)
{
}

Token::Token(Token_Type t, const std::string& s): type(t)
{
	switch (type)
	{
	case T_KEYWORD: kword = s;
		break;
	case T_BRACKET: bkt = s;
		break;
	case T_IDENTIFIER: ident = s;
		break;
	case T_OPERATOR: oper = s;
		break;
	case T_STRING: str = s;
		break;
	case T_CHAR: chr = s;
		break;
	case T_NUMBER: num = to_int(s);
		break;
	default: throw TokenizeError("Empty token can't have value!");
	}
}

Token::Token(const Token& tok)
{
	this->type = tok.type;
	switch (type)
	{
	case T_KEYWORD: kword = tok.kword;
		break;
	case T_BRACKET: bkt = tok.bkt;
		break;
	case T_IDENTIFIER: ident = tok.ident;
		break;
	case T_OPERATOR: oper = tok.oper;
		break;
	case T_STRING: str = tok.str;
		break;
	case T_CHAR: chr = tok.chr;
		break;
	case T_NUMBER: num = tok.num;
		break;
	default: ;
	}
}

Token& Token::operator=(const Token& tok)
{
	this->type = tok.type;
	switch (type)
	{
	case T_KEYWORD: kword = tok.kword;
		break;
	case T_BRACKET: bkt = tok.bkt;
		break;
	case T_IDENTIFIER: ident = tok.ident;
		break;
	case T_OPERATOR: oper = tok.oper;
		break;
	case T_STRING: str = tok.str;
		break;
	case T_CHAR: chr = tok.chr;
		break;
	case T_NUMBER: num = tok.num;
		break;
	default: ;
	}
	return *this;
}

Token::~Token()
{
}

::Token Token::empty_token()
{
	return Token();
}

bool Token::operator==(const Token& tok) const
{
	if (type != tok.type)
	{
		return false;
	}
	switch (type)
	{
	case T_EMPTY: return true;
	case T_KEYWORD: return kword == tok.kword;
	case T_BRACKET: return bkt == tok.bkt;
	case T_IDENTIFIER: return ident == tok.ident;
	case T_OPERATOR: return oper == tok.oper;
	case T_STRING: return str == tok.str;
	case T_CHAR: return chr == tok.chr;
	case T_NUMBER: return num == tok.num;
	default: return false;
	}
}

std::string Token::get_str() const
{
	switch (type)
	{
	case T_EMPTY: return ("Empty TOKEN!");
	case T_KEYWORD: return kword;
	case T_BRACKET: return bkt;
	case T_IDENTIFIER: return ident;
	case T_OPERATOR: return oper;
	case T_STRING: return str;
	case T_CHAR: return chr;
	default: return to_string(num);
	}
}

bool Token::operator==(const std::string& str) const
{
	switch (type)
	{
	case T_EMPTY: return false;
	case T_KEYWORD: return kword == str;
	case T_BRACKET: return bkt == str;
	case T_IDENTIFIER: return ident == str;
	case T_OPERATOR: return oper == str;
	case T_STRING: return this->str == str;
	case T_CHAR: return chr == str;
	default: return false;
	}
}

bool Token::operator==(const data_type& val) const
{
	switch (type)
	{
	case T_NUMBER: return num == val;
	case T_EMPTY: return false;
	default: return false;
	}
}

template <>
std::string to_string<Token::Token_Type>(const Token::Token_Type& ttype)
{
	switch (ttype)
	{
	case Token::T_EMPTY: return("Empty TOKEN!");
	case Token::T_NUMBER: return "number";
	case Token::T_KEYWORD: return "keyword";
	case Token::T_IDENTIFIER: return "id";
	case Token::T_STRING: return "string";
	case Token::T_CHAR: return "char";
	case Token::T_OPERATOR: return "operator";
	case Token::T_BRACKET: return "bracket";
	default: throw TokenizeError("to_string<TokenType> :: What the fuck???");
	}
}

template <>
std::string to_string<Token>(const Token& tok)
{
	std::string ans = "<";
	ans += to_string(tok.type);
	ans += " : ";
	if (tok.type != Token::T_NUMBER)
	{
		ans += "'";
		ans += tok.get_str();
		ans += "'";
	}
	else
	{
		ans += tok.get_str();
	}
	ans += ">";
	return ans;
}

std::string keywords[] = {
	"for", 
	"while",
	"break",
	"continue",
	"if",
	"else",
	"var",
	"fun", 
	"ret",
	"print",
	"read" // todo
};

std::string operators[] = {
	"/*",
	"*/",
	"+",
	"-",
	"*",
	"/",
	"=",
	"+=",
	"-=",
	"*=",
	"/=",
	"&&",
	"||",
	"==",
	"===",
	"!=",
	"!==",
	">",
	"<",
	">=",
	"<=",
	"++",
	"=>",
	";",
	",",
	"."
};

bool is_keyword(const std::string& str)
{
	for (const auto& keyword : keywords)
	{
		if (str == keyword)
		{
			return true;
		}
	}
	return false;
}

auto is_symbol_head = [](char c) -> bool {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
};

auto is_symbol_tail = [](char c) -> bool {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || ('0' <= c && c <= '9');
};

bool is_oper(const std::string& str)
{
	for (const auto& oper : operators)
	{
		if (str == oper)
		{
			return true;
		}
	}
	return false;
}

bool is_digit(char c)
{
	return oneOf("0123456789")(c);
}

bool is_space(char c)
{
	return oneOf(" \t\n\r")(c);
}

bool is_operator(char c)
{
	return oneOf("<>=!&|-+*/^;,.")(c);
}

bool is_bracket(char c)
{
	return oneOf("{}[]()")(c);
}

TokenStream::TokenStream(const std::string& str): code(str), pos(0)
{
	next_token();
}

Token TokenStream::current_token() const
{
	return cur;
}

char TokenStream::cur_char() const
{
	if (pos >= code.length())
	{
		return EOF;
	}
	return code.at(pos);
}

void TokenStream::match_char(char c)
{
	if (cur_char() == c)
	{
		pos++;
	}
	else
	{
		throw TokenizeError("Expected: '" + to_string(c) + "', but got " + to_string(code[pos]));
	}
}

void TokenStream::reset()
{
	pos = 0;
}

std::string TokenStream::get_code() const
{
	return code;
}

void TokenStream::match_char(const std::function<bool(char)>& fun)
{
	if (fun(cur_char()))
	{
		pos++;
	}
	else
	{
		throw TokenizeError("Got: " + to_string(code[pos]));
	}
}

Token TokenStream::parseNumber()
{
	int start = pos;
	match_char(is_digit);
	while (is_digit(cur_char()))
	{
		pos++;
	}
	return cur = Token(Token::T_NUMBER, code.substr(start, pos - start));
}

Token TokenStream::parseString()
{
	std::string result;
	match_char('"');
	while (cur_char() != EOF && cur_char() != '"')
	{
		if (cur_char() == '\\')
		{
			match_char('\\');
			switch (cur_char())
			{
			case 'n': result += "\n";
				break;
			case 't': result += "\t";
				break;
			case 'r': result += "\r";
				break;
			case '\\': result += "\\";
				break;
			case '\"': result += "\"";
				break;
			case '\'': result += "\'";
				break;
			default: throw TokenizeError("Unrecognized character: " + code.substr(pos - 1, 2));
			}
		}
		else
		{
			result += to_string(cur_char());
			pos++;
		}
	}
	match_char('"');
	return cur = Token(Token::T_STRING, result);
}

Token TokenStream::parseChar()
{
	std::string result;
	match_char('\'');
	if (cur_char() == '\\')
	{
		match_char('\\');
		switch (cur_char())
		{
		case 'n': match_char('n');
			result = "\n";
			break;
		case 't': match_char('t');
			result = "\t";
			break;
		case 'r': match_char('r');
			result = "\r";
			break;
		case '\\': match_char('\\');
			result = "\\";
			break;
		case '\"': match_char('\"');
			result = "\"";
			break;
		case '\'': match_char('\'');
			result = "\'";
			break;
		default: throw TokenizeError("Unrecognized character: " + code.substr(pos - 1, 2));
		}
	}
	else if (cur_char() == EOF)
	{
		throw TokenizeError("No more tokens after \'");
	}
	else
	{
		result = to_string(cur_char());
		pos++;
	}
	match_char('\'');
	return cur = Token(Token::T_CHAR, result);
}

void TokenStream::parseSpace()
{
	while (is_space(cur_char()))
	{
		pos++;
	}
}

Token TokenStream::parseOperator()
{
	int start = pos;
	match_char(is_operator);
	while (is_operator(cur_char()))
	{
		match_char(is_operator);
	}
	std::string oper = code.substr(start, pos - start);
	if (is_oper(oper))
	{
		return cur = Token(Token::T_OPERATOR, oper);
	}
	throw TokenizeError("Unrecognized operator " + oper);
}

Token TokenStream::parseBracket()
{
	auto temp = pos;
	match_char(is_bracket);
	return cur = Token(Token::T_BRACKET, code.substr(temp, 1));
}

Token TokenStream::parseSymbol()
{
	int start = pos;
	match_char(is_symbol_head);
	while (is_symbol_tail(cur_char()))
	{
		match_char(is_symbol_tail);
	}
	auto symbol1 = code.substr(start, pos - start);
	std::string symbol = symbol1;
	if (is_keyword(symbol))
	{
		return cur = Token(Token::T_KEYWORD, symbol);
	}
	return cur = Token(Token::T_IDENTIFIER, symbol);
}

Token TokenStream::next_token()
{
	if (cur_char() == EOF)
	{
		return cur = Token::empty_token();
	}
	if (is_space(cur_char()))
	{
		parseSpace();
	}
	if (is_digit(cur_char()))
	{
		return parseNumber();
	}
	if (is_symbol_head(cur_char()))
	{
		return parseSymbol();
	}
	if (is_operator(cur_char()))
	{
		return parseOperator();
	}
	if (cur_char() == '\'')
	{
		return parseChar();
	}
	if (cur_char() == '\"')
	{
		return parseString();
	}
	if (is_bracket(cur_char()))
	{
		return parseBracket();
	}
	return cur = Token::empty_token();
}

std::vector<Token> TokenStream::retrieve_all_tokens()
{
	std::vector<Token> result;
	Token temp;
	while (!((temp = current_token()) == Token::empty_token()))
	{
		result.push_back(temp);
		next_token();
	}
	return result;
}

void TokenStream::match_token(std::string str)
{
	if (cur == str)
	{
		next_token();
	}
}


void test_for_tokenizer()
{
	std::string str;

	while (true)
	{
		std::getline(std::cin, str);
		try {
			TokenStream ts = TokenStream(str);
			while (!(ts.current_token() == Token::empty_token()))
			{
				std::cout << to_string(ts.current_token()) << " ";
				ts.next_token();
			}
			std::cout << std::endl;
		}
		catch (const Exception& exp)
		{
			std::cout << exp.get_message() << std::endl;
		}
		catch (...)
		{
			std::cout << "something happened ..." << std::endl;
		}
	}
}