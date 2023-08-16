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

		bool Tokenize(const std::string& codeText, std::vector<Token*>& tokenArray);

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
				NUMBER_LITERAL_FLOAT,
				NUMBER_LITERAL_INT,
				BOOLEAN_LITERAL,
				OPEN_PARAN,
				CLOSE_PARAN,
				OPEN_SQUARE_BRACKET,
				CLOSE_SQUARE_BRACKET,
				OPEN_CURLY_BRACE,
				CLOSE_CURCLY_BRACE,
			};

			bool Eat(const char* givenBuffer, int& i);

			Type type;
			std::string* text;
		};
	};
}