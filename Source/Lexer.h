#pragma once

#include "Common.h"

namespace ParseParty
{
	class PARSE_PARTY_API Lexer
	{
	public:
		Lexer();
		virtual ~Lexer();

		class Token;

		bool Tokenize(const char* codeBuffer, std::vector<Token>& tokenArray);

		class PARSE_PARTY_API Token
		{
		public:
			Token();
			virtual ~Token();

			enum class Type
			{
				UNKNOWN,
				COMMA,
				COLON,
				SEMI_COLON,
				OPERATOR,
				IDENTIFIER,
				STRING_LITERAL,
				NUMBER_LITERAL,
				LEFT_PARAN,
				RIGHT_PARAN,
				LEFT_SQUARE_BRACKET,
				RIGHT_SQUARE_BRACKET,
				LEFT_CURLY_BRACE,
				RIGHT_CURCLY_RACE,
			};

			Type type;
			std::string* text;
		};
	};
}