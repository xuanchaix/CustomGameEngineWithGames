#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <map>

class TypedPropertiesBase {
public:
	virtual ~TypedPropertiesBase() {};
};

template <typename T>
class TypedProperties : public TypedPropertiesBase {
public:
	TypedProperties( T data ) : m_data( data ) {};
	virtual ~TypedProperties() {};
	T m_data;
};

class NamedProperties {
public:
	~NamedProperties() {
		for (auto pair : m_keyValuePairs) {
			free( pair.second );
		}
	}

	void PopulateFromXmlElementAttributes( XmlElement const& element );

	template <typename T>
	void SetValue( std::string const& keyName, T const& newValue ) {
		std::map<HashedCaseInsensitiveString, TypedPropertiesBase*>::iterator found = m_keyValuePairs.find( HashedCaseInsensitiveString( keyName ) );
		if (found != m_keyValuePairs.end()) {
			TypedProperties<T>* asTypedProperties = dynamic_cast<TypedProperties<T>*>(found->second);
			if (asTypedProperties) {
				asTypedProperties->m_data = newValue;
			}
			else {
				delete asTypedProperties;
				m_keyValuePairs[keyName] = new TypedProperties<T>( newValue );
			}
		}
		else {
			m_keyValuePairs[keyName] = new TypedProperties<T>( newValue );
		}
	}

	void SetValue( std::string const& keyName, char const* newValue )
	{
		std::map<HashedCaseInsensitiveString, TypedPropertiesBase*>::iterator found = m_keyValuePairs.find( HashedCaseInsensitiveString( keyName ) );
		if (found != m_keyValuePairs.end()) {
			TypedProperties<std::string>* asTypedProperties = dynamic_cast<TypedProperties<std::string>*>(found->second);
			if (asTypedProperties) {
				asTypedProperties->m_data = std::string(newValue);
			}
			else {
				delete asTypedProperties;
				m_keyValuePairs[keyName] = new TypedProperties<std::string>( newValue );
			}
		}
		else {
			m_keyValuePairs[keyName] = new TypedProperties<std::string>( newValue );
		}
	}

	template <typename T>
	T GetValue( std::string const& keyName, T const& defaultValue ) const {
		std::map<HashedCaseInsensitiveString, TypedPropertiesBase*>::const_iterator found = m_keyValuePairs.find( HashedCaseInsensitiveString( keyName ) );
		if (found != m_keyValuePairs.end()) {
			TypedProperties<T>* asTypedProperties = dynamic_cast<TypedProperties<T>*>(found->second);
			if (asTypedProperties) {
				return asTypedProperties->m_data;
			}
			else {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	std::string GetValue( std::string const& keyName, char const* defaultValue ) const
	{
		std::map<HashedCaseInsensitiveString, TypedPropertiesBase*>::const_iterator found = m_keyValuePairs.find( HashedCaseInsensitiveString( keyName ) );
		if (found != m_keyValuePairs.end()) {
			TypedProperties<std::string>* asTypedProperties = dynamic_cast<TypedProperties<std::string>*>(found->second);
			if (asTypedProperties) {
				return asTypedProperties->m_data;
			}
			else {
				return defaultValue;
			}
		}
		return defaultValue;
	}

private:
	std::map<HashedCaseInsensitiveString, TypedPropertiesBase*> m_keyValuePairs;
};