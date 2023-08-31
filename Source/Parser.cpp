#include "Parser.h"
#include "QuickParseAlgorithm.h"
#include "SlowParseAlgorithm.h"

using namespace ParseParty;

//------------------------------- Parser -------------------------------

Parser::Parser()
{
}

/*virtual*/ Parser::~Parser()
{
}

Parser::SyntaxNode* Parser::ParseFile(const std::string& codeFile, const Grammar& grammar, std::string* error /*= nullptr*/)
{
	std::fstream fileStream(codeFile.c_str(), std::ios::in);
	if (!fileStream.is_open())
		return nullptr;

	std::stringstream stringStream;
	stringStream << fileStream.rdbuf();
	std::string codeText = stringStream.str();
	SyntaxNode* rootNode = this->Parse(codeText, grammar, error);
	fileStream.close();
	return rootNode;
}

Parser::SyntaxNode* Parser::Parse(const std::string& codeText, const Grammar& grammar, std::string* error /*= nullptr*/)
{
	std::vector<Lexer::Token*> tokenArray;

	if (!this->lexer.Tokenize(codeText, tokenArray))
	{
		if (error)
			*error = "Tokenization failed.";

		return nullptr;
	}

	SyntaxNode* rootNode = this->Parse(tokenArray, grammar, error);

	for (Lexer::Token* token : tokenArray)
		delete token;

	return rootNode;
}

Parser::SyntaxNode* Parser::Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar, std::string* error /*= nullptr*/)
{
	Algorithm* algorithm = nullptr;

	if (*grammar.algorithmName == "quick")
		algorithm = new QuickParseAlgorithm(&tokenArray, &grammar);
	else if (*grammar.algorithmName == "slow")
		algorithm = new SlowParseAlgorithm(&tokenArray, &grammar);

	if (!algorithm)
	{
		if (error)
			*error = (grammar.algorithmName->length() > 0) ? std::format("Unrecognized parse algorithm: {}", grammar.algorithmName->c_str()) : "No parse algorithm specified.";

		return nullptr;
	}

	SyntaxNode* rootNode = algorithm->Parse();

	if (!rootNode)
	{
		if (error)
			*error = *algorithm->error;
	}
	else
	{
		if ((grammar.flags & PARSE_PARTY_GRAMMAR_FLAG_DELETE_STRUCTURE_TOKENS) != 0)
		{
			// Nodes with the following text no longer give meaning or structure to the code.
			// The structure/meaning of the code is now found in the structure of the AST.
			std::set<std::string> textSet;
			textSet.insert(";");
			textSet.insert(",");
			textSet.insert("(");
			textSet.insert(")");
			textSet.insert("{");
			textSet.insert("}");
			textSet.insert("[");
			textSet.insert("]");
			rootNode->RemoveNodesWithText(textSet);
		}

		if ((grammar.flags & PARSE_PARTY_GRAMMAR_FLAG_FLATTEN_AST) != 0)
		{
			// Recursive definitions in the grammar cause unnecessary structure in the AST
			// that we are trying to remove here.  This makes the tree easier to read and process.
			rootNode->Flatten();
		}
	}

	delete algorithm;

	return rootNode;
}

//------------------------------- Parser::Algorithm -------------------------------

Parser::Algorithm::Algorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar)
{
	this->tokenArray = tokenArray;
	this->grammar = grammar;
	this->error = new std::string();
}

/*virtual*/ Parser::Algorithm::~Algorithm()
{
	delete this->error;
}

//------------------------------- Parser::SyntaxNode -------------------------------

Parser::SyntaxNode::SyntaxNode()
{
	this->parentNode = nullptr;
	this->childList = new std::list<SyntaxNode*>();
	this->text = new std::string();
	this->fileLocation.line = -1;
	this->fileLocation.column = -1;
}

Parser::SyntaxNode::SyntaxNode(const std::string& text, const Lexer::FileLocation& fileLocation)
{
	this->parentNode = nullptr;
	this->childList = new std::list<SyntaxNode*>();
	this->text = new std::string(text.c_str());
	this->fileLocation = fileLocation;
}

/*virtual*/ Parser::SyntaxNode::~SyntaxNode()
{
	this->WipeChildren();

	delete this->text;
	delete this->childList;
}

bool Parser::SyntaxNode::ReadFromJson(const JsonObject* jsonParentNode, std::string& parseError)
{
	const JsonString* jsonText = dynamic_cast<const JsonString*>(jsonParentNode->GetValue("text"));
	if (!jsonText)
	{
		parseError = "Expected \"text\" entry in syntax node object.";
		return false;
	}

	const JsonInt* jsonLine = dynamic_cast<const JsonInt*>(jsonParentNode->GetValue("line"));
	if (!jsonLine)
	{
		parseError = "Expected \"line\" entry in syntax node object.";
		return false;
	}

	const JsonInt* jsonColumn = dynamic_cast<const JsonInt*>(jsonParentNode->GetValue("column"));
	if (!jsonColumn)
	{
		parseError = "Expected \"column\" entry in syntax node object.";
		return false;
	}

	const JsonArray* jsonChildArray = dynamic_cast<const JsonArray*>(jsonParentNode->GetValue("children"));
	if (!jsonChildArray)
	{
		parseError = "Expected \"children\" array entry in syntax node object.";
		return false;
	}

	*this->text = jsonText->GetValue();
	this->fileLocation.line = (int)jsonLine->GetValue();
	this->fileLocation.column = (int)jsonLine->GetValue();

	this->WipeChildren();

	for (int i = 0; i < (signed)jsonChildArray->GetSize(); i++)
	{
		const JsonObject* jsonChildNode = dynamic_cast<const JsonObject*>(jsonChildArray->GetValue(i));
		if (!jsonChildNode)
		{
			parseError = std::format("Expected child entry {} to be an object.", i);
			return false;
		}

		SyntaxNode* childNode = new SyntaxNode();
		this->childList->push_back(childNode);
		childNode->parentNode = this;
		if (!childNode->ReadFromJson(jsonChildNode, parseError))
			return false;
	}

	return true;
}

bool Parser::SyntaxNode::WriteToJson(JsonObject* jsonParentNode) const
{
	jsonParentNode->SetValue("text", new JsonString(*this->text));
	jsonParentNode->SetValue("line", new JsonInt(this->fileLocation.line));
	jsonParentNode->SetValue("column", new JsonInt(this->fileLocation.column));

	JsonArray* jsonChildArray = new JsonArray();
	jsonParentNode->SetValue("children", jsonChildArray);

	for (const SyntaxNode* childNode : *this->childList)
	{
		JsonObject* jsonChildNode = new JsonObject();
		jsonChildArray->PushValue(jsonChildNode);
		if (!childNode->WriteToJson(jsonChildNode))
			return false;
	}

	return true;
}

/*static*/ bool Parser::SyntaxNode::ReadFromFile(const std::string& syntaxTreeFile, SyntaxNode*& rootNode, std::string& parseError)
{
	bool success = false;
	JsonValue* jsonValue = nullptr;
	
	if (rootNode)
	{
		delete rootNode;
		rootNode = nullptr;
	}

	do
	{
		std::ifstream fileStream(syntaxTreeFile, std::ios::in);
		if (!fileStream.is_open())
			break;

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string jsonString = stringStream.str();
		fileStream.close();

		jsonValue = JsonValue::ParseJson(jsonString, parseError);
		if (!jsonValue)
			break;

		JsonObject* jsonObject = dynamic_cast<JsonObject*>(jsonValue);
		if (!jsonObject)
		{
			parseError = "Expected root-level value to be an object.";
			break;
		}

		const JsonObject* jsonRootNode = dynamic_cast<JsonObject*>(jsonObject->GetValue("root"));
		if (!jsonRootNode)
		{
			parseError = "Expected \"root\" entry.";
			break;
		}

		rootNode = new SyntaxNode();
		if (!rootNode->ReadFromJson(jsonRootNode, parseError))
			break;

		success = true;
	} while (false);

	if (!success)
	{
		delete rootNode;
		rootNode = nullptr;
	}

	delete jsonValue;
	return success;
}

/*static*/ bool Parser::SyntaxNode::WriteToFile(const std::string& syntaxTreeFile, const SyntaxNode* rootNode)
{
	bool success = false;
	JsonObject* jsonValue = nullptr;

	do
	{
		if (!rootNode)
			break;

		jsonValue = new JsonObject();
		if (!jsonValue)
			break;

		time_t timeValue = std::time(nullptr);
		std::tm* localTimeValue = std::localtime(&timeValue);
		std::ostringstream timeStringStream;
		timeStringStream << std::put_time(localTimeValue, "Generated on %m/%d/%Y at %H:%M:%S by Parse-Party!  It's a party, brah!");
		jsonValue->SetValue("comment", new JsonString(timeStringStream.str()));

		JsonObject* jsonRootNode = new JsonObject();
		jsonValue->SetValue("root", jsonRootNode);
		if (!rootNode->WriteToJson(jsonRootNode))
			break;

		std::ofstream fileStream(syntaxTreeFile, std::ios::out);
		if (!fileStream.is_open())
			break;

		std::string jsonString;
		if (!jsonValue->PrintJson(jsonString))
			break;

		fileStream << jsonString;
		fileStream.close();

		success = true;
	} while (false);

	delete jsonValue;
	return success;
}

const Parser::SyntaxNode* Parser::SyntaxNode::FindChild(const std::string& text, int maxRecurseDepth, int depth /*= 1*/) const
{
	for (const SyntaxNode* childNode : *this->childList)
	{
		if (*childNode->text == text)
			return childNode;

		if (depth < maxRecurseDepth)
		{
			const SyntaxNode* foundNode = childNode->FindChild(text, maxRecurseDepth, depth + 1);
			if (foundNode)
				return foundNode;
		}
	}

	return nullptr;
}

const Parser::SyntaxNode* Parser::SyntaxNode::FindParent(const std::string& text, int maxRecurseDepth, int depth /*= 1*/) const
{
	if (!this->parentNode)
		return nullptr;

	if (*this->parentNode->text == text)
		return this->parentNode;

	if (depth < maxRecurseDepth)
	{
		const SyntaxNode* foundNode = this->parentNode->FindParent(text, maxRecurseDepth, depth + 1);
		if (foundNode)
			return foundNode;
	}

	return nullptr;
}

const Parser::SyntaxNode* Parser::SyntaxNode::GetParent() const
{
	return this->parentNode;
}

const Parser::SyntaxNode* Parser::SyntaxNode::GetChild(int i) const
{
	return const_cast<SyntaxNode*>(this)->GetChild(i);
}

Parser::SyntaxNode* Parser::SyntaxNode::GetParent()
{
	return this->parentNode;
}

Parser::SyntaxNode* Parser::SyntaxNode::GetChild(int i)
{
	std::list<SyntaxNode*>::iterator iter;
	if (this->GetChildIterator(iter, i))
		return *iter;

	return nullptr;
}

bool Parser::SyntaxNode::SetChild(int i, SyntaxNode* childNode)
{
	std::list<SyntaxNode*>::iterator iter;
	if (this->GetChildIterator(iter, i))
	{
		*iter = childNode;
		return true;
	}

	return false;
}

bool Parser::SyntaxNode::DelChild(int i)
{
	std::list<SyntaxNode*>::iterator iter;
	if (this->GetChildIterator(iter, i))
	{
		delete* iter;
		this->childList->erase(iter);
		return true;
	}

	return false;
}

bool Parser::SyntaxNode::GetChildIterator(std::list<SyntaxNode*>::iterator& iter, int i)
{
	if (0 <= i && i < (signed)this->childList->size())
	{
		iter = this->childList->begin();
		while (--i >= 0)
			iter++;

		return true;
	}

	return false;
}

int Parser::SyntaxNode::GetChildCount() const
{
	return (int)this->childList->size();
}

void Parser::SyntaxNode::WipeChildren()
{
	for (SyntaxNode* childNode : *this->childList)
		delete childNode;

	this->childList->clear();
}

void Parser::SyntaxNode::Flatten()
{
	for (SyntaxNode* childNode : *this->childList)
		childNode->Flatten();

	std::list<SyntaxNode*>::iterator nextIterA;
	for (std::list<SyntaxNode*>::iterator iterA = this->childList->begin(); iterA != this->childList->end(); iterA = nextIterA)
	{
		nextIterA = iterA;
		nextIterA++;

		SyntaxNode* childNode = *iterA;

		if (*this->text == *childNode->text)
		{
			for (std::list<SyntaxNode*>::iterator iterB = childNode->childList->begin(); iterB != childNode->childList->end(); iterB++)
				this->childList->insert(iterA, *iterB);

			childNode->childList->clear();
			delete childNode;
			this->childList->erase(iterA);
		}
	}
}

void Parser::SyntaxNode::RemoveNodesWithText(const std::set<std::string>& textSet)
{
	// TODO: Isn't there some fancy boost algorithm for this kind of stuff?  Like some sort of filter function?
	std::list<SyntaxNode*>::iterator nextIter;
	for (std::list<SyntaxNode*>::iterator iter = this->childList->begin(); iter != this->childList->end(); iter = nextIter)
	{
		nextIter = iter;
		nextIter++;

		SyntaxNode* childNode = *iter;
		if (textSet.find(*childNode->text) != textSet.end())
		{
			delete childNode;
			this->childList->erase(iter);
		}
	}

	for (SyntaxNode* childNode : *this->childList)
		childNode->RemoveNodesWithText(textSet);
}

int Parser::SyntaxNode::CalcSize() const
{
	int count = 1;

	for (SyntaxNode* childNode : *this->childList)
		count += childNode->CalcSize();

	return count;
}

void Parser::SyntaxNode::Print(std::ostream& stream, int tabCount /*= 0*/) const
{
	for (int i = 0; i < tabCount; i++)
		stream << "\t";

	stream << *this->text << "\n";

	for (const SyntaxNode* childNode : *this->childList)
		childNode->Print(stream, tabCount + 1);
}

Parser::SyntaxNode* Parser::SyntaxNode::Clone() const
{
	SyntaxNode* syntaxNode = new SyntaxNode(*this->text, this->fileLocation);

	for (const SyntaxNode* childNode : *this->childList)
		syntaxNode->childList->push_back(childNode->Clone());

	return syntaxNode;
}