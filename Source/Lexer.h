#pragma once

#include "Common.h"

namespace ParseParty
{
	// TODO: Customization of the lexer would involve letting the user populate the token-generator list themselves.
	class PARSE_PARTY_API Lexer
	{
	public:
		Lexer();
		virtual ~Lexer();

		struct FileLocation
		{
			int line, column;
		};

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
				COMMENT,		// Typically these are discarded before parsing.
				DELIMETER_COMMA,
				DELIMETER_COLON,
				DELIMETER_SEMI_COLON,
				OPERATOR,
				IDENTIFIER,		// These could be variable names or keywords.
				STRING_LITERAL,
				NUMBER_LITERAL_FLOAT,
				NUMBER_LITERAL_INT,
				OPEN_PARAN,
				CLOSE_PARAN,
				OPEN_SQUARE_BRACKET,
				CLOSE_SQUARE_BRACKET,
				OPEN_CURLY_BRACE,
				CLOSE_CURLY_BRACE,
			};

			Type type;
			std::string* text;
			FileLocation fileLocation;
		};

		class PARSE_PARTY_API TokenGenerator
		{
		public:
			TokenGenerator();
			virtual ~TokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) = 0;

			bool IsCharFoundIn(char ch, const char* charSet);
		};

		class PARSE_PARTY_API ParanTokenGenerator : public TokenGenerator
		{
		public:
			ParanTokenGenerator();
			virtual ~ParanTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;
		};

		class PARSE_PARTY_API DelimeterTokenGenerator : public TokenGenerator
		{
		public:
			DelimeterTokenGenerator();
			virtual ~DelimeterTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;
		};

		class PARSE_PARTY_API StringTokenGenerator : public TokenGenerator
		{
		public:
			StringTokenGenerator();
			virtual ~StringTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;
		};

		class PARSE_PARTY_API NumberTokenGenerator : public TokenGenerator
		{
		public:
			NumberTokenGenerator();
			virtual ~NumberTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;
		};

		class PARSE_PARTY_API OperatorTokenGenerator : public TokenGenerator
		{
		public:
			OperatorTokenGenerator();
			virtual ~OperatorTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;

			std::set<std::string>* operatorSet;
		};

		class PARSE_PARTY_API IdentifierTokenGenerator : public TokenGenerator
		{
		public:
			IdentifierTokenGenerator();
			virtual ~IdentifierTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;
		};

		class PARSE_PARTY_API CommentTokenGenerator : public TokenGenerator
		{
		public:
			CommentTokenGenerator();
			virtual ~CommentTokenGenerator();

			virtual Token* GenerateToken(const char* codeBuffer, int& i) override;
		};

		std::list<TokenGenerator*>* tokenGeneratorList;
	};
}