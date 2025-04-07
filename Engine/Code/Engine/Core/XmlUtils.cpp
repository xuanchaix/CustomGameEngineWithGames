
#include "Engine/Core/XmlUtils.hpp"

int ParseXmlAttribute( XmlElement const& element, char const* attributeName, int defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		return atoi( attrValue );
	}
	return defaultValue;
}

char ParseXmlAttribute( XmlElement const& element, char const* attributeName, char defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		return attrValue[0];
	}
	return defaultValue;
}

bool ParseXmlAttribute( XmlElement const& element, char const* attributeName, bool defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (!attrValue) {
		return defaultValue;
	}
	else if (!strcmp( attrValue, "true" ) || !strcmp( attrValue, "True" ) || !strcmp( attrValue, "TRUE" )) {
		return true;
	}
	else if (!strcmp( attrValue, "false" ) || !strcmp( attrValue, "False" ) || !strcmp( attrValue, "FALSE" )) {
		return false;
	}
	return defaultValue;
}

float ParseXmlAttribute( XmlElement const& element, char const* attributeName, float defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		return (float)atof( attrValue );
	}
	return defaultValue;
}

Rgba8 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		Rgba8 rgba8;
		if (rgba8.SetFromText( attrValue )) {
			return rgba8;
		}
	}
	return defaultValue;
}

Vec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec2 const& defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		Vec2 vec2;
		if (vec2.SetFromText( attrValue )) {
			return vec2;
		}
	}
	return defaultValue;
}

IntVec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		IntVec2 intVec2;
		if (intVec2.SetFromText( attrValue )) {
			return intVec2;
		}
	}
	return defaultValue;
}

std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, std::string const& defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		return std::string( attrValue );
	}
	return defaultValue;
}

Strings ParseXmlAttribute( XmlElement const& element, char const* attributeName, Strings const& defaultValues )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		// ???
		return SplitStringOnDelimiter( std::string( attrValue ) );
	}
	return defaultValues;
}

std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, char const* defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		return std::string( attrValue );
	}
	return std::string( defaultValue );
}

Vec3 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec3 const& defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		Vec3 vec3;
		if (vec3.SetFromText( attrValue )) {
			return vec3;
		}
	}
	return defaultValue;
}

Vec4 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec4 const& defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		Vec4 vec4;
		if (vec4.SetFromText( attrValue )) {
			return vec4;
		}
	}
	return defaultValue;
}

EulerAngles ParseXmlAttribute( XmlElement const& element, char const* attributeName, EulerAngles defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		EulerAngles orientation;
		if (orientation.SetFromText( attrValue )) {
			return orientation;
		}
	}
	return defaultValue;
}

FloatRange ParseXmlAttribute( XmlElement const& element, char const* attributeName, FloatRange defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		FloatRange range;
		if (range.SetFromText( attrValue )) {
			return range;
		}
	}
	return defaultValue;
}

unsigned int ParseXmlAttribute( XmlElement const& element, char const* attributeName, unsigned int defaultValue )
{
	char const* attrValue = element.Attribute( attributeName );
	if (attrValue) {
		return strtoul( attrValue, 0l, 10 );
	}
	return defaultValue;
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, int value )
{
	element->SetAttribute( attributeName, value );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, unsigned int value )
{
	element->SetAttribute( attributeName, value );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, bool value )
{
	element->SetAttribute( attributeName, value );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, float value )
{
	element->SetAttribute( attributeName, value );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, Rgba8 const& value )
{
	std::string valueStr = Stringf( "%d,%d,%d,%d", value.r, value.g, value.b, value.a );
	element->SetAttribute( attributeName, valueStr.c_str() );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, Vec2 const& value )
{
	std::string valueStr = Stringf( "%f,%f", value.x, value.y );
	element->SetAttribute( attributeName, valueStr.c_str() );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, std::string const& value )
{
	element->SetAttribute( attributeName, value.c_str() );
}

void SetXmlAttribute( XmlElement* element, char const* attributeName, char const* value )
{
	element->SetAttribute( attributeName, value );
}
