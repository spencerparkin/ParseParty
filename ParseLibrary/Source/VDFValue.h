#pragma once

#include "Common.h"
#include "Lexer.h"
#include <memory>

namespace ParseParty
{
	/**
	 * VDF stands for value data format.
	 */
	class PARSE_PARTY_API VDFValue
	{
	public:
		VDFValue();
		virtual ~VDFValue();

		virtual void PrintVDF(std::string& vdfString, int tabLevel = 0) const = 0;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) = 0;

		static std::shared_ptr<VDFValue> ParseVDF(const std::string& vdfString, std::string& parseError);
		static std::string MakeError(const std::vector<Lexer::Token*>& tokenArray, int parsePosition, const std::string& errorMsg);
		static std::string MakeTabs(int tabCount);
	};

	/**
	 * 
	 */
	class PARSE_PARTY_API VDFStringValue : public VDFValue
	{
	public:
		VDFStringValue();
		virtual ~VDFStringValue();

		virtual void PrintVDF(std::string& vdfString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		void SetValue(const std::string& value);
		const std::string& GetValue() const;

	private:
		std::string* value;
	};

	/**
	 * 
	 */
	class PARSE_PARTY_API VDFBlockValue : public VDFValue
	{
	public:
		VDFBlockValue();
		virtual ~VDFBlockValue();

		virtual void PrintVDF(std::string& vdfString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError) override;

		struct Pair
		{
			std::string key;
			std::shared_ptr<VDFValue> value;
		};

		const std::shared_ptr<VDFValue> FindValue(const std::string& key, int keyInstance = 0) const;
		std::shared_ptr<VDFValue> FindValue(const std::string& key, int keyInstance = 0);

		template<typename T>
		const T* FindValueOfType(const std::string& key, int keyInstance = 0) const
		{
			const std::shared_ptr<VDFValue> vdfValue = this->FindValue(key, keyInstance);
			return dynamic_cast<const T*>(vdfValue.get());
		}

		template<typename T>
		T* FindValueOfType(const std::string& key, int keyInstance = 0)
		{
			std::shared_ptr<VDFValue> vdfValue = this->FindValue(key, keyInstance);
			return dynamic_cast<T*>(vdfValue.get());
		}

		bool DeleteKey(const std::string& key, int keyInstance = 0);
		void AddKey(const std::string& key, std::shared_ptr<VDFValue> vdfValue);

	public:
		// Note that we use an array here instead of a map, because the
		// VDF file format specification allows for duplicate keys.
		std::vector<Pair> pairArray;
	};
}