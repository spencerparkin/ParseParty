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
			SyntaxNode(const std::string& text, const Lexer::FileLocation& fileLocation);
			virtual ~SyntaxNode();

			void WipeChildren();
			void Flatten();
			void RemoveNodesWithText(const std::set<std::string>& textSet);
			int CalcSize() const;
			const SyntaxNode* FindChild(const std::string& text, int maxRecurseDepth, int depth = 1) const;
			const SyntaxNode* FindParent(const std::string& text, int maxRecurseDepth, int depth = 1) const;
			const SyntaxNode* GetParent() const;
			const SyntaxNode* GetChild(int i) const;
			SyntaxNode* GetParent();
			SyntaxNode* GetChild(int i);
			bool SetChild(int i, SyntaxNode* childNode);
			bool DelChild(int i);
			int GetChildCount() const;
			void Print(std::ostream& stream, int tabCount = 0) const;
			SyntaxNode* Clone() const;
			bool GetChildIterator(std::list<SyntaxNode*>::iterator& iter, int i);

			static bool ReadFromFile(const std::string& syntaxTreeFile, SyntaxNode*& rootNode);
			static bool WriteToFile(const std::string& syntaxTreeFile, const SyntaxNode* rootNode);

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

		Lexer lexer;
	};
}