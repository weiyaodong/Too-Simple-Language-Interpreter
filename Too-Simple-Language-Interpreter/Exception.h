#pragma once
#include <string>

class Exception 
{
protected:
	std::string message;
public:

	explicit Exception(std::string msg);
	virtual ~Exception();;

	virtual std::string get_message() const;
};

class TokenizeError : public Exception
{
public:
	explicit TokenizeError(const std::string& msg);

	std::string get_message() const override;
};

class ParseError: public Exception
{
public:
	explicit ParseError(const std::string& msg);

	std::string get_message() const override;
};

class RunTimeError : public Exception
{
public:
	explicit RunTimeError(const std::string& str);

	std::string get_message() const override;
};

class TypeError : public RunTimeError
{
	std::string expected_type;
	std::string actual_type;
public:
	TypeError(const std::string& e_type, const std::string& a_type);

	std::string get_message() const override;
};

class SubscriptOutOfRange : public RunTimeError
{
	std::string a, b;
public:
	explicit SubscriptOutOfRange(const std::string& a, const std::string& b);

	std::string get_message() const override;
};

class LoopInterrupt
{
	int flag;
public:
	explicit LoopInterrupt(int flag);
	int get_flag() const;
};
