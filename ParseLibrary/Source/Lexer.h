#pragma once

#include "Common.h"
#include "FormatString.h"

namespace ParseParty
{
	class JsonObject;

	class PARSE_PARTY_API Lexer
	{
	public:
		Lexer();
		virtual ~Lexer();

		struct FileLocation
		{
			int line, column;

			operator std::string() const
			{
				return FormatString("Line %d, column %d: ", this->line, this->column);
			}
		};

		class Token;

		void Clear();
		bool ReadFile(const std::string& lexiconFile, std::string& error);
		bool WriteFile(const std::string& lexiconFile) const;

		bool Tokenize(const std::string& codeText, std::vector<std::shared_ptr<Token>>& tokenArray, std::string& error, bool keepComments = false, FileLocation initialFileLocation = FileLocation{ 1, 1 });

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
				IDENTIFIER,
				IDENTIFIER_KEYWORD,
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

			bool IsOpener() const;
			bool IsCloser() const;

			Type type;
			std::string* text;
			FileLocation fileLocation;
		};

		class PARSE_PARTY_API TokenGenerator
		{
		public:
			TokenGenerator();
			virtual ~TokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) = 0;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) = 0;
			virtual bool WriteConfig(JsonObject* jsonConfig) const = 0;
		};

		class PARSE_PARTY_API ParanTokenGenerator : public TokenGenerator
		{
		public:
			ParanTokenGenerator();
			virtual ~ParanTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;
		};

		class PARSE_PARTY_API DelimeterTokenGenerator : public TokenGenerator
		{
		public:
			DelimeterTokenGenerator();
			virtual ~DelimeterTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;
		};

		class PARSE_PARTY_API StringTokenGenerator : public TokenGenerator
		{
		public:
			StringTokenGenerator(bool processEscapeSequences = false);
			virtual ~StringTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;

			bool CollapseEscapeSequences(std::string& text);

			bool processEscapeSequences;
		};

		class PARSE_PARTY_API NumberTokenGenerator : public TokenGenerator
		{
		public:
			NumberTokenGenerator();
			virtual ~NumberTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;
		};

		class PARSE_PARTY_API OperatorTokenGenerator : public TokenGenerator
		{
		public:
			OperatorTokenGenerator();
			virtual ~OperatorTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;

			std::set<std::string>* operatorSet;
			std::set<char>* operatorCharSet;
		};

		class PARSE_PARTY_API IdentifierTokenGenerator : public TokenGenerator
		{
		public:
			IdentifierTokenGenerator();
			virtual ~IdentifierTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;

			std::set<std::string>* keywordSet;
		};

		class PARSE_PARTY_API CommentTokenGenerator : public TokenGenerator
		{
		public:
			CommentTokenGenerator();
			virtual ~CommentTokenGenerator();

			virtual std::shared_ptr<Token> GenerateToken(const char* codeBuffer, int& i) override;
			virtual bool ReadConfig(const JsonObject* jsonConfig, std::string& error) override;
			virtual bool WriteConfig(JsonObject* jsonConfig) const override;
		};

		std::list<TokenGenerator*>* tokenGeneratorList;
		int tabSize;
	};

	bool operator<(const Lexer::FileLocation& locationA, const Lexer::FileLocation& locationB);
}