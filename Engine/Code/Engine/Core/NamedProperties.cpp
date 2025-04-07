#include "Engine/Core/NamedProperties.hpp"


void NamedProperties::PopulateFromXmlElementAttributes( XmlElement const& element )
{
	XmlAttribute const* iter = element.FirstAttribute();
	while (iter)
	{
		SetValue( iter->Name(), iter->Value() );
		iter = iter->Next();
	}
}
