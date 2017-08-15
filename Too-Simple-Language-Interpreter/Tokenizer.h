#pragma once

#include <string>
#include <sstream>
#include "Exception.h"
#include <functional>
#include <vector>

template<typename T>
std::string to_string(const T& var)
{
	std::stringstream ss;
	std::string result;
	ss << var;
	ss >> result;
	return result;
}

template <>
std::string to_string<Exception>(const Exception& exp);

int to_int(const std::string str);

class Token
{
public:

	enum Token_Type
	{
		T_NUMBER,
		T_KEYWORD,
		T_IDENTIFIER,
		T_STRING,
		T_CHAR,
		T_OPERATOR,
		T_BRACKET,
		T_EMPTY
	};

	typedef long long data_type;

	Token_Type type;

	std::string str;
	std::string kword;
	std::string ident;
	std::string oper;
	std::string bkt;
	std::string chr;
	data_type num;


	explicit Token();

	explicit Token(int val);

	explicit Token(Token_Type t, const std::string& s);

	Token(const Token& tok);

	Token& operator =(const Token& tok);

	~Token();

	static Token empty_token();

	bool operator ==(const Token& tok) const;

	std::string get_str() const;

	bool operator ==(const std::string& str) const;

	bool operator ==(const data_type& val) const;
};

template<>
std::string to_string(const Token::Token_Type& ttype);

template<>
std::string to_string(const Token& tok);



bool is_keyword(const std::string& str);

bool is_oper(const std::string& str);

inline auto oneOf(const std::string& str)
{
	return [&](char c) -> bool
	{
		for(const char& c2 : str)
		{
			if (c == c2)
			{
				return true;
			}
		}
		return false;
	};
}

inline auto noneOf(const std::string& str)
{
	return [&](char c) -> bool {
		for (const char& c2 : str)
		{
			if (c == c2)
			{
				return false;
			}
		}
		return true;
	};
}

bool is_digit(char c);

bool is_space(char c);

bool is_operator(char c);

bool is_bracket(char c);


class TokenStream
{
	std::string code;
	size_t pos;
	Token cur;
public:

	explicit TokenStream(const std::string& str);

	Token current_token() const;

	char cur_char() const;

	void match_char(char c);

	void reset();

	std::string get_code() const;

	void match_char(const std::function<bool(char)>& fun);

	Token parseNumber();

	Token parseString();

	Token parseChar();

	void parseSpace();

	Token parseOperator();

	Token parseBracket();

	Token parseSymbol();

	Token next_token();

	std::vector<Token> retrieve_all_tokens();

	void match_token(std::string str);
};