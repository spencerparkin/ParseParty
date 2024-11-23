#include "StringTransformer.h"

using namespace ParseParty;

//---------------------- StringTransformer ----------------------

StringTransformer::StringTransformer()
{
}

/*virtual*/ StringTransformer::~StringTransformer()
{
}

//---------------------- EspaceSequenceEncoder ----------------------

EspaceSequenceEncoder::EspaceSequenceEncoder()
{
}

/*virtual*/ EspaceSequenceEncoder::~EspaceSequenceEncoder()
{
}

/*virtual*/ bool EspaceSequenceEncoder::Transform(const std::string& inString, std::string& outString)
{
	if (&inString == &outString)
		return false;

	outString = "";

	for (int i = 0; inString.c_str()[i] != '\0'; i++)
	{
		char ch = inString.c_str()[i];
		if (ch != '\\')
			outString += ch;
		else
		{
			char nextCh = inString.c_str()[++i];
			if (nextCh == '\0')
				return false;

			switch (nextCh)
			{
			case 't':
				outString += '\t';
				break;
			case 'n':
				outString += '\n';
				break;
			case 'r':
				outString += '\r';
				break;
			case '"':
				outString += '"';
				break;
			case '\\':
				outString += '\\';
				break;
			default:
				outString += '\\';
				i--;
				break;
			}
		}
	}

	return true;
}

//---------------------- EspaceSequenceDecoder ----------------------

EspaceSequenceDecoder::EspaceSequenceDecoder()
{
}

/*virtual*/ EspaceSequenceDecoder::~EspaceSequenceDecoder()
{
}

/*virtual*/ bool EspaceSequenceDecoder::Transform(const std::string& inString, std::string& outString)
{
	if (&inString == &outString)
		return false;

	outString = "";

	for (int i = 0; inString.c_str()[i] != '\0'; i++)
	{
		char ch = inString.c_str()[i];
		switch (ch)
		{
		case '\t':
			outString += "\\t";
			break;
		case '\n':
			outString += "\\n";
			break;
		case '\r':
			outString += "\\r";
			break;
		case '"':
			outString += "\\\"";
			break;
		case '\\':
			outString += "\\\\";
			break;
		default:
			outString += ch;
			break;
		}
	}

	return true;
}