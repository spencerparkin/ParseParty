#pragma once

#include "Common.h"
#include "Lexer.h"
#include <format>

namespace ParseParty
{
	class PARSE_PARTY_API JsonException : public std::exception
	{
	public:
		JsonException(const std::string& errorMsg)
		{
			this->errorMsg = errorMsg;
		}

		const char* what() const noexcept override
		{
			return this->errorMsg.c_str();
		}

		std::string errorMsg;
	};

	class PARSE_PARTY_API JsonValue : public std::enable_shared_from_this<JsonValue>
	{
	public:
		struct Token;

		JsonValue();
		virtual ~JsonValue();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const = 0;
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) = 0;
		virtual std::shared_ptr<JsonValue> Clone() const = 0;

		static std::shared_ptr<JsonValue> ParseJson(const std::string& jsonString, std::string& parseError);
		static std::shared_ptr<JsonValue> ValueFactory(const Lexer::Token& token);
		static std::string MakeTabs(int tabCount);
		static std::string MakeError(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int parsePosition, const std::string& errorMsg);
	};

	class PARSE_PARTY_API JsonString : public JsonValue
	{
	public:
		JsonString();
		JsonString(const std::string& value);
		virtual ~JsonString();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;

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
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;

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
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;

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
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;

		void Clear();
		unsigned int GetSize() const;
		std::shared_ptr<JsonValue> GetValue(const std::string& key);
		std::shared_ptr<const JsonValue> GetValue(const std::string& key) const;
		bool SetValue(const std::string& key, std::shared_ptr<JsonValue> value);
		bool DeleteValue(const std::string& key);

		template<typename T>
		std::shared_ptr<T> GetValueOrThrow(const std::string& key)
		{
			std::shared_ptr<T> value = std::dynamic_pointer_cast<T>(this->GetValue(key));
			if (!value)
				throw JsonException(std::format("Did not find key \"{}\".", key.c_str()));

			return value;
		}

		template<typename T>
		std::shared_ptr<const T> GetValueOrThrow(const std::string& key) const
		{
			std::shared_ptr<const T> value = std::dynamic_pointer_cast<const T>(this->GetValue(key));
			if (!value)
				throw JsonException(std::format("Did not find key \"{}\".", key.c_str()));

			return value;
		}

		typedef std::map<std::string, std::shared_ptr<JsonValue>> JsonValueMap;

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
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;

		void Clear();
		unsigned int GetSize() const;
		std::shared_ptr<JsonValue> GetValue(unsigned int i);
		std::shared_ptr<const JsonValue> GetValue(unsigned int i) const;
		bool SetValue(unsigned int i, std::shared_ptr<JsonValue> value);
		bool RemoveValue(unsigned int i);
		bool InsertValue(unsigned int i, std::shared_ptr<JsonValue> value);
		void PushValue(std::shared_ptr<JsonValue> value);
		std::shared_ptr<JsonValue> PopValue();

		template<typename T>
		std::shared_ptr<T> GetValueOrThrow(unsigned int i)
		{
			std::shared_ptr<T> value = std::dynamic_pointer_cast<T>(this->GetValue(i));
			if (!value)
				throw JsonException(std::format("Did not find value at offset {}.", i));

			return value;
		}

		template<typename T>
		std::shared_ptr<const T> GetValueOrThrow(unsigned int i) const
		{
			std::shared_ptr<const T> value = std::dynamic_pointer_cast<const T>(this->GetValue(i));
			if (!value.get())
				throw JsonException(std::format("Did not find value at offset: {}.", i));

			return value;
		}

	private:
		typedef std::vector<std::shared_ptr<JsonValue>> JsonValueArray;
		JsonValueArray* valueArray;
	};

	class PARSE_PARTY_API JsonBool : public JsonValue
	{
	public:
		JsonBool();
		JsonBool(bool value);
		virtual ~JsonBool();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;

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
		virtual bool ParseTokens(const std::vector<std::shared_ptr<Lexer::Token>>& tokenArray, int& parsePosition, std::string& parseError) override;
		virtual std::shared_ptr<JsonValue> Clone() const override;
	};
}