#pragma once
#include <string>

class Exception 
{
	std::string message;
public:

	explicit Exception(std::string msg);

	std::string get_message() const;
};

class LoopInterrupt
{
	int flag;
public:
	explicit LoopInterrupt(int flag);
	int get_flag() const;
};
