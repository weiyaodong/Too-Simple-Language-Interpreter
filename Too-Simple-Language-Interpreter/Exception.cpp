#include "Exception.h"

Exception::Exception(std::string msg): message(msg)
{
}

Exception::~Exception()
{
}

std::string Exception::get_message() const
{
	return message;
}

TokenizeError::TokenizeError(const std::string& msg): Exception(msg)
{
}

std::string TokenizeError::get_message() const
{
	return "Tokenize Error : " + message;
}

ParseError::ParseError(const std::string& msg): Exception(msg)
{
}

std::string ParseError::get_message() const
{
	return "Parse Error : " + message;
}

RunTimeError::RunTimeError(const std::string& str): Exception(str)
{
}

std::string RunTimeError::get_message() const
{
	return "Run Time Error : " + message;
}

TypeError::TypeError(const std::string& e_type, const std::string& a_type): RunTimeError(""), expected_type(e_type), actual_type(a_type)
{
}

std::string TypeError::get_message() const
{
	return "Type Error : Expected " + expected_type + " , got " + actual_type;
}

LoopInterrupt::LoopInterrupt(int flag) : flag(flag)
{
}

int LoopInterrupt::get_flag() const
{
	return flag;
}
