#pragma once

#include "Common.h"

namespace ParseParty
{
	class PARSE_PARTY_API StringTransformer
	{
	public:
		StringTransformer();
		virtual ~StringTransformer();

		virtual bool Transform(const std::string& inString, std::string& outString) = 0;
	};

	class PARSE_PARTY_API EspaceSequenceEncoder : public StringTransformer
	{
	public:
		EspaceSequenceEncoder();
		virtual ~EspaceSequenceEncoder();

		virtual bool Transform(const std::string& inString, std::string& outString) override;
	};

	class PARSE_PARTY_API EspaceSequenceDecoder : public StringTransformer
	{
	public:
		EspaceSequenceDecoder();
		virtual ~EspaceSequenceDecoder();

		virtual bool Transform(const std::string& inString, std::string& outString) override;
	};
}