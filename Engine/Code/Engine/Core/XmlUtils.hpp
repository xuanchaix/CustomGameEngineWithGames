#pragma once
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;
typedef tinyxml2::XMLError XmlError;


int ParseXmlAttribute( XmlElement const& element, char const* attributeName, int defaultValue );
unsigned int ParseXmlAttribute( XmlElement const& element, char const* attributeName, unsigned int defaultValue );
char ParseXmlAttribute( XmlElement const& element, char const* attributeName, char defaultValue );
bool ParseXmlAttribute( XmlElement const& element, char const* attributeName, bool defaultValue );
float ParseXmlAttribute( XmlElement const& element, char const* attributeName, float defaultValue );
Rgba8 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue );
Vec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec2 const& defaultValue );
Vec3 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec3 const& defaultValue );
Vec4 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec4 const& defaultValue );
IntVec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue );
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, std::string const& defaultValue );
Strings ParseXmlAttribute( XmlElement const& element, char const* attributeName, Strings const& defaultValues );
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, char const* defaultValue );
EulerAngles ParseXmlAttribute( XmlElement const& element, char const* attributeName, EulerAngles defaultValue );
FloatRange ParseXmlAttribute( XmlElement const& element, char const* attributeName, FloatRange defaultValue );

void SetXmlAttribute( XmlElement* element, char const* attributeName, int value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, unsigned int value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, bool value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, float value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, Rgba8 const& value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, Vec2 const& value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, std::string const& value );
void SetXmlAttribute( XmlElement* element, char const* attributeName, char const* value );
