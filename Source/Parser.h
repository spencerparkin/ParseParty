#pragma once

#include "Common.h"
#include "Lexer.h"
#include "Grammar.h"

namespace ParseParty
{
	class PARSE_PARTY_API Parser
	{
	public:
		Parser();
		virtual ~Parser();

		class SyntaxNode;

		SyntaxNode* ParseFile(const std::string& codeFile, const Grammar& grammar, std::string* error = nullptr);
		SyntaxNode* Parse(const std::string& codeText, const Grammar& grammar, std::string* error = nullptr);
		SyntaxNode* Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar, std::string* error = nullptr);

		class PARSE_PARTY_API SyntaxNode
		{
		public:
			SyntaxNode();
			virtual ~SyntaxNode();

			void WipeChildren();
			void Flatten();
			void RemoveNodesWithText(const std::set<std::string>& textSet);
			int CalcSize() const;
			const SyntaxNode* FindChild(const std::string& text, int maxRecurseDepth, int depth = 1) const;
			const SyntaxNode* FindParent(const std::string& text, int maxRecurseDepth, int depth = 1) const;
			const SyntaxNode* GetParent() const;
			const SyntaxNode* GetChild(int i) const;
			int GetChildCount() const;

			SyntaxNode* parentNode;
			std::list<SyntaxNode*>* childList;
			std::string* text;
			Lexer::FileLocation fileLocation;
		};

		// A grammar may specify an algorithm to use.
		class PARSE_PARTY_API Algorithm
		{
		public:
			Algorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
			virtual ~Algorithm();

			virtual SyntaxNode* Parse() = 0;

			const std::vector<Lexer::Token*>* tokenArray;
			const Grammar* grammar;

			std::string* error;
		};
	};
}