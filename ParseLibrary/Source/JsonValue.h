#pragma once

#include "Common.h"
#include "Lexer.h"

namespace ParseParty
{
	// In hind-sight, I should have used std::shared_ptr<> here, but I didn't.
	// I now have so much code that uses these classes that I'm afraid to change it.

	class PARSE_PARTY_API JsonValue
	{
	public:
		struct Token;

		JsonValue();
		virtual ~JsonValue();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const = 0;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) = 0;

		static JsonValue* ParseJson(const std::string& jsonString, std::string& parseError);
		static JsonValue* ValueFactory(const Lexer::Token& token);
		static std::string MakeTabs(int tabCount);
		static std::string MakeError(const std::vector<Lexer::Token*>& tokenArray, int parsePosition, const std::string& errorMsg);
	};

	class PARSE_PARTY_API JsonString : public JsonValue
	{
	public:
		JsonString();
		JsonString(const std::string& value);
		virtual ~JsonString();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		const std::string& GetValue() const;
		void SetValue(const std::string& value);

	private:
		std::string* value;
	};

	class PARSE_PARTY_API JsonFloat : public JsonValue
	{
	public:
		JsonFloat();
		JsonFloat(double value);
		virtual ~JsonFloat();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		double GetValue() const;
		void SetValue(double value);

	private:
		double value;
	};

	class PARSE_PARTY_API JsonInt : public JsonValue
	{
	public:
		JsonInt();
		JsonInt(long value);
		virtual ~JsonInt();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		long GetValue() const;
		void SetValue(long value);

	private:
		long value;
	};

	class PARSE_PARTY_API JsonObject : public JsonValue
	{
	public:
		JsonObject();
		virtual ~JsonObject();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		void Clear();
		unsigned int GetSize() const;
		const JsonValue* GetValue(const std::string& key) const;
		JsonValue* GetValue(const std::string& key);
		bool SetValue(const std::string& key, JsonValue* value, bool freeMemory = true);
		bool DeleteValue(const std::string& key, bool freeMemory = true);

		typedef std::map<std::string, JsonValue*> JsonValueMap;

		JsonValueMap::iterator begin() { return this->valueMap->begin(); }
		JsonValueMap::const_iterator begin() const { return this->valueMap->begin(); }
		JsonValueMap::iterator end() { return this->valueMap->end(); }
		JsonValueMap::const_iterator end() const { return this->valueMap->end(); }

	private:
		JsonValueMap* valueMap;
	};

	class PARSE_PARTY_API JsonArray : public JsonValue
	{
	public:
		JsonArray();
		JsonArray(const std::vector<double>& floatArray);
		JsonArray(const std::vector<int>& intArray);
		virtual ~JsonArray();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		void Clear();
		unsigned int GetSize() const;
		const JsonValue* GetValue(unsigned int i) const;
		JsonValue* GetValue(unsigned int i);
		bool SetValue(unsigned int i, JsonValue* value);
		bool RemoveValue(unsigned int i);
		bool InsertValue(unsigned int i, JsonValue* value);
		void PushValue(JsonValue* value);
		JsonValue* PopValue();

	private:
		typedef std::vector<JsonValue*> JsonValueArray;
		JsonValueArray* valueArray;
	};

	class PARSE_PARTY_API JsonBool : public JsonValue
	{
	public:
		JsonBool();
		JsonBool(bool value);
		virtual ~JsonBool();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		bool GetValue() const;
		void SetValue(bool value);

	private:
		bool value;
	};

	class PARSE_PARTY_API JsonNull : public JsonValue
	{
	public:
		JsonNull();
		virtual ~JsonNull();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;
	};
}