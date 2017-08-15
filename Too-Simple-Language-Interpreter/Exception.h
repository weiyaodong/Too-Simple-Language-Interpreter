#pragma once
#include <string>

class Exception 
{
	std::string message;
public:

	explicit Exception(std::string msg);

	std::string get_message() const;
};
