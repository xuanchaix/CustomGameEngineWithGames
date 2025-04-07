#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector<std::string> Strings;
typedef std::vector<std::wstring> WStrings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

int SplitStringOnDelimiter( Strings& resultStringVector, std::string const& originalString, char delimiterToSplitOn = ',', bool removeExtraSpace = false );
int SplitStringOnDelimiter( Strings& resultStringVector, std::string const& originalString, std::string const& delimiterToSplitOn = std::string( "," ), bool removeExtraSpace = false );
Strings SplitStringOnDelimiter( std::string const& originalString, char delimiterToSplitOn = ',', bool removeExtraSpace = false );
int SplitStringOnDelimiter( WStrings& resultStringVector, std::wstring const& originalString, char delimiterToSplitOn = ',' );
WStrings SplitStringOnDelimiter( std::wstring const& originalString, char delimiterToSplitOn = ',' );
int SplitStringIntoLines( Strings& resultStringVector, std::string const& originalString );
int SplitStringWithQuotes( Strings& resultStringVector, std::string const& originalString, char delimiterToSplitOn = ',' );
void TrimString( std::string& originalString, char delimiterToTrim );
void StringToLower( std::string& str );
std::string StringToLower( std::string const& str );

class HashedCaseInsensitiveString {
public:
	HashedCaseInsensitiveString() {};
	HashedCaseInsensitiveString( std::string const& str );
	HashedCaseInsensitiveString( char const* str );
	HashedCaseInsensitiveString( HashedCaseInsensitiveString const& cpy );

	unsigned int GetHash() const;
	std::string const& GetString() const;
	char const* c_str() const;

	bool operator<( HashedCaseInsensitiveString const& compare ) const;
	bool operator==( HashedCaseInsensitiveString const& compare ) const;
	bool operator!=( HashedCaseInsensitiveString const& compare ) const;
	bool operator==( char const* text ) const;
	bool operator!=( char const* text ) const;
	bool operator==( std::string const& text ) const;
	bool operator!=( std::string const& text ) const;
	void operator=( HashedCaseInsensitiveString const& assignFrom );
	void operator=( char const* text );
	void operator=( std::string const& text );

	static unsigned int HashStringCaseInsensitive( std::string const& str );
	static unsigned int HashStringCaseInsensitive( char const* str );
protected:
	std::string m_caseIntactStr;
	unsigned int m_hash = 0;
};

