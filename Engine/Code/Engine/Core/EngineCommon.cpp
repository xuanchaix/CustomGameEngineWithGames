#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"

NamedStrings g_gameConfigBlackboard;
NetSystem* g_theNetSystem = nullptr;
RandomNumberGenerator* g_engineRNG = new RandomNumberGenerator();

bool Event_LoadGameConfig( EventArgs& args )
{
	std::string filePath = args.GetValue( "File", "" );
	if (filePath != "") {
		XmlDocument gameConfig;
		XmlError errorCode = gameConfig.LoadFile( filePath.c_str() );
		GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document GameConfig.xml error" );
		XmlElement* root = gameConfig.FirstChildElement();
		GUARANTEE_OR_DIE( !strcmp( root->Name(), "GameConfig" ), "Syntax Error! Name of the root of GameConfig.xml should be \"GameConfig\" " );

		g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *root );
		return true;
	}
	return false;
}

EndianMode GetPlatformLocalEndian()
{
	int n = 1;
	if (*(char*)&n == 1) {
		return EndianMode::Little;
	}
	else {
		return EndianMode::Big;
	}
}
