#include "Exception.h"

Exception::Exception(std::string msg): message(msg)
{
}

std::string Exception::get_message() const
{
	return message;
}

LoopInterrupt::LoopInterrupt(int flag) : flag(flag)
{
}

int LoopInterrupt::get_flag() const
{
	return flag;
}
