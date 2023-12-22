#pragma once

#include <string>

namespace ParseParty
{
	// I was using std:format(...), but I ran into a situation where I wanted
	// to use the parser, but on a system where C++20 was not available.
	std::string FormatString(const char* format, ...);
}