#include "FormatString.h"
#include <stdarg.h>

namespace ParseParty
{
	std::string FormatString(const char* format, ...)
	{
		va_list argList;
		char buffer[512];
		va_start(argList, format);
		vsprintf(buffer, format, argList);
		va_end(argList);
		return std::string(buffer);
	}
}