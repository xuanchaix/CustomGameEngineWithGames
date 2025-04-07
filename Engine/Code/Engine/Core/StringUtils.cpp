#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include <algorithm>

//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

int SplitStringOnDelimiter( Strings& resultStringVector, std::string const& originalString, char delimiterToSplitOn, bool removeExtraSpace )
{
	resultStringVector.clear();

	size_t findRes;
	size_t firstPos = 0;
	do {
		findRes = originalString.find_first_of( delimiterToSplitOn, firstPos );
		std::string strSplited = originalString.substr( firstPos, findRes - firstPos );
		if (removeExtraSpace && findRes != std::string::npos) {
			size_t nextNoSpacePos = originalString.find_first_not_of( ' ', findRes + 1 );
			if (nextNoSpacePos != 0 && nextNoSpacePos != std::string::npos) {
				firstPos = nextNoSpacePos;
			}
			else if (nextNoSpacePos == std::string::npos) {
				findRes = std::string::npos;
			}
			else {
				firstPos = findRes + 1;
			}
			strSplited.erase( remove( strSplited.begin(), strSplited.end(), ' ' ), strSplited.end() );
		}
		else {
			firstPos = findRes + 1;
		}
		
		resultStringVector.push_back( strSplited );
	} while (findRes != std::string::npos);

	return (int)resultStringVector.size();

}

Strings SplitStringOnDelimiter( std::string const& originalString, char delimiterToSplitOn, bool removeExtraSpace )
{
	Strings resultStringVector;

	size_t findRes;
	size_t firstPos = 0;
	do {
		findRes = originalString.find_first_of( delimiterToSplitOn, firstPos );
		std::string strSplited = originalString.substr( firstPos, findRes - firstPos );
		if (removeExtraSpace && findRes != std::string::npos) {
			size_t nextNoSpacePos = originalString.find_first_not_of( ' ', findRes + 1 );
			if (nextNoSpacePos != 0 && nextNoSpacePos != std::string::npos) {
				firstPos = nextNoSpacePos;
			}
			else if (nextNoSpacePos == std::string::npos) {
				findRes = std::string::npos;
			}
			else {
				firstPos = findRes + 1;
			}
			strSplited.erase( remove( strSplited.begin(), strSplited.end(), ' ' ), strSplited.end() );
		}
		else {
			firstPos = findRes + 1;
		}
		resultStringVector.push_back( strSplited );
		firstPos = findRes + 1;
	} while (findRes != std::string::npos);

	return resultStringVector;
}

WStrings SplitStringOnDelimiter( std::wstring const& originalString, char delimiterToSplitOn /*= ',' */ )
{
	WStrings resultStringVector;
	size_t findRes;
	size_t firstPos = 0;
	do {
		findRes = originalString.find_first_of( delimiterToSplitOn, firstPos );
		resultStringVector.push_back( originalString.substr( firstPos, findRes - firstPos ) );
		firstPos = findRes + 1;
	} while (findRes != std::wstring::npos);

	return resultStringVector;
}

int SplitStringOnDelimiter( WStrings& resultStringVector, std::wstring const& originalString, char delimiterToSplitOn /*= ',' */ )
{
	resultStringVector.clear();
	size_t findRes;
	size_t firstPos = 0;
	do {
		findRes = originalString.find_first_of( delimiterToSplitOn, firstPos );
		resultStringVector.push_back( originalString.substr( firstPos, findRes - firstPos ) );
		firstPos = findRes + 1;
	} while (findRes != std::wstring::npos);

	return (int)resultStringVector.size();
}

int SplitStringOnDelimiter( Strings& resultStringVector, std::string const& originalString, std::string const& delimiterToSplitOn /*= std::string( "," ) */, bool removeExtraSpace )
{
	resultStringVector.clear();
	size_t findRes;
	size_t firstPos = 0;
	size_t fwdSteps = delimiterToSplitOn.size();
	do {
		findRes = originalString.find_first_of( delimiterToSplitOn, firstPos );
		std::string strSplited = originalString.substr( firstPos, findRes - firstPos );
		if (removeExtraSpace && findRes != std::string::npos) {
			size_t nextNoSpacePos = originalString.find_first_not_of( ' ', findRes + 1 );
			if (nextNoSpacePos != 0 && nextNoSpacePos != std::string::npos) {
				firstPos = nextNoSpacePos;
			}
			else if (nextNoSpacePos == std::string::npos) {
				findRes = std::string::npos;
			}
			else {
				firstPos = findRes + 1;
			}
			strSplited.erase( remove( strSplited.begin(), strSplited.end(), ' ' ), strSplited.end() );
		}
		else {
			firstPos = findRes + 1;
		}
		resultStringVector.push_back( strSplited );
		firstPos = findRes + fwdSteps;
	} while (findRes != std::string::npos);

	return (int)resultStringVector.size();
	
}

int SplitStringIntoLines( Strings& resultStringVector, std::string const& originalString )
{
	resultStringVector.clear();
	size_t findRes;
	size_t firstPos = 0;
	do {
		findRes = originalString.find_first_of( '\n', firstPos);
		std::string strSplited = originalString.substr( firstPos, findRes - firstPos );
		strSplited.erase( remove( strSplited.begin(), strSplited.end(), '\r' ), strSplited.end() );
		resultStringVector.push_back( strSplited );
		firstPos = findRes + 1;
	} while (findRes != std::string::npos);

	return (int)resultStringVector.size();

}

int SplitStringWithQuotes( Strings& resultStringVector, std::string const& originalString, char delimiterToSplitOn /*= ',' */ )
{
	resultStringVector.clear();

	Strings splitByQuotes;
	SplitStringOnDelimiter( splitByQuotes, originalString, '"' );
	std::string prevStrBuffer;
	bool shouldSplitByDelimiter = true;
	for (auto& strByQuote : splitByQuotes) {
		if (shouldSplitByDelimiter) {
			size_t findRes;
			size_t firstPos = 0;
			do {
				findRes = strByQuote.find_first_of( delimiterToSplitOn, firstPos );
				if (findRes != std::wstring::npos) {
					if (!prevStrBuffer.empty()) {
						resultStringVector.push_back( prevStrBuffer + strByQuote.substr( firstPos, findRes - firstPos ) );
						prevStrBuffer.clear();
					}
					else {
						resultStringVector.push_back( strByQuote.substr( firstPos, findRes - firstPos ) );
					}
				}
				else {
					prevStrBuffer += strByQuote.substr( firstPos, (size_t)-1 );
				}
				firstPos = findRes + 1;
			} while (findRes != std::wstring::npos);
			shouldSplitByDelimiter = false;
		}
		else {
			shouldSplitByDelimiter = true;
			prevStrBuffer += ("\"" + strByQuote + "\"");
		}
	}
	if (!prevStrBuffer.empty()) {
		resultStringVector.push_back( prevStrBuffer );
	}

	return (int)resultStringVector.size();
}

void TrimString( std::string& originalString, char delimiterToTrim )
{
	while (!originalString.empty() && originalString[0] == delimiterToTrim) {
		originalString.erase( 0, 1 );
	}
	while (!originalString.empty() && originalString[originalString.size() - 1] == delimiterToTrim) {
		originalString.pop_back();
	}
}

void StringToLower( std::string& str )
{
	std::transform( str.begin(), str.end(), str.begin(), []( unsigned char c )->unsigned char { return (unsigned char)std::tolower( c ); } );
}

std::string StringToLower( std::string const& str )
{
	std::string retStr = str;
	std::transform( retStr.begin(), retStr.end(), retStr.begin(), []( unsigned char c )->unsigned char { return (unsigned char)std::tolower( c ); } );
	return retStr;
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString( std::string const& str )
	:HashedCaseInsensitiveString(str.c_str())
{

}

HashedCaseInsensitiveString::HashedCaseInsensitiveString( char const* str )
	:m_caseIntactStr(str)
	,m_hash(HashStringCaseInsensitive(str))
{

}

HashedCaseInsensitiveString::HashedCaseInsensitiveString( HashedCaseInsensitiveString const& cpy )
	:m_caseIntactStr(cpy.m_caseIntactStr)
	,m_hash(cpy.m_hash)
{

}

unsigned int HashedCaseInsensitiveString::GetHash() const
{
	return m_hash;
}

std::string const& HashedCaseInsensitiveString::GetString() const
{
	return m_caseIntactStr;
}

char const* HashedCaseInsensitiveString::c_str() const
{
	return m_caseIntactStr.c_str();
}

bool HashedCaseInsensitiveString::operator<( HashedCaseInsensitiveString const& compare ) const
{
	if (m_hash < compare.m_hash) {
		return true;
	}
	else if (m_hash == compare.m_hash) {
		return _stricmp( m_caseIntactStr.c_str(), compare.m_caseIntactStr.c_str() ) < 0;
	}
	return false;
}

unsigned int HashedCaseInsensitiveString::HashStringCaseInsensitive( std::string const& str )
{
	return HashStringCaseInsensitive( str.c_str() );
}

unsigned int HashedCaseInsensitiveString::HashStringCaseInsensitive( char const* str )
{
	char const* t = str;
	unsigned int hashValue = 0;
	while (*t != '\0') {
		hashValue *= 31;
		hashValue += (unsigned int)tolower( *t );
		++t;
	}
	return hashValue;
}

void HashedCaseInsensitiveString::operator=( std::string const& text )
{
	m_caseIntactStr = text;
	m_hash = HashStringCaseInsensitive( text );
}

void HashedCaseInsensitiveString::operator=( char const* text )
{
	m_caseIntactStr = text;
	m_hash = HashStringCaseInsensitive( text );
}

void HashedCaseInsensitiveString::operator=( HashedCaseInsensitiveString const& assignFrom )
{
	m_caseIntactStr = assignFrom.m_caseIntactStr;
	m_hash = assignFrom.m_hash;
}

bool HashedCaseInsensitiveString::operator!=( std::string const& text ) const
{
	if (m_hash != HashStringCaseInsensitive( text )) {
		return true;
	}
	return _stricmp( m_caseIntactStr.c_str(), text.c_str() ) != 0;
}

bool HashedCaseInsensitiveString::operator==( std::string const& text ) const
{
	if (m_hash == HashStringCaseInsensitive( text )) {
		return _stricmp( m_caseIntactStr.c_str(), text.c_str() ) == 0;
	}
	return false;
}

bool HashedCaseInsensitiveString::operator!=( char const* text ) const
{
	if (m_hash != HashStringCaseInsensitive( text )) {
		return true;
	}
	return _stricmp( m_caseIntactStr.c_str(), text ) != 0;
}

bool HashedCaseInsensitiveString::operator==( char const* text ) const
{
	if (m_hash == HashStringCaseInsensitive( text )) {
		return _stricmp( m_caseIntactStr.c_str(), text ) == 0;
	}
	return false;
}

bool HashedCaseInsensitiveString::operator!=( HashedCaseInsensitiveString const& compare ) const
{
	if (m_hash != compare.m_hash) {
		return true;
	}
	return _stricmp( m_caseIntactStr.c_str(), compare.m_caseIntactStr.c_str() ) != 0;
}

bool HashedCaseInsensitiveString::operator==( HashedCaseInsensitiveString const& compare ) const
{
	if (m_hash == compare.m_hash) {
		return _stricmp( m_caseIntactStr.c_str(), compare.m_caseIntactStr.c_str() ) == 0;
	}
	return false;
}
