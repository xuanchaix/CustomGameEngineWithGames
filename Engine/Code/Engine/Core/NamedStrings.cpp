#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdexcept>

void NamedStrings::PopulateFromXmlElementAttributes( XmlElement const& element )
{
	XmlAttribute const* iter = element.FirstAttribute();
	while (iter)
	{
		SetValue( iter->Name(), iter->Value() );
		iter = iter->Next();
	}
}

void NamedStrings::SetValue( std::string const& keyName, std::string const& newValue )
{
	m_keyValuePairs[StringToLower(keyName)] = newValue;
}

std::string NamedStrings::GetValue( std::string const& keyName, std::string const& defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		return found->second;
	}
	return defaultValue;
}

bool NamedStrings::GetValue( std::string const& keyName, bool defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		if (found->second == "True" || found->second == "true" || found->second == "TRUE") {
			return true;
		}
		else if (found->second == "False" || found->second == "false" || found->second == "FALSE") {
			return false;
		}
	}
	return defaultValue;
}

int NamedStrings::GetValue( std::string const& keyName, int defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		try
		{
			return stoi( found->second );
		}
		catch (std::exception const& e)
		{
			UNUSED( e );
			return defaultValue;
		}
	}
	return defaultValue;
}

float NamedStrings::GetValue( std::string const& keyName, float defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		try
		{
			return stof( found->second );
		}
		catch (std::exception const& e)
		{
			UNUSED( e );
			return defaultValue;
		}
	}
	return defaultValue;
}

std::string NamedStrings::GetValue( std::string const& keyName, char const* defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		return found->second;
	}
	return defaultValue;
}

Rgba8 NamedStrings::GetValue( std::string const& keyName, Rgba8 const& defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		Rgba8 retValue;
		retValue.SetFromText( found->second.c_str() );
		return retValue;
	}
	return defaultValue;
}

Vec2 NamedStrings::GetValue( std::string const& keyName, Vec2 const& defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		Vec2 retValue;
		retValue.SetFromText( found->second.c_str() );
		return retValue;
	}
	return defaultValue;
}

IntVec2 NamedStrings::GetValue( std::string const& keyName, IntVec2 const& defaultValue ) const
{
	std::map<std::string, std::string>::const_iterator found = m_keyValuePairs.find( StringToLower(keyName) );
	if (found != m_keyValuePairs.end()) {
		IntVec2 retValue;
		retValue.SetFromText( found->second.c_str() );
		return retValue;
	}
	return defaultValue;
}
