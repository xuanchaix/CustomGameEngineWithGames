#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
}

Game::~Game()
{
	g_theAudio->StopSound( m_gameMusic );
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
	for (int i = 0; i < (int)m_maps.size(); i++) {
		if (m_maps[i]) {
			delete m_maps[i];
		}
	}
}

void Game::Startup()
{
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Game Initializing..." );
	LoadMapDefinitions();
	PlayerTank* player = nullptr;
	std::string maps = g_gameConfigBlackboard.GetValue( "maps", "" );
	if (maps != "") {
		Strings splitedMaps;
		int numOfLevels = SplitStringOnDelimiter( splitedMaps, maps, ',' );
		m_maps.reserve( numOfLevels );
		for (int i = 0; i < numOfLevels; i++) {
			MapDefinition const& def = GetMapDef( splitedMaps[i] );
			if (i == 0) {
				m_numOfReinforcements = def.m_addReinforcements;
			}
			Map* newMap = new Map();
			m_maps.push_back( newMap );
			if (i == 0) {
				player = (PlayerTank*)(newMap->SpawnNewEntity( EntityType::_GOOD_PLAYER, Vec2( 1.5, 1.5 ), EntityFaction::FACTION_GOOD ));
			}
			newMap->SetPlayer( player );
			newMap->Startup( def );
		}
	}
	else {
		ERROR_AND_DIE( "Cannot read maps in GameConfig" );
	}
	m_curMap = m_maps[m_curMapIndex];
	m_gameMusic = g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::GameMode ), true );
	g_enemyVisibleRange = g_gameConfigBlackboard.GetValue( "enemyVisibleRange", 10.f );
}

void Game::Update( float deltaTime )
{
	HandleKey();
	if (deltaTime != 0.f && m_curMap) {
		m_curMap->Update( deltaTime );
		if (m_curMap->m_curMapState == MapState::FINISH_EXIT) {
			CallBackGoToNextMap();
		}
	}
}

void Game::Render() const
{
	if (m_curMap) {
		m_curMap->Render();
	}
}

void Game::GoToNextMap()
{
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, Stringf( "Going To Next Map..." ) );
	m_curMapIndex++;
	if (m_curMapIndex >= (int)m_maps.size()) {
		// victory
		m_curMap->m_curMapState = MapState::WIN;
		g_theAudio->StopSound( m_gameMusic );
		g_theApp->PlaySound( AudioName::Victory );
		return;
	}
	m_curMap->m_curMapState = MapState::EXIT_MAP;
	m_curMap->m_enterExitTimer = 0.f;
}

void Game::GoToPreviousMap()
{

}

Map* Game::GetCurrentMap() const
{
	return m_curMap;
}

int& Game::GetNumOfReinforcements()
{
	return m_numOfReinforcements;
}

void Game::LoadMapDefinitions()
{
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Reading map deinitions..." );
	m_mapDefinitions.resize( 50 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/MapDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document MapDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "MapDefinitions" ), "Syntax Error! Name of the root of MapDefinitions.xml should be \"MapDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	int i = 0;
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "MapDefinition" ), "Syntax Error! Names of the elements of MapDefinitions.xml should be \"MapDefinition\" " );
		m_mapDefinitions[i].m_mapName = ParseXmlAttribute( *xmlIter, "name", "Default" );
		m_mapDefinitions[i].m_dimensions = ParseXmlAttribute( *xmlIter, "dimensions", IntVec2( -1, -1 ) );
		GUARANTEE_OR_DIE( m_mapDefinitions[i].m_dimensions.x + m_mapDefinitions[i].m_dimensions.y >= 12,
			Stringf( "Syntax Error! Map cannot be constructed by dimensions: %i, %i " , 
				m_mapDefinitions[i].m_dimensions.x, m_mapDefinitions[i].m_dimensions.y ) );
		m_mapDefinitions[i].m_fillTile = ParseXmlAttribute( *xmlIter, "fillTileType", "Default" );
		m_mapDefinitions[i].m_borderTile = ParseXmlAttribute( *xmlIter, "borderTileType", "Default" );
		m_mapDefinitions[i].m_bunkerWallTile = ParseXmlAttribute( *xmlIter, "bunkerWallTileType", "Default" );
		m_mapDefinitions[i].m_bunkerFloorTile = ParseXmlAttribute( *xmlIter, "bunkerFloorTileType", "Default" );
		m_mapDefinitions[i].m_worm1Tile = ParseXmlAttribute( *xmlIter, "worm1TileType", "Default" );
		m_mapDefinitions[i].m_worm1Count = ParseXmlAttribute( *xmlIter, "worm1Count", 0 );
		m_mapDefinitions[i].m_worm1MaxLength = ParseXmlAttribute( *xmlIter, "worm1MaxLength", 0 );
		m_mapDefinitions[i].m_worm2Tile = ParseXmlAttribute( *xmlIter, "worm2TileType", "Default" );
		m_mapDefinitions[i].m_worm2Count = ParseXmlAttribute( *xmlIter, "worm2Count", 0 );
		m_mapDefinitions[i].m_worm2MaxLength = ParseXmlAttribute( *xmlIter, "worm2MaxLength", 0 );
		m_mapDefinitions[i].m_worm3Tile = ParseXmlAttribute( *xmlIter, "worm3TileType", "Default" );
		m_mapDefinitions[i].m_worm3Count = ParseXmlAttribute( *xmlIter, "worm3Count", 0 );
		m_mapDefinitions[i].m_worm3MaxLength = ParseXmlAttribute( *xmlIter, "worm3MaxLength", 0 );

		m_mapDefinitions[i].m_numOfLeo = ParseXmlAttribute( *xmlIter, "numOfLeo", 0 );
		m_mapDefinitions[i].m_numOfAries = ParseXmlAttribute( *xmlIter, "numOfAries", 0 );
		m_mapDefinitions[i].m_numOfScorpio = ParseXmlAttribute( *xmlIter, "numOfScorpio", 0 );
		m_mapDefinitions[i].m_numOfCapricorn = ParseXmlAttribute( *xmlIter, "numOfCapricorn", 0 );
		m_mapDefinitions[i].m_numOfCancer = ParseXmlAttribute( *xmlIter, "numOfCancer", 0 );

		m_mapDefinitions[i].m_mapImageName = ParseXmlAttribute( *xmlIter, "mapImageName", "Default" );
		m_mapDefinitions[i].m_mapImageOffset = ParseXmlAttribute( *xmlIter, "mapImageOffset", IntVec2( 0, 0 ) );
		m_mapDefinitions[i].m_startEntityImagePath = ParseXmlAttribute( *xmlIter, "startEntityImagePath", "Default" );
		
		m_mapDefinitions[i].m_addReinforcements = ParseXmlAttribute( *xmlIter, "addReinforcements", 0 );
		// deal with reinforcements
		char const* reinf = xmlIter->Attribute( "reinforcements" );
		if (reinf) {
			m_mapDefinitions[i].m_reinforcements.reserve( (size_t)m_mapDefinitions[i].m_addReinforcements * 4 );
			Strings reinfs;
			reinfs.reserve( m_mapDefinitions[i].m_addReinforcements );
			int num = SplitStringOnDelimiter( reinfs, reinf, ';' );
			for (int j = 0; j < num; j++) {
				Strings singleReinByType;
				singleReinByType.reserve( 4 );
				int numOfEntityTypes = SplitStringOnDelimiter( singleReinByType, reinfs[j], ',' );
				GUARANTEE_OR_DIE( numOfEntityTypes == 4, Stringf( "Num of type in reinforcements do not match 4! %i", numOfEntityTypes ) );
				for (int k = 0; k < 4; k++) {
					m_mapDefinitions[i].m_reinforcements.push_back( stoi( singleReinByType[k] ) );
				}
			}
		}

		m_mapDefinitions[i].m_task1Type = ParseXmlAttribute( *xmlIter, "task1Type", "DEFAULT" );
		m_mapDefinitions[i].m_task1Time = ParseXmlAttribute( *xmlIter, "task1Time", 0.f );

		m_mapDefinitions[i].m_enemyReinforcements = ParseXmlAttribute( *xmlIter, "enemyReinforcements", 0 );
		// deal with enemy reinforcements
		char const* enemyReinfTime = xmlIter->Attribute( "enemyReinfTime" );
		char const* enemyReinfPos = xmlIter->Attribute( "enemyReinfMapPos" );
		char const* enemyReinfDetails = xmlIter->Attribute( "enemyReinfDetails" );
		if (m_mapDefinitions[i].m_enemyReinforcements > 0 && enemyReinfTime && enemyReinfPos && enemyReinfDetails) {
			Strings splitedReinfTime;
			int numOfTimes = SplitStringOnDelimiter( splitedReinfTime, enemyReinfTime, ';' );
			Strings splitedReinfPos;
			int numOfPos = SplitStringOnDelimiter( splitedReinfPos, enemyReinfPos, ';' );
			Strings splitedReinfDetails;
			int numOfDetails = SplitStringOnDelimiter( splitedReinfDetails, enemyReinfDetails, ';' );
			GUARANTEE_OR_DIE( numOfDetails == m_mapDefinitions[i].m_enemyReinforcements && numOfDetails == numOfPos && numOfPos == numOfTimes, std::string( "Cannot parse enemy reinforcements information!" ) );
			std::vector<EnemyReinfDef>& defs = m_mapDefinitions[i].m_enemyReinfWaveDefs;
			defs.resize( m_mapDefinitions[i].m_enemyReinforcements );
			for (int j = 0; j < m_mapDefinitions[i].m_enemyReinforcements; j++) {
				defs[j].m_tileToCome.SetFromText( splitedReinfPos[j].c_str() );
				defs[j].m_timeToCome = stof( splitedReinfTime[j] );
				Strings singleReinfDetails;
				int numOfRes = SplitStringOnDelimiter( singleReinfDetails, splitedReinfDetails[j], ',' );
				GUARANTEE_OR_DIE( numOfRes == 4, std::string( "Cannot parse enemy reinforcements information!" ) );
				defs[j].m_numOfAries = stoi( singleReinfDetails[0] );
				defs[j].m_numOfLeo = stoi( singleReinfDetails[1] );
				defs[j].m_numOfCapricorn = stoi( singleReinfDetails[2] );
				defs[j].m_numOfCancer = stoi( singleReinfDetails[3] );
			}
		}

		xmlIter = xmlIter->NextSiblingElement();
		i++;
		if (i == 50) {
			g_devConsole->AddLine( DevConsole::INFO_ERROR, "Cannot read more than 50 maps!" );
			break;
		}
	}
}

void Game::HandleKey()
{
	if (g_theInput->WasKeyJustPressed( KEYCODE_F9 )) {
		GoToNextMap();
	}
	if (!g_theApp->m_isPaused) {
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 1.f );
	}
	if (g_theInput->WasKeyJustPressed( 'T' )) {
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 0.5f );
	}
	if (g_theInput->WasKeyJustReleased( 'T' )) {
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 1.f );
	}
	if (g_theInput->WasKeyJustPressed( 'Y' )) {
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 2.f );
	}
	if (g_theInput->WasKeyJustReleased( 'Y' )) {
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 1.f );
	}
	if (g_theApp->m_isPaused) {
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 0.f );
	}
}

void Game::CallBackGoToNextMap()
{
	PlayerTank* player = m_curMap->GetPlayer();
	m_curMap->RemoveEntityFromMap( (Entity&)(*player) );
	m_curMap = m_maps[m_curMapIndex];
	m_curMap->AddEntityToMap( (Entity&)(*player) );
	player->TransferToMap( m_curMap );
	m_curMap->m_curMapState = MapState::ENTER_MAP;
	m_curMap->m_enterExitTimer = 0.f;
	m_numOfReinforcements += m_curMap->GetMapDef().m_addReinforcements;
}

MapDefinition const& Game::GetMapDef( std::string const& name ) const
{
	for (MapDefinition const& def : m_mapDefinitions) {
		if (name == def.m_mapName) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find map definition \"%s\", check spaces or spell!", name.c_str() ) );
}

