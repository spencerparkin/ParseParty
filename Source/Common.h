#pragma once

#if defined PARSE_PARTY_EXPORT
#	define PARSE_PARTY_API		__declspec(dllexport)
#elif defined PARSE_PARTY_IMPORT
#	define PARSE_PARTY_API		__declspec(dllimport)
#else
#	define PARSE_PARTY_API
#endif

#include <list>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <fstream>
#include <sstream>