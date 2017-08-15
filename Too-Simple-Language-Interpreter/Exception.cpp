#include "Exception.h"

Exception::Exception(std::string msg): message(msg)
{
}

std::string Exception::get_message() const
{
	return message;
}
