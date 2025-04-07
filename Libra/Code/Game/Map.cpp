#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Leo.hpp"
#include "Game/Ray.hpp"
#include "Game/Bullet.hpp"
#include "Game/Aries.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Capricorn.hpp"
#include "Game/Cancer.hpp"
#include "Game/Explosion.hpp"
#include "Game/Building.hpp"
#include "Game/Rubble.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Core/Image.hpp"

Map::Map()
{
	m_deadScreen = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/YouDiedScreen.png" );
	m_winScreen = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/VictoryScreen.jpg" );
	m_tileSpriteTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	m_tileSpriteSheet = new SpriteSheet( *m_tileSpriteTexture, IntVec2( 8, 8 ) );
}

Map::~Map()
{
	for (int i = 0; i < (int)EntityType::NUM; i++) {
		for (Entity* entity : m_entityListsByType[i]) {
			delete entity;
		}
	}
	delete m_tileSpriteSheet;
	delete m_startToEndHeatMap;
	delete m_solidTileHeatMap;
	delete m_amphibiousTileHeatMap;
}

void Map::Startup( MapDefinition const& config )
{
	m_mapDef = &config;
	// record task type
	if (config.m_task1Type == "Clear") {
		m_taskType = TaskType::Clear;
	}
	else if (config.m_task1Type == "Breakthrough") {
		m_taskType = TaskType::Breakthrough;
	}
	else if (config.m_task1Type == "Capture") {
		m_taskType = TaskType::Capture;
	}
	else if (config.m_task1Type == "Defense") {
		m_taskType = TaskType::Defense;
	}
	m_timerForTask = config.m_task1Time;

	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( NUM_TILES_IN_X_AXIS, NUM_TILES_IN_X_AXIS / g_window->GetAspect() ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;

	// reserve for entity array
	for (int i = 0; i < (int)EntityType::NUM; i++) {
		if (i == (int)EntityType::_BULLET || i == (int)EntityType::_BOLT || i == (int)EntityType::_EXPLOSION 
			|| i == (int)EntityType::_GUIDED_BULLET) {
			m_entityListsByType[i].reserve( 500 );
		}
		else {
			m_entityListsByType[i].reserve( 30 );
		}
	}
	m_allEntityList.reserve( 1000 );
	for (int i = 0; i < (int)EntityFaction::NUM; i++) {
		m_actorListsByFaction->reserve( 50 );
	}

	PopulateMap( config );
	LoadEnemy( config );

	g_devConsole->AddLine( DevConsole::INFO_MINOR, Stringf( "Successfully create map %s", config.m_mapName.c_str() ) );
}

void Map::Update( float deltaTime )
{
	m_timerFromStart += deltaTime;
	if (m_taskType == TaskType::Defense || m_taskType == TaskType::Capture) {
		m_timerForTask -= deltaTime;
	}
	UpdateState( deltaTime );
	HandleKeys();
	if (m_curMapState == MapState::PLAYING) {
		UpdateEntityLists( deltaTime );
		AllEntitiesCorrectCollisionWithEachOther( deltaTime );
		AllEntitiesCorrectCollisionWithWall();
		CheckBulletCollision();

		DeleteGarbage();
		UpdateCamera();
		HandleTasks();
	}
}

void Map::Render() const
{
	g_theRenderer->BeginCamera( m_worldCamera );
	if (m_curMapState == MapState::PLAYING || m_curMapState == MapState::PLAYER_DEAD || m_curMapState == MapState::ENTER_MAP
		|| m_curMapState == MapState::EXIT_MAP || m_curMapState == MapState::FINISH_EXIT) {
		RenderTilemap();
		RenderEntityLists();
		RenderEnterExitMode();
	}

	if (g_theApp->m_isPaused) {
		RenderPauseMode();
	}
	g_theRenderer->EndCamera( m_worldCamera );
	// UI camera
	g_theRenderer->BeginCamera( m_screenCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_screenCamera );
}

MapDefinition const& Map::GetMapDef() const
{
	return *m_mapDef;
}

void Map::PopulateMap( MapDefinition const& config )
{
	TileDefinition const& borderTileDef = GetTileDef( config.m_borderTile );
	TileDefinition const& fillTileDef = GetTileDef( config.m_fillTile );

	m_dimensions = config.m_dimensions;
	m_startToEndHeatMap = new TileHeatMap( m_dimensions );
	m_exitPoint = Vec2( (float)m_dimensions.x - 1.5f, (float)m_dimensions.y - 1.5f );
	// generate Tiles in tile array
	m_tiles.resize( (size_t)m_dimensions.x * m_dimensions.y );

	int tryTimes = 0;
	do {
		// fill all tiles with fill tile
		for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++) {
			IntVec2 coord = GetXYPosFromArrayIndex( i );
			ChangeTileType( coord, fillTileDef );
			m_tiles[i].m_coords = coord;
		}

		// grow worm in tiles
		GrowWorm( config.m_worm1Tile, config.m_worm1Count, config.m_worm1MaxLength );
		GrowWorm( config.m_worm2Tile, config.m_worm2Count, config.m_worm2MaxLength );
		GrowWorm( config.m_worm3Tile, config.m_worm3Count, config.m_worm3MaxLength );

		// if has image then draw image
		if (strcmp( config.m_mapImageName.c_str(), "Default" )) {
			Image image( config.m_mapImageName.c_str() );
			IntVec2 dimensions = image.GetDimensions();
			bool spawnedBuilding = false;
			for (int j = 0; j < dimensions.y; j++) {
				for (int i = 0; i < dimensions.x; i++) {
					if (IsIndexInBound( IntVec2( i, j ) + config.m_mapImageOffset ) && !(image.GetTexelColor( IntVec2( i, j ) ) == Rgba8(255, 255, 255, 255))) {
						if (image.GetTexelColor( IntVec2( i, j ) ) == Rgba8( 153, 204, 255 )) {
							if (!spawnedBuilding && m_taskType == TaskType::Defense) {
								spawnedBuilding = true;
								m_theBuilding = SpawnNewEntity( EntityType::_BUILDING, GetWorldPosFromMapPosLeftBottom( IntVec2( i + 1, j + 1 ) + config.m_mapImageOffset ), EntityFaction::FACTION_GOOD );
							}
						}
						else if (m_taskType == TaskType::Capture && image.GetTexelColor( IntVec2( i, j ) ) == Rgba8( 0, 0, 255 )) {
							m_captureCenter = GetWorldPosFromMapPosCenter( IntVec2( i, j ) );
						}
						else {
							unsigned char rnd = (unsigned char)(g_theGame->m_randNumGen->RollRandomIntLessThan( 255 ));
							if (rnd < image.GetTexelColor( IntVec2( i, j ) ).a) {
								ChangeTileType( IntVec2( i, j ) + config.m_mapImageOffset, GetTileDef( image.GetTexelColor( IntVec2( i, j ) ) ) );
							}
						}
					}
					else if (!IsIndexInBound( IntVec2( i, j ) + config.m_mapImageOffset )) {
						g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "Image %s with offset IntVec2(%i, %i) is out of bounds!", config.m_mapImageName.c_str(), config.m_mapImageOffset.x, config.m_mapImageOffset.y ) );
					}
				}
			}
		}

		// set up bunker floor and border
		for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++) {
			IntVec2 coord = GetXYPosFromArrayIndex( i );
			if (coord.x != 0 && coord.x != m_dimensions.x - 1 && coord.y != 0 && coord.y != m_dimensions.y - 1) {
				if ((coord.x <= 5 && coord.y <= 5) || (m_taskType == TaskType::Breakthrough && (coord.x >= m_dimensions.x - 5) && (coord.y >= m_dimensions.y - 5))) {
					ChangeTileType( coord, GetTileDef( config.m_bunkerFloorTile ) );
				}
			}
			else {
				ChangeTileType( coord, borderTileDef );
			}
		}

		// set up entrance, exit and bunker wall
		ChangeTileType( 1, 1, GetTileDef( "ENTRANCE" ) );
		ChangeTileType( 4, 2, GetTileDef( config.m_bunkerWallTile ) );
		ChangeTileType( 4, 3, GetTileDef( config.m_bunkerWallTile ) );
		ChangeTileType( 4, 4, GetTileDef( config.m_bunkerWallTile ) );
		ChangeTileType( 2, 4, GetTileDef( config.m_bunkerWallTile ) );
		ChangeTileType( 3, 4, GetTileDef( config.m_bunkerWallTile ) );
		if (m_taskType == TaskType::Breakthrough) {
			ChangeTileType( m_dimensions.x - 5, m_dimensions.y - 3, GetTileDef( config.m_bunkerWallTile ) );
			ChangeTileType( m_dimensions.x - 5, m_dimensions.y - 4, GetTileDef( config.m_bunkerWallTile ) );
			ChangeTileType( m_dimensions.x - 5, m_dimensions.y - 5, GetTileDef( config.m_bunkerWallTile ) );
			ChangeTileType( m_dimensions.x - 4, m_dimensions.y - 5, GetTileDef( config.m_bunkerWallTile ) );
			ChangeTileType( m_dimensions.x - 3, m_dimensions.y - 5, GetTileDef( config.m_bunkerWallTile ) );
			ChangeTileType( m_dimensions.x - 2, m_dimensions.y - 2, GetTileDef( "EXIT" ) );
		}

		// calculate distance field tile heat map
		PopulateHeatMapDistanceField( *m_startToEndHeatMap, IntVec2( 1, 1 ), FLT_MAX, 0.f, true );
		++tryTimes;
	} while (m_startToEndHeatMap->GetTileValue( m_dimensions - IntVec2( 2, 2 ) ) == FLT_MAX && tryTimes <= 100);

	if (m_taskType == TaskType::Defense && m_theBuilding == nullptr) {
		ERROR_AND_DIE( Stringf( "No building to defend in map name: %s", config.m_mapName.c_str() ) );
	}
	
	if (tryTimes > 100) {
		ERROR_AND_DIE( Stringf( "Cannot generate map by config %s!!!!!", config.m_mapName.c_str() ) );
	}

	// change all unreachable tiles to border tile
	for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++) {
		IntVec2 coord = GetXYPosFromArrayIndex( i );
		if (m_startToEndHeatMap->GetTileValue( coord ) == FLT_MAX && !IsTileSolid( coord ) && !IsTileWater( coord )) {
			ChangeTileType( coord, borderTileDef );
		}
	}

	// build following tile heat maps
	m_solidTileHeatMap = new TileHeatMap( m_dimensions );
	m_amphibiousTileHeatMap = new TileHeatMap( m_dimensions );
	BuildSolidTileHeatMap();
	BuildAmphibiousTileHeatMap();
}

void Map::LoadEnemy( MapDefinition const& config )
{
	// set up player tank
	// m_playerTank = (PlayerTank*)SpawnNewEntity( EntityType::_GOOD_PLAYER, Vec2( 1.5f, 1.5f ) );
	// 
	// a pointer pool of empty tiles
	std::vector<Tile*> emptyTiles;
	emptyTiles.reserve( (size_t)m_dimensions.x * (size_t)m_dimensions.y );
	for (Tile& t : m_tiles) {
		if (!IsTileSolid( t.m_coords ) && !IsTileWater( t.m_coords ) && !(t.m_coords.x <= 5 && t.m_coords.y <= 5) && !(t.m_coords.x >= m_dimensions.x - 7 && t.m_coords.y >= m_dimensions.y - 7)) {
			emptyTiles.push_back( &t );
		}
	}
	for (int i = 0; i < config.m_numOfScorpio; i++) {
		if (emptyTiles.empty()) {
			return;
		}
		int tileIndex = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)emptyTiles.size() );
		SpawnNewEntity( EntityType::_SCORPIO, Vec2( emptyTiles[tileIndex]->m_coords.x + 0.5f, emptyTiles[tileIndex]->m_coords.y + 0.5f ), EntityFaction::FACTION_EVIL );
		emptyTiles.erase( emptyTiles.begin() + tileIndex );
	}
	for (int i = 0; i < config.m_numOfLeo; i++) {
		if (emptyTiles.empty()) {
			return;
		}
		int tileIndex = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)emptyTiles.size() );
		SpawnNewEntity( EntityType::_LEO, Vec2( emptyTiles[tileIndex]->m_coords.x + 0.5f, emptyTiles[tileIndex]->m_coords.y + 0.5f ), EntityFaction::FACTION_EVIL );
		emptyTiles.erase( emptyTiles.begin() + tileIndex );
	}
	// test for 45 degrees
	/*SpawnNewEntity(EntityType::_EVIL_SCORPIO, Vec2(2 + 0.5f, 2 + 0.5f));
	SpawnNewEntity( EntityType::_EVIL_SCORPIO, Vec2( 3 + 0.5f, 3 + 0.5f ) );
	SpawnNewEntity( EntityType::_EVIL_SCORPIO, Vec2( 6 + 0.5f, 6 + 0.5f ) );
	SpawnNewEntity( EntityType::_EVIL_SCORPIO, Vec2( 7 + 0.5f, 7 + 0.5f ) );*/
	for (int i = 0; i < config.m_numOfAries; i++) {
		if (emptyTiles.empty()) {
			return;
		}
		int tileIndex = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)emptyTiles.size() );
		SpawnNewEntity( EntityType::_ARIES, Vec2( emptyTiles[tileIndex]->m_coords.x + 0.5f, emptyTiles[tileIndex]->m_coords.y + 0.5f ), EntityFaction::FACTION_EVIL );
		emptyTiles.erase( emptyTiles.begin() + tileIndex );
	}
	for (int i = 0; i < config.m_numOfCapricorn; i++) {
		if (emptyTiles.empty()) {
			return;
		}
		int tileIndex = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)emptyTiles.size() );
		SpawnNewEntity( EntityType::_CAPRICORN, Vec2( emptyTiles[tileIndex]->m_coords.x + 0.5f, emptyTiles[tileIndex]->m_coords.y + 0.5f ), EntityFaction::FACTION_EVIL );
		emptyTiles.erase( emptyTiles.begin() + tileIndex );
	}
	for (int i = 0; i < config.m_numOfCancer; i++) {
		if (emptyTiles.empty()) {
			return;
		}
		int tileIndex = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)emptyTiles.size() );
		SpawnNewEntity( EntityType::_CANCER, Vec2( emptyTiles[tileIndex]->m_coords.x + 0.5f, emptyTiles[tileIndex]->m_coords.y + 0.5f ), EntityFaction::FACTION_EVIL );
		emptyTiles.erase( emptyTiles.begin() + tileIndex );
	}
	for (int i = 0; i < 10; i++) {
		if (emptyTiles.empty()) {
			return;
		}
		int tileIndex = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)emptyTiles.size() );
		SpawnNewEntity( EntityType::_RUBBLE, GetWorldPosFromMapPosCenter( emptyTiles[tileIndex]->m_coords ), EntityFaction::FACTION_NEUTRAL );
		emptyTiles.erase( emptyTiles.begin() + tileIndex );
	}

	// if has image then put entity
	if (strcmp( config.m_startEntityImagePath.c_str(), "Default" )) {
		Image image( config.m_startEntityImagePath.c_str() );
		IntVec2 dimensions = image.GetDimensions();
		for (int j = 0; j < dimensions.y; j++) {
			for (int i = 0; i < dimensions.x; i++) {
				if (IsIndexInBound( IntVec2( i, j ) + config.m_mapImageOffset )) {
					Rgba8 const& info = image.GetTexelColor( IntVec2( i, j ) );
					if (info == ENEMY_SCORPIO) {
						SpawnNewEntity( EntityType::_SCORPIO, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_EVIL );
					}
					else if (info == ALLY_SCORPIO) {
						SpawnNewEntity( EntityType::_SCORPIO, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_GOOD );
					}
				}
			}
		}

		for (int j = 0; j < dimensions.y; j++) {
			for (int i = 0; i < dimensions.x; i++) {
				if (IsIndexInBound( IntVec2( i, j ) + config.m_mapImageOffset )) {
					Rgba8 const& info = image.GetTexelColor( IntVec2( i, j ) );
					if (info == ENEMY_LEO) {
						SpawnNewEntity( EntityType::_LEO, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_EVIL );
					}
					else if (info == ENEMY_ARIES) {
						SpawnNewEntity( EntityType::_ARIES, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_EVIL );
					}
					else if (info == ENEMY_CAPRICORN) {
						SpawnNewEntity( EntityType::_CAPRICORN, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_EVIL );
					}
					else if (info == ENEMY_CANCER) {
						SpawnNewEntity( EntityType::_CANCER, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_EVIL );
					}
					else if (info == ALLY_LEO) {
						SpawnNewEntity( EntityType::_LEO, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_GOOD );
					}
					else if (info == ALLY_ARIES) {
						SpawnNewEntity( EntityType::_ARIES, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_GOOD );
					}
					else if (info == ALLY_CAPRICORN) {
						SpawnNewEntity( EntityType::_CAPRICORN, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_GOOD );
					}
					else if (info == ALLY_CANCER) {
						SpawnNewEntity( EntityType::_CANCER, GetWorldPosFromMapPosCenter( IntVec2( i, j ) ), EntityFaction::FACTION_GOOD );
					}
				}
			}
		}
	}
}

void Map::GrowWorm( std::string const& tileType, int wormCount, int wormMaxLength )
{
	if (tileType == "Default") {
		return;
	}
	TileDefinition const& tileDef = GetTileDef( tileType );
	for (int i = 0; i < wormCount; i++) {
		int curPointX = g_theGame->m_randNumGen->RollRandomIntInRange( 1, m_dimensions.x - 2 );
		int curPointY = g_theGame->m_randNumGen->RollRandomIntInRange( 1, m_dimensions.y - 2 );
		for (int j = 0; j < wormMaxLength; j++) {
			ChangeTileType( curPointX, curPointY, tileDef );
			float rnd = g_theGame->m_randNumGen->RollRandomFloatZeroToOne();
			if (rnd < 0.25f) {
				curPointX++;
				if (curPointX >= m_dimensions.x - 1) {
					break;
				}
			}
			else if (rnd < 0.5f) {
				curPointX--;
				if (curPointX <= 0) {
					break;
				}
			}
			else if (rnd < 0.75f) {
				curPointY--;
				if (curPointY <= 0) {
					break;
				}
			}
			else {
				curPointY++;
				if (curPointY >= m_dimensions.y - 1) {
					break;
				}
			}
		}
	}
}

void Map::UpdateState( float deltaTime )
{
	if (m_curMapState == MapState::ENTER_MAP) {
		m_enterExitTimer += deltaTime;
		if (m_enterExitTimer >= 1.f) {
			m_curMapState = MapState::PLAYING;
			m_enterExitTimer = 0.f;
		}
	}
	else if (m_curMapState == MapState::PLAYING) {
		if (!(m_playerTank->IsAlive())) {
			m_toRespwanModeTime -= deltaTime;
		}
		if (m_toRespwanModeTime <= 0.f) {
			m_toRespwanModeTime = 3.f;
			m_curMapState = MapState::PLAYER_DEAD;
		}
	}
	else if (m_curMapState == MapState::EXIT_MAP) {
		m_enterExitTimer += deltaTime;
		if (m_enterExitTimer > 1.f) {
			m_curMapState = MapState::FINISH_EXIT;
			m_enterExitTimer = 0.f;
		}
	}
}

void Map::UpdateEntityLists( float deltaTime )
{
	for (int i = 0; i < (int)EntityType::NUM; i++) {
		UpdateEntityList( m_entityListsByType[i], deltaTime );
	}
}

void Map::UpdateEntityList( EntityList& entityArray, float deltaTime )
{
	for (Entity* i : entityArray) {
		if (i) {
			i->Update( deltaTime );
		}
	}
}

void Map::UpdateCamera()
{
	if (!m_isHighCameraMode) {
		m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
		m_worldCamera.SetCenter( Vec2( GetClamped( m_playerTank->m_position.x, WORLD_SIZE_X * 0.5f, m_dimensions.x - WORLD_SIZE_X * 0.5f ), GetClamped( m_playerTank->m_position.y, WORLD_SIZE_Y * 0.5f, m_dimensions.y - WORLD_SIZE_Y * 0.5f ) ) );
	}
	else {
		if (m_dimensions.x > g_window->GetAspect() * m_dimensions.y) {
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( (float)m_dimensions.x, (float)m_dimensions.x / g_window->GetAspect() ), 1.f, -1.f );
		}
		else {
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( (float)m_dimensions.y * g_window->GetAspect(), (float)m_dimensions.y ), 1.f, -1.f );
		}
	}

	if (m_doScreenShake) {
		m_worldCamera.Translate2D( Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -0.05f, 0.05f ), g_theGame->m_randNumGen->RollRandomFloatInRange( -0.05f, 0.05f ) ) );
	}
}

void Map::DeleteGarbage()
{
	for (int i = 0; i < (int)EntityType::NUM; i++) {
		EntityList& entityArray = m_entityListsByType[i];
		for (int j = 0; j < (int)entityArray.size(); j++) {
			if (entityArray[j] && entityArray[j]->m_isGarbage) {
				if (entityArray[j]->IsActor()) {
					EntityList& actorListByFaction = m_actorListsByFaction[(int)entityArray[j]->m_faction];
					DeleteGarbageInEntityList( entityArray[j], actorListByFaction );
				}
				delete entityArray[j];
				DeleteGarbageInEntityList( entityArray[j], m_allEntityList );
				entityArray[j] = nullptr;
			}
		}
	}
}

void Map::DeleteGarbageInEntityList( Entity* deletedEntity, EntityList& entityArray )
{
	for (int i = 0; i < (int)entityArray.size(); i++) {
		if (entityArray[i] == deletedEntity) {
			entityArray[i] = nullptr;
		}
	}
}

void Map::RenderEntityLists() const
{
	for (int i = 0; i < (int)EntityType::NUM; i++) {
		RenderEntityList( m_entityListsByType[i] );
	}
}

void Map::RenderEntityList(EntityList const& entityArray) const
{
	for (Entity* i : entityArray) {
		if (i && (i->IsAlive() || i->m_type == EntityType::_FLAME_BULLET)) {
			i->Render();
		}
	}
	for (Entity* i : entityArray) {
		if (i && i->IsAlive()) {
			i->RenderUI();
		}
	}
	if (m_isDebugMode) {
		for (Entity* i : entityArray) {
			if (i) {
				i->DebugRender();
			}
		}
	}
}

void Map::RenderTilemap() const
{
	if (m_mapRenderState == MapRenderState::NORMAL) {
		// prepare the fixed tile map vertex array
		std::vector<Vertex_PCU> tilemapVerts;
		tilemapVerts.reserve( (size_t)6 * m_dimensions.x * m_dimensions.y );
		for (Tile const& i : m_tiles) {
			Vec2 LBPos = GetWorldPosFromMapPosLeftBottom( i.m_coords );
			TileDefinition const& tileDef = *(i.m_tileDefinition);
			AddVertsForAABB2D( tilemapVerts, AABB2( LBPos, LBPos + Vec2( 1.f, 1.f ) ), tileDef.m_tintColor, m_tileSpriteSheet->GetSpriteUVs( tileDef.m_spriteSheetUVIndex ) );
		}
		g_theRenderer->BindTexture( m_tileSpriteTexture );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( tilemapVerts );
	}
	else if (m_mapRenderState == MapRenderState::DISTANCE_MAP) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 * (size_t)m_dimensions.x * m_dimensions.y );
		m_startToEndHeatMap->AddVertsForDebugDraw( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), 
			FloatRange( 0.f, m_startToEndHeatMap->GetMaxValueExceptSpecialValue(FLT_MAX) ) );
		//m_startToEndHeatMap->AddTextVertsForDebugDraw( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), FLT_MAX );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
	else if (m_mapRenderState == MapRenderState::SOLID_MAP) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 * (size_t)m_dimensions.x * m_dimensions.y );
		m_solidTileHeatMap->AddVertsForDebugDraw( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), 
			FloatRange( 0.f, FLT_MAX ), Rgba8( 102, 178, 255 ), Rgba8( 96, 96, 96 ), -1 );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
	else if (m_mapRenderState == MapRenderState::AMPHIBIOUS_MAP) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 * (size_t)m_dimensions.x * m_dimensions.y );
		m_amphibiousTileHeatMap->AddVertsForDebugDraw( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), 
			FloatRange( 0.f, FLT_MAX ), Rgba8( 102, 178, 255 ), Rgba8( 96, 96, 96 ), -1 );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
	else if (m_mapRenderState == MapRenderState::ENEMY_PATH_FINDING_MAP) {
		if (m_allEntityList[m_thisRenderHeatMapEntityIndex]) {
			TileHeatMap const* tileHeatMap = m_allEntityList[m_thisRenderHeatMapEntityIndex]->GetTargetDistanceTileHeatMap();
			if (tileHeatMap) {
				std::vector<Vertex_PCU> verts;
				verts.reserve( 6 * (size_t)m_dimensions.x * m_dimensions.y + 100 );
				tileHeatMap->AddVertsForDebugDraw( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), 
					FloatRange( 0.f, tileHeatMap->GetMaxValueExceptSpecialValue( FLT_MAX ) ) );
				//tileHeatMap->AddTextVertsForDebugDraw( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), FLT_MAX );
				AddVertsForArrow2D( verts, m_allEntityList[m_thisRenderHeatMapEntityIndex]->m_position + Vec2( 100.f, 100.f ), m_allEntityList[m_thisRenderHeatMapEntityIndex]->m_position, 1.f, 0.1f, Rgba8( 255, 0, 0 ) );
				g_theRenderer->BindTexture( nullptr );
				g_theRenderer->SetModelConstants();
				g_theRenderer->DrawVertexArray( verts );
			}
		}
	}
}

void Map::RenderUI() const
{
	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 1000 );
	if (m_curMapState == MapState::PLAYER_DEAD) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 );
		AddVertsForAABB2D( verts, m_screenCamera.m_cameraBox, Rgba8( 255, 255, 255, 255 ) );
		g_theRenderer->BindTexture( m_deadScreen );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
	}
	if (m_curMapState == MapState::WIN) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 );
		AddVertsForAABB2D( verts, m_screenCamera.m_cameraBox, Rgba8( 255, 255, 255, 255 ) );
		g_theRenderer->BindTexture( m_winScreen );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
	}

	if (m_mapRenderState == MapRenderState::DISTANCE_MAP) {
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 600.f, 760.f ), 30.f, Stringf( "Distance Map/F6 Next Map" ), Rgba8( 255, 0, 0 ), 0.6f );
	}
	else if (m_mapRenderState == MapRenderState::SOLID_MAP) {
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 600.f, 760.f ), 30.f, Stringf( "Solid/Water Tile Map/F6 Next Map" ), Rgba8( 255, 0, 0 ), 0.6f );
	}
	else if (m_mapRenderState == MapRenderState::AMPHIBIOUS_MAP) {
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 600.f, 760.f ), 30.f, Stringf( "Amphibious Map/F6 Next Map" ), Rgba8( 255, 0, 0 ), 0.6f );
	}
	else if (m_mapRenderState == MapRenderState::ENEMY_PATH_FINDING_MAP) {
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 600.f, 760.f ), 30.f, Stringf( "Enemy Path Finding Map/F6 Next Map/N Next Enemy" ), Rgba8( 255, 0, 0 ), 0.6f );
	}

	if (m_playerTank->m_canCallReinforcements) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 400.f, 30.f ), Vec2( 1200.f, 60.f ) ), 30.f, std::string( "B to call reinforcements" ), Rgba8( 102, 178, 255, 255 ), 0.6f );
	}

	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 1290.f, 720.f ), 30.f, Stringf( "Reinforcements:%i", g_theGame->GetNumOfReinforcements() ), Rgba8( 51, 51, 200 ), 0.6f );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 1470.f, 760.f ), 30.f, Stringf( "FPS:%i", g_theApp->m_framePerSecond ), Rgba8( 255, 0, 0 ), 0.6f );
	if (m_taskType == TaskType::Breakthrough) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 760.f ), Vec2( 350.f, 790.f ) ), 30.f, std::string( "Task: Go to north east corner" ), Rgba8::RED, 0.6f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	}
	else if (m_taskType == TaskType::Capture) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 760.f ), Vec2( 350.f, 790.f ) ), 30.f, std::string( Stringf( "Task: Capture the star point and defend it for %02i:%02i", (RoundDownToInt( m_timerForTask ) + 1) / 60, (RoundDownToInt( m_timerForTask ) + 1) % 60 ) ), Rgba8::RED, 0.6f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	}
	else if (m_taskType == TaskType::Clear) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 760.f ), Vec2( 350.f, 790.f ) ), 30.f, std::string( "Task: Kill all enemies" ), Rgba8::RED, 0.6f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	}
	else if (m_taskType == TaskType::Defense) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 760.f ), Vec2( 350.f, 790.f ) ), 30.f, std::string( Stringf("Task: Defend the blue building for %02i:%02i", (RoundDownToInt( m_timerForTask ) + 1) / 60, (RoundDownToInt( m_timerForTask ) + 1) % 60) ), Rgba8::RED, 0.6f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	}
	
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void Map::HandleKeys()
{
	XboxController& controller = g_theInput->GetController( 0 );
#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) {
		m_isDebugMode = !m_isDebugMode;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F4 )) {
		m_isHighCameraMode = !m_isHighCameraMode;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F3 )) {
		m_isNoClipMode = !m_isNoClipMode;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F6 )) {
		m_mapRenderState = (MapRenderState)(((int)m_mapRenderState + 1) % (int)MapRenderState::NUM);
		m_thisRenderHeatMapEntityIndex = 0;
		if (m_mapRenderState == MapRenderState::ENEMY_PATH_FINDING_MAP) {
			for (int i = 0; i < (int)m_allEntityList.size(); i++) {
				if (m_allEntityList[i] && m_allEntityList[i]->IsAlive() && m_allEntityList[i]->GetTargetDistanceTileHeatMap()) {
					m_thisRenderHeatMapEntityIndex = i;
					break;
				}
			}
		}
	}
	if (m_curMapState != MapState::PLAYER_DEAD && m_mapRenderState == MapRenderState::ENEMY_PATH_FINDING_MAP && g_theInput->WasKeyJustPressed( 'N' )) {
		for (int i = 1; i < (int)m_allEntityList.size(); i++) {
			int index = (i + m_thisRenderHeatMapEntityIndex) % (int)m_allEntityList.size();
			if (m_allEntityList[index] && m_allEntityList[index]->IsAlive() &&
				m_allEntityList[index]->GetTargetDistanceTileHeatMap()) {
				m_thisRenderHeatMapEntityIndex = index;
				break;
			}
		}
	}
	//if (g_theInput->WasKeyJustPressed( 'V' )) {
	//	SpawnNewEntity( EntityType::_CANCER, m_enterPoint, EntityFaction::FACTION_GOOD, nullptr, 45.f );
	//}
#endif
	if ((g_theInput->WasKeyJustPressed( 'N' ) || controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START)) && m_curMapState == MapState::PLAYER_DEAD) {
		m_curMapState = MapState::PLAYING;
		if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )) {
			g_theApp->m_isPaused = !(g_theApp->m_isPaused);
		}
		m_playerTank->Reborn();
	}
	if (m_curMapState == MapState::WIN && ((g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->WasKeyJustPressed( 'N' ) || g_theInput->WasKeyJustPressed( KEYCODE_ESC ) || controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START ) || controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK ) || controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A )))) {
		g_theApp->ToAttractMode();
	}
}

void Map::HandleTasks()
{
	if (m_curEnemyReinforcements < (int)(m_mapDef->m_enemyReinfWaveDefs.size()) && m_timerFromStart >= m_mapDef->m_enemyReinfWaveDefs[m_curEnemyReinforcements].m_timeToCome) {
		SpawnEnemyReinforcemets( m_mapDef->m_enemyReinfWaveDefs[m_curEnemyReinforcements] );
		++m_curEnemyReinforcements;
	}
	if (m_taskType == TaskType::Breakthrough) {
		if (m_playerTank->IsAlive() && IsPointInsideDisc2D( m_exitPoint, m_playerTank->m_position, m_playerTank->m_physicsRadius )) {
			g_devConsole->AddLine( DevConsole::INFO_MINOR, "Task breakthrough completed!" );
			g_theApp->PlaySound( AudioName::ExitMap );
			g_theGame->GoToNextMap();
		}
	}
	else if (m_taskType == TaskType::Clear) {
		if (m_playerTank->IsAlive() && !GetNearestEnemyActor( m_playerTank )) {
			g_devConsole->AddLine( DevConsole::INFO_MINOR, "Task clear region completed!" );
			g_theApp->PlaySound( AudioName::ExitMap );
			g_theGame->GoToNextMap();
		}
	}
	else if (m_taskType == TaskType::Defense) {
		if (m_playerTank->IsAlive() && m_timerForTask < 0.f) {
			g_devConsole->AddLine( DevConsole::INFO_MINOR, "Task defense important point completed!" );
			g_theApp->PlaySound( AudioName::ExitMap );
			g_theGame->GoToNextMap();
		}
		if (m_theBuilding && !m_theBuilding->IsAlive() && m_playerTank->IsAlive()) {
			g_devConsole->AddLine( DevConsole::INFO_MINOR, "Task defense important point failed!" );
			m_playerTank->Die();
		}
	}
	else if (m_taskType == TaskType::Capture) {
		if (m_playerTank->IsAlive() && m_timerForTask < 0.f) {
			g_devConsole->AddLine( DevConsole::INFO_MINOR, "Task capture a place completed!" );
			g_theApp->PlaySound( AudioName::ExitMap );
			g_theGame->GoToNextMap();
		}
		if (!m_playerTank->IsAlive()) {
			g_devConsole->AddLine( DevConsole::INFO_MINOR, "Task defense important point failed!" );
			m_playerTank->Die();
		}
	}
}

void Map::PopulateHeatMapDistanceField( TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost, float minCost, bool treatWaterAsSolid, bool treatScorpioAsSolid ) const
{
	// bfs
	int maxQueueLength = m_dimensions.x * m_dimensions.y;
	IntVec2* queue = new IntVec2[maxQueueLength];
	int start = 0;
	int end = 1;
	out_distanceField.SetAllValues( maxCost );
	queue[start] = startCoords;
	out_distanceField.SetTileValue( startCoords, minCost );
	while (start < end) {
		IntVec2 const& thisTile = queue[start];
		float thisTileValue = out_distanceField.GetTileValue( thisTile );
		if (thisTile.x >= 1) {
			IntVec2 nextTile = thisTile + IntVec2( -1, 0 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost 
				&& !IsTileSolid( nextTile ) && !(treatWaterAsSolid && IsTileWater( nextTile )) && !(treatScorpioAsSolid && IsTileScorpio( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.y >= 1) {
			IntVec2 nextTile = thisTile + IntVec2( 0, -1 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost 
				&& !IsTileSolid( nextTile ) && !(treatWaterAsSolid && IsTileWater( nextTile )) && !(treatScorpioAsSolid && IsTileScorpio( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.x < m_dimensions.x - 1) {
			IntVec2 nextTile = thisTile + IntVec2( 1, 0 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost 
				&& !IsTileSolid( nextTile ) && !(treatWaterAsSolid && IsTileWater( nextTile )) && !(treatScorpioAsSolid && IsTileScorpio( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.y < m_dimensions.y - 1) {
			IntVec2 nextTile = thisTile + IntVec2( 0, 1 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost 
				&& !IsTileSolid( nextTile ) && !(treatWaterAsSolid && IsTileWater( nextTile )) && !(treatScorpioAsSolid && IsTileScorpio( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		start++;//= (start + 1) % maxQueueLength;
	}
	delete[] queue;
}

void Map::BuildSolidTileHeatMap()
{
	m_solidTileHeatMap->SetAllValues( FLT_MAX );
	for (int y = 0; y < m_dimensions.y; y++) {
		for (int x = 0; x < m_dimensions.x; x++) {
			if (!IsTileSolid( IntVec2( x, y ) ) && !IsTileWater( IntVec2( x, y ) )){
				m_solidTileHeatMap->SetTileValue( IntVec2( x, y ), 0.f );
			}
		}
	}
}

void Map::BuildAmphibiousTileHeatMap()
{
	m_amphibiousTileHeatMap->SetAllValues( FLT_MAX );
	for (int y = 0; y < m_dimensions.y; y++) {
		for (int x = 0; x < m_dimensions.x; x++) {
			if (!IsTileSolid( IntVec2( x, y ) )) {
				m_amphibiousTileHeatMap->SetTileValue( IntVec2( x, y ), 0.f );
			}
		}
	}
}

void Map::AddEntityToAllEntityLists( Entity* entity )
{
	EntityList& entityTypeArray = m_entityListsByType[(int)entity->m_type];
	AddEntityToEntityList( entity, entityTypeArray );
	if (entity->IsActor()) {
		EntityList& actorFactionArray = m_actorListsByFaction[(int)entity->m_faction];
		AddEntityToEntityList( entity, actorFactionArray );
	}
	AddEntityToEntityList( entity, m_allEntityList );
}

void Map::AddEntityToEntityList( Entity* entity, EntityList& entityList )
{
	for (int i = 0; i < (int)entityList.size(); i++) {
		if (!entityList[i]) {
			entityList[i] = entity;
			return;
		}
	}
	entityList.push_back( entity );
}

IntVec2 const Map::GetMapPosFromWorldPos( Vec2 const& worldPos ) const
{
	return IntVec2( RoundDownToInt( worldPos.x ), RoundDownToInt( worldPos.y ) );
}

Vec2 const Map::GetWorldPosFromMapPosCenter( IntVec2 const& mapPos ) const
{
	return Vec2( mapPos.x + 0.5f, mapPos.y + 0.5f );
}

std::string const& Map::GetTileType( int x, int y ) const
{
	return m_tiles[(size_t)y * m_dimensions.x + x].m_tileDefinition->m_tileType;
}

std::string const& Map::GetTileType( IntVec2 const& mapPos ) const
{
	return m_tiles[(size_t)mapPos.y * m_dimensions.x + mapPos.x].m_tileDefinition->m_tileType;
}

TileDefinition const& Map::GetTileDef( IntVec2 const& mapPos ) const
{
	return *(m_tiles[(size_t)mapPos.y * m_dimensions.x + mapPos.x].m_tileDefinition);
}

TileDefinition const& Map::GetTileDef( int x, int y ) const
{
	return *(m_tiles[(size_t)y * m_dimensions.x + x].m_tileDefinition);
}

TileDefinition const& Map::GetTileDef( std::string const& tileType ) const
{
	for (TileDefinition const& tileDef : TileDefinition::s_definitions) {
		if (tileType == tileDef.m_tileType) {
			return tileDef;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find definition of %s", tileType.c_str() ) );
}

TileDefinition const& Map::GetTileDef( Rgba8 const& tileImageColor ) const
{
	for (TileDefinition const& tileDef : TileDefinition::s_definitions) {
		if (tileImageColor == tileDef.m_mapImageColor) {
			return tileDef;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find image color definition of Rgb8: %d %d %d %d", tileImageColor.r, tileImageColor.g, tileImageColor.b, tileImageColor.a ) );
}

IntVec2 const& Map::GetDimensions() const
{
	return m_dimensions;
}

Entity* Map::GetNearestEnemyActor( Entity const* inquiryer )
{
	// #ToDo: now no neutral!
	float minDistanceSquared = FLT_MAX;
	int resIndex = -1;
	if (inquiryer->m_faction == EntityFaction::FACTION_EVIL) {
		EntityList& actorList = m_actorListsByFaction[(int)EntityFaction::FACTION_GOOD];
		for (int i = 0; i < (int)actorList.size(); i++) {
			if (actorList[i] && actorList[i]->IsAlive()) {
				float disSquared = GetDistanceSquared2D( actorList[i]->m_position, inquiryer->m_position );
				if (disSquared < minDistanceSquared) {
					minDistanceSquared = disSquared;
					resIndex = i;
				}
			}
		}
		if (resIndex == -1) {
			return nullptr;
		}
		else {
			return actorList[resIndex];
		}
	}
	else if (inquiryer->m_faction == EntityFaction::FACTION_GOOD) {
		EntityList& actorList = m_actorListsByFaction[(int)EntityFaction::FACTION_EVIL];
		for (int i = 0; i < (int)actorList.size(); i++) {
			if (actorList[i] && actorList[i]->IsAlive()) {
				float disSquared = GetDistanceSquared2D( actorList[i]->m_position, inquiryer->m_position );
				if (disSquared < minDistanceSquared) {
					minDistanceSquared = disSquared;
					resIndex = i;
				}
			}
		}
		if (resIndex == -1) {
			return nullptr;
		}
		else {
			return actorList[resIndex];
		}
	}
	return nullptr;
}

Vec2 const Map::GetNextPosGoTo( Entity const* inquiryer, Vec2 const& targetPosition, TileHeatMap const& tileHeatMap ) const
{
	// do not use taxicab distance! take care of diagonal 
	if (GetDistanceSquared2D( targetPosition, inquiryer->m_position ) <= 1.f) {
		return targetPosition;
	}
	return GetNextPosGoTo( inquiryer->m_position, targetPosition, tileHeatMap );
}

Vec2 const Map::GetNextPosGoTo( Vec2 const& startPos, Vec2 const& targetPosition, TileHeatMap const& tileHeatMap ) const
{
	UNUSED( targetPosition );
	IntVec2 inquiryerCurMapPos = GetMapPosFromWorldPos( startPos );
	float inquiryerCurTileValue = tileHeatMap.GetTileValue( inquiryerCurMapPos );
	int nextDirection = 0;
	float maxWeight = -1;
	float eastWeight = tileHeatMap.GetTileValue( inquiryerCurMapPos + IntVec2( 1, 0 ), FLT_MAX );
	if (eastWeight != FLT_MAX && eastWeight < inquiryerCurTileValue) {
		if (inquiryerCurTileValue - eastWeight > maxWeight) {
			nextDirection = 1;
			maxWeight = inquiryerCurTileValue - eastWeight;
		}
	}
	float westWeight = tileHeatMap.GetTileValue( inquiryerCurMapPos + IntVec2( -1, 0 ), FLT_MAX );
	if (westWeight != FLT_MAX && westWeight < inquiryerCurTileValue) {
		if (inquiryerCurTileValue - westWeight > maxWeight) {
			nextDirection = 2;
			maxWeight = inquiryerCurTileValue - westWeight;
		}
	}
	float northWeight = tileHeatMap.GetTileValue( inquiryerCurMapPos + IntVec2( 0, 1 ), FLT_MAX );
	if (northWeight != FLT_MAX && northWeight < inquiryerCurTileValue) {
		if (inquiryerCurTileValue - northWeight > maxWeight) {
			nextDirection = 3;
			maxWeight = inquiryerCurTileValue - northWeight;
		}
	}
	float southWeight = tileHeatMap.GetTileValue( inquiryerCurMapPos + IntVec2( 0, -1 ), FLT_MAX );
	if (southWeight != FLT_MAX && southWeight < inquiryerCurTileValue) {
		if (inquiryerCurTileValue - southWeight > maxWeight) {
			nextDirection = 4;
			maxWeight = inquiryerCurTileValue - southWeight;
		}
	}

	if (nextDirection == 1) {
		return GetWorldPosFromMapPosCenter( inquiryerCurMapPos + IntVec2( 1, 0 ) );
	}
	else if (nextDirection == 2) {
		return GetWorldPosFromMapPosCenter( inquiryerCurMapPos + IntVec2( -1, 0 ) );
	}
	else if (nextDirection == 3) {
		return GetWorldPosFromMapPosCenter( inquiryerCurMapPos + IntVec2( 0, 1 ) );
	}
	else if (nextDirection == 4) {
		return GetWorldPosFromMapPosCenter( inquiryerCurMapPos + IntVec2( 0, -1 ) );
	}
	return GetWorldPosFromMapPosCenter( inquiryerCurMapPos );
}

Vec2 const Map::GetDistanceTileHeatMapForTargetPos( Entity const* inquiryer, Vec2 const& targetPosition, TileHeatMap& out_tileHeatMap ) const
{
	IntVec2 targetCurMapPos = GetMapPosFromWorldPos( targetPosition );
	// IntVec2 inquiryerCurMapPos = GetMapPosFromWorldPos( inquiryer->m_position );
	PopulateHeatMapDistanceField( out_tileHeatMap, targetCurMapPos, FLT_MAX, 0.f, !inquiryer->m_isAmphibious, true );
	return GetNextPosGoTo( inquiryer, targetPosition, out_tileHeatMap );
}

Vec2 const Map::GetRandomWonderPosAndDistanceTileHeatMap( Entity const* inquiryer, Vec2& out_WonderPos, TileHeatMap& out_tileHeatMap ) const
{
	Vec2 nextPosToGo;
	IntVec2 newPos;
	static int tryTime = 0;
	tryTime = 0;
	int searchStrategy = 0;
	if((m_taskType == TaskType::Breakthrough && inquiryer->m_faction == EntityFaction::FACTION_EVIL)
		|| (m_taskType == TaskType::Clear && inquiryer->m_faction == EntityFaction::FACTION_GOOD)) {
		searchStrategy = 0;
	}
	else if ((m_taskType == TaskType::Defense && inquiryer->m_faction == EntityFaction::FACTION_GOOD)
		|| (m_taskType == TaskType::Clear && inquiryer->m_faction == EntityFaction::FACTION_EVIL)) {
		searchStrategy = 1;
	}
	else if ((m_taskType == TaskType::Defense && inquiryer->m_faction == EntityFaction::FACTION_EVIL)
		|| (m_taskType == TaskType::Breakthrough && inquiryer->m_faction == EntityFaction::FACTION_GOOD)
		|| m_taskType == TaskType::Capture) {
		searchStrategy = 2;
	}

	do {
		tryTime++;
		if (tryTime > 100) {
			break;
		}
		if (searchStrategy == 0) {
			newPos = IntVec2( g_theGame->m_randNumGen->RollRandomIntInRange( 1, m_dimensions.x - 2 ), g_theGame->m_randNumGen->RollRandomIntInRange( 1, m_dimensions.y - 2 ) );
		}
		else if (searchStrategy == 1) {
			newPos = IntVec2(
				g_theGame->m_randNumGen->RollRandomIntInRange( RoundDownToInt( inquiryer->m_position.x ) - 2, RoundDownToInt( inquiryer->m_position.x ) + 2 ),
				g_theGame->m_randNumGen->RollRandomIntInRange( RoundDownToInt( inquiryer->m_position.y ) - 2, RoundDownToInt( inquiryer->m_position.y ) + 2 ) );
		}
		else if (searchStrategy == 2) {
			if (m_taskType == TaskType::Defense) {
				newPos = IntVec2(
					g_theGame->m_randNumGen->RollRandomIntInRange( RoundDownToInt( m_theBuilding->m_position.x ) - 3, RoundDownToInt( m_theBuilding->m_position.x ) + 3 ),
					g_theGame->m_randNumGen->RollRandomIntInRange( RoundDownToInt( m_theBuilding->m_position.y ) - 3, RoundDownToInt( m_theBuilding->m_position.y ) + 3 ) );
			}
			else if (m_taskType == TaskType::Capture) {
				newPos = IntVec2(
					g_theGame->m_randNumGen->RollRandomIntInRange( RoundDownToInt( m_captureCenter.x ) - 6, RoundDownToInt( m_captureCenter.x ) + 6 ),
					g_theGame->m_randNumGen->RollRandomIntInRange( RoundDownToInt( m_captureCenter.y ) - 6, RoundDownToInt( m_captureCenter.y ) + 6 ) );
			}
			else if (m_taskType == TaskType::Breakthrough) {
				newPos = IntVec2(
					g_theGame->m_randNumGen->RollRandomIntInRange( m_dimensions.x - 6, m_dimensions.x - 2 ),
					g_theGame->m_randNumGen->RollRandomIntInRange( m_dimensions.y - 6, m_dimensions.y - 2 ) );
			}
		}

		if (IsTileSolid( newPos ) || (!inquiryer->m_isAmphibious && IsTileWater( newPos )) || IsTileScorpio( newPos )) {
			continue;
		}
		nextPosToGo = GetDistanceTileHeatMapForTargetPos( inquiryer, GetWorldPosFromMapPosCenter( newPos ), out_tileHeatMap );
	} while (IsTileSolid( newPos ) || (!inquiryer->m_isAmphibious && IsTileWater( newPos )) || IsTileScorpio( newPos ) || out_tileHeatMap.GetTileValue(GetMapPosFromWorldPos(inquiryer->m_position)) == FLT_MAX);
	// #ToDo: what will happen when exceeds max try time
	out_WonderPos = GetWorldPosFromMapPosCenter( newPos );
	return nextPosToGo;
}

void Map::GenerateEntityPathToGoal( std::vector<Vec2>& out_pathPoints, TileHeatMap const& tileHeatMap, Vec2 const& startPos, Vec2 const& targetPos ) const
{
	out_pathPoints.clear();
	std::vector<Vec2> tempVector;
	tempVector.reserve( 20 );
	Vec2 curPos = startPos;
	Vec2 lastPos = curPos;
	while ( GetMapPosFromWorldPos( curPos ) != GetMapPosFromWorldPos( targetPos ) ) {
		curPos = GetNextPosGoTo( curPos, targetPos, tileHeatMap );
		if (curPos == lastPos) {
			// cannot find path
			break;
		}
		else {
			lastPos = curPos;
			tempVector.push_back( curPos );
		}
	}
	// reverse order
	for (int i = 0; i < (int)tempVector.size(); i++) {
		out_pathPoints.push_back( tempVector[tempVector.size() - 1 - i] );
	}
}

Entity* Map::SpawnNewEntity( EntityType type, Vec2 const& position, EntityFaction faction, Entity const* spawner, float orientationDegrees )
{
	Entity* retEntity = nullptr;
	if (type == EntityType::_BULLET) {
		if (spawner && spawner->m_type == EntityType::_GOOD_PLAYER) {
			retEntity = new Bullet( position, this, orientationDegrees, faction, EntityType::_BULLET, spawner->m_damage, 2, 0.f );
		}
		else if (spawner) {
			retEntity = new Bullet( position, this, orientationDegrees, faction, EntityType::_BULLET, spawner->m_damage, 0, spawner->m_velocity.GetLength() );
		}
	}
	else if (type == EntityType::_LEO) {
		retEntity = new Leo( position, this, faction );
	}
	else if (type == EntityType::_ARIES) {
		retEntity = new Aries( position, this, faction );
	}
	else if (type == EntityType::_SCORPIO) {
		retEntity = new Scorpio( position, this, faction );
	}
	else if (type == EntityType::_CANCER) {
		retEntity = new Cancer( position, this, faction );
	}
	else if (type == EntityType::_GOOD_PLAYER) {
		retEntity = new PlayerTank( position, this );
	}
	else if (type == EntityType::_CAPRICORN) {
		retEntity = new Capricorn( position, this, faction );
	}
	else if (type == EntityType::_GUIDED_BULLET) {
		retEntity = new Bullet( position, this, orientationDegrees, faction, EntityType::_GUIDED_BULLET, spawner->m_damage, 0, spawner->m_velocity.GetLength(), spawner->m_target );
	}
	else if (type == EntityType::_EXPLOSION) {
		if (spawner && (spawner->m_type == EntityType::_BULLET || spawner->m_type == EntityType::_BOLT || spawner->m_type == EntityType::_GUIDED_BULLET)) {
			retEntity = new Explosion( position, this, 0.3f, 0.4f );
		}
		else if (spawner && spawner->IsAlive()) {
			retEntity = new Explosion( position, this, 0.2f, 0.2f );
		}
		else {
			retEntity = new Explosion( position, this, 1.f, 0.6f );
		}
	}
	else if (type == EntityType::_FLAME_BULLET) {
		retEntity = new Bullet( position, this, orientationDegrees, faction, EntityType::_FLAME_BULLET, 0.2f, 0, spawner->m_velocity.GetLength() );
	}
	else if (type == EntityType::_BUILDING) {
		retEntity = new Building( position, this );
	}
	else if (type == EntityType::_RUBBLE) {
		retEntity = new Rubble( position, this );
	}
	
	if (retEntity) {
		AddEntityToAllEntityLists( retEntity );
	}
	return retEntity;
}

void Map::SpawnExplosion( Vec2 const& position, float sizeFactor, float lifeSpanSeconds )
{
	Entity* newEntity = new Explosion( position, this, sizeFactor, lifeSpanSeconds );
	AddEntityToAllEntityLists( newEntity );
}

void Map::CallReinforcemets( Entity* caller )
{
	int& numOfReins = g_theGame->GetNumOfReinforcements();
	if (numOfReins > 0) {
		--numOfReins;
	}
	else {
		return;
	}

	g_devConsole->AddLine( DevConsole::INFO_MINOR, Stringf("Call reinforcements at (%2.2f, %2.2f)", caller->m_position.x, caller->m_position.y) );
	IntVec2 mapPos = GetMapPosFromWorldPos( caller->m_position );
	float xPossibleOffset = 0.f;
	float yPossibleOffset = 0.f;
	if ((mapPos.x == 1 || mapPos.x == m_dimensions.x - 2) && mapPos.y != 1 && mapPos.y != m_dimensions.y - 2) {
		yPossibleOffset = 1.f;
	}
	else if((mapPos.y == 1 || mapPos.y == m_dimensions.y - 2) && mapPos.x != 1 && mapPos.x != m_dimensions.x - 2) {
		xPossibleOffset = 1.f;
	}
	else {
		xPossibleOffset = 0.2f;
		yPossibleOffset = 0.2f;
	}
	Vec2 offset;
	if (m_curReinforcements * 4 < (int)m_mapDef->m_reinforcements.size()) {
		for (int i = 0; i < 4; i++) {
			switch (i)
			{
			case 0: {
				for (int j = 0; j < m_mapDef->m_reinforcements[i + (size_t)4 * m_curReinforcements]; j++) {
					offset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -xPossibleOffset, xPossibleOffset ), g_theGame->m_randNumGen->RollRandomFloatInRange( -yPossibleOffset, yPossibleOffset ) );
					SpawnNewEntity( EntityType::_ARIES, caller->m_position + offset, EntityFaction::FACTION_GOOD );
				}
				break;
			}
			case 1: {
				for (int j = 0; j < m_mapDef->m_reinforcements[i + (size_t)4 * m_curReinforcements]; j++) {
					offset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -xPossibleOffset, xPossibleOffset ), g_theGame->m_randNumGen->RollRandomFloatInRange( -yPossibleOffset, yPossibleOffset ) );
					SpawnNewEntity( EntityType::_LEO, caller->m_position + offset, EntityFaction::FACTION_GOOD );
				}
				break;
			}
			case 2: {
				for (int j = 0; j < m_mapDef->m_reinforcements[i + (size_t)4 * m_curReinforcements]; j++) {
					offset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -xPossibleOffset, xPossibleOffset ), g_theGame->m_randNumGen->RollRandomFloatInRange( -yPossibleOffset, yPossibleOffset ) );
					SpawnNewEntity( EntityType::_CAPRICORN, caller->m_position + offset, EntityFaction::FACTION_GOOD );
				}
				break;
			}
			case 3: {
				for (int j = 0; j < m_mapDef->m_reinforcements[i + (size_t)4 * m_curReinforcements]; j++) {
					offset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -xPossibleOffset, xPossibleOffset ), g_theGame->m_randNumGen->RollRandomFloatInRange( -yPossibleOffset, yPossibleOffset ) );
					SpawnNewEntity( EntityType::_CANCER, caller->m_position + offset, EntityFaction::FACTION_GOOD );
				}
				break;
			}
			default:
				break;
			}
		}
		++m_curReinforcements;
	}
	else {
		for (int i = 0; i < 4; i++) {
			SpawnNewEntity( EntityType::_LEO, caller->m_position, EntityFaction::FACTION_GOOD );
		}
	}
}

void Map::SpawnEnemyReinforcemets( EnemyReinfDef const& reinfDef )
{
	Vec2 spawnPos = GetWorldPosFromMapPosCenter( reinfDef.m_tileToCome );
	Vec2 randomOffset;
	g_devConsole->AddLine( DevConsole::INFO_MINOR, Stringf( "Enemy reinforcements arrive at (%2.2f, %2.2f)", spawnPos.x, spawnPos.y ) );
	for (int i = 0; i < reinfDef.m_numOfAries; i++) {
		randomOffset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ), g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ) );
		SpawnNewEntity( EntityType::_ARIES, spawnPos + randomOffset, EntityFaction::FACTION_EVIL );
	}
	for (int i = 0; i < reinfDef.m_numOfCancer; i++) {
		randomOffset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ), g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ) );
		SpawnNewEntity( EntityType::_CANCER, spawnPos + randomOffset, EntityFaction::FACTION_EVIL );
	}
	for (int i = 0; i < reinfDef.m_numOfCapricorn; i++) {
		randomOffset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ), g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ) );
		SpawnNewEntity( EntityType::_CAPRICORN, spawnPos + randomOffset, EntityFaction::FACTION_EVIL );
	}
	for (int i = 0; i < reinfDef.m_numOfLeo; i++) {
		randomOffset = Vec2( g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ), g_theGame->m_randNumGen->RollRandomFloatInRange( -0.1f, 0.1f ) );
		SpawnNewEntity( EntityType::_LEO, spawnPos + randomOffset, EntityFaction::FACTION_EVIL );
	}
}

void Map::AddEntityToMap( Entity& e )
{
	AddEntityToAllEntityLists( &e );
}

bool Map::RemoveEntityFromMap( Entity& e )
{
	EntityList& entityArray = m_entityListsByType[(int)e.m_type];
	for (int i = 0; i < (int)entityArray.size(); i++) {
		if (entityArray[i] == &e) {
			if (entityArray[i]->IsActor()) {
				EntityList& actorListByFaction = m_actorListsByFaction[(int)entityArray[i]->m_faction];
				DeleteGarbageInEntityList( entityArray[i], actorListByFaction );
			}
			DeleteGarbageInEntityList( entityArray[i], m_allEntityList );
			entityArray[i] = nullptr;
			return true;
		}
	}
	return false;
}

PlayerTank* Map::GetPlayer() const
{
	return m_playerTank;
}

void Map::SetPlayer( PlayerTank* player )
{
	m_playerTank = player;
}

Vec2 const Map::GetEnterPoint() const
{
	return m_enterPoint;
}

TileHeatMap const* Map::GetAmphibiousTileHeatMap() const
{
	return m_amphibiousTileHeatMap;
}

bool Map::IsPointInSolid( Vec2 const& pointWorldPos ) const
{
	return IsTileSolid( GetMapPosFromWorldPos( pointWorldPos ) );
}

bool Map::IsTileSolid( IntVec2 const& mapPos ) const
{
	if (mapPos.x < 0 || mapPos.x >= m_dimensions.x || mapPos.y < 0 || mapPos.y >= m_dimensions.y) {
		return true;
	}
	return GetTileDef( mapPos ).m_isSolid;
}

bool Map::IsTileWater( IntVec2 const& mapPos ) const
{
	if (mapPos.x < 0 || mapPos.x >= m_dimensions.x || mapPos.y < 0 || mapPos.y >= m_dimensions.y) {
		return true;
	}
	return GetTileDef( mapPos ).m_isWater;
}

bool Map::IsTileScorpio( IntVec2 const& mapPos ) const
{
	for (Entity* entity : m_entityListsByType[(int)EntityType::_SCORPIO]) {
		if (entity && entity->IsAlive() && GetMapPosFromWorldPos( entity->m_position ) == mapPos) {
			return true;
		}
	}
	return false;
}

bool Map::IsTileDestructible( IntVec2 const& mapPos ) const
{
	if (mapPos.x < 0 || mapPos.x >= m_dimensions.x || mapPos.y < 0 || mapPos.y >= m_dimensions.y) {
		return false;
	}
	return GetTileDef( mapPos ).m_isDestructible;
}

void Map::RaycastVsTiles( Ray2D const& rayInfo, RayCastResult2D& outRayCastResult, TileHeatMap const& tileHeatMap, float opaqueValue ) const
{
	tileHeatMap.RayCastVsGrid2D( outRayCastResult, rayInfo, opaqueValue );
	/*Vec2 const& startPoint = rayInfo.m_startPosition;
	// forward vector
	Vec2 iBasisNormal = rayInfo.GetiBasisNormal();
	// for better performance
	Vec2 inversedAbsiBasisNormal = Vec2( abs( 1.f / iBasisNormal.x ), abs( 1.f / iBasisNormal.y ) );
	// lastHitEdgePoint for calculate length
	Vec2 lastHitEdgePoint = startPoint;
	// if two entities in the same position, stop to avoid infinite loop bug
	// no bugs at all!
	//if (iBasisNormal == Vec2( 0.f, 0.f )) {
	//	return false;
	//}
	// now or current which tile the ray head is
	IntVec2 nowMapPos = GetMapPosFromWorldPos( startPoint );
	// how long does the ray travel
	float length = 0.f;
	// which direction will the ray go
	int stepX = 0;
	int stepY = 0;
	// which edge of the AABB tile box will the ray hit
	IntVec2 testEdge;
	float rayOrientationDegrees = rayInfo.GetOrientationDegrees();
	if (rayOrientationDegrees > 0.f && rayOrientationDegrees <= 90.f) {
		stepX = 1;
		stepY = 1;
		testEdge = IntVec2( 1, 1 );
	}
	else if (rayOrientationDegrees > 90.f && rayOrientationDegrees <= 180.f) {
		stepX = -1;
		stepY = 1;
		testEdge = IntVec2( 0, 1 );
	}
	else if (rayOrientationDegrees > -180.f && rayOrientationDegrees <= -90.f) {
		stepX = -1;
		stepY = -1;
		testEdge = IntVec2( 0, 0 );
	}
	else if (rayOrientationDegrees > -90.f && rayOrientationDegrees <= 0.f) {
		stepX = 1;
		stepY = -1;
		testEdge = IntVec2( 1, 0 );
	}
	else {
		ERROR_RECOVERABLE( "Degrees not in range" );
	}

	outRayCastResult.fromPoint = startPoint;
	outRayCastResult.maxLength = rayInfo.m_maxRange;
	outRayCastResult.orientationDegrees = rayOrientationDegrees;
	// infinite loop to find target
	for (;;) {
		// calculate which edges to test
		IntVec2 curTestEdge = testEdge + nowMapPos;
		// first test y
		float testY = rayInfo.GetYFromX( (float)curTestEdge.x );
		// if hit y(vertical) edge of current tile
		if (testY >= (float)(nowMapPos.y - 0) && testY <= (float)(nowMapPos.y + 1)) {
			// add travel length of this tile to length: be care of nan and inf
			if (iBasisNormal.y != 0.f) {
				length += abs( testY - lastHitEdgePoint.y ) * inversedAbsiBasisNormal.y;
			}
			else {
				length += abs( (float)curTestEdge.x - lastHitEdgePoint.x ) * inversedAbsiBasisNormal.x;
			}
			// goto next tile
			nowMapPos.x += stepX;
			// record last point to calculate length
			lastHitEdgePoint = Vec2( (float)curTestEdge.x, testY );
			// if meet solid or exceed max length: end ray cast
			if (IsTileSolid( nowMapPos ) || length > rayInfo.m_maxRange) {
				if (length > rayInfo.m_maxRange) {
					outRayCastResult.length = rayInfo.m_maxRange;
				}
				else {
					outRayCastResult.length = length;
				}
				outRayCastResult.stopPoint = lastHitEdgePoint;
				return;
			}
			continue;
		}
		// if not hit vertical then must hit horizontal
		float testX = rayInfo.GetXFromY( (float)curTestEdge.y );
		// if hit x(horizontal) edge of current tile
		//if (testX >= (float)(nowMapPos.x - 0) && testX <= (float)(nowMapPos.x + 1)) {
		// add travel length of this tile to length: be care of nan and inf
		if (iBasisNormal.y != 0.f) {
			length += abs( (float)curTestEdge.y - lastHitEdgePoint.y ) * inversedAbsiBasisNormal.y;
		}
		else {
			length += abs( testX - lastHitEdgePoint.x ) * inversedAbsiBasisNormal.x;
		}
		// goto next tile
		nowMapPos.y += stepY;
		// record last point to calculate length
		lastHitEdgePoint = Vec2( testX, (float)curTestEdge.y );
		// if meet solid or exceed max length: end ray cast
		if (IsTileSolid( nowMapPos ) || length > rayInfo.m_maxRange) {
			if (length > rayInfo.m_maxRange) {
				outRayCastResult.length = rayInfo.m_maxRange;
			}
			else {
				outRayCastResult.length = length;
			}
			outRayCastResult.stopPoint = lastHitEdgePoint;
			return;
		}
		//	continue;
		//}
	}
	return;*/
}

bool Map::HasLineOfSight( Entity const* rayShooter, Entity const* target, RayCastResult2D& outRayCastResult, TileHeatMap const& tileHeatMap, float opaqueValue ) const
{
	Vec2 forwardVector = target->m_position - rayShooter->m_position;
	float length = forwardVector.GetLength();
	if (length > g_enemyVisibleRange) {
		return false;
	}
	Vec2 forwardNormal = forwardVector.GetNormalized();
	RaycastVsTiles( Ray2D( rayShooter->m_position, forwardNormal, length ), outRayCastResult, tileHeatMap, opaqueValue );
	return !outRayCastResult.m_didImpact;
	/*
	Vec2 const& startPoint = rayShooter->m_position;
	Vec2 const& targetPoint = target->m_position;
	// if far from range, just return
	if (GetDistanceSquared2D( startPoint, targetPoint ) > 10.f * 10.f) {
		return false;
	}
	// forward vector
	Vec2 iBasisNormal = (targetPoint - startPoint).GetNormalized();
	// for better performance
	Vec2 inversedAbsiBasisNormal = Vec2( abs(1.f / iBasisNormal.x), abs(1.f / iBasisNormal.y) );
	RayCast ray( startPoint, iBasisNormal, g_enemyVisibleRange );
	// lastHitEdgePoint for calculate length
	Vec2 lastHitEdgePoint = startPoint;
	// if two entities in the same position, stop to avoid infinite loop bug
	// no bugs at all!
	//if (iBasisNormal == Vec2( 0.f, 0.f )) {
	//	return false;
	//}
	// now or current which tile the ray head is
	IntVec2 nowMapPos = GetMapPosFromWorldPos( startPoint );
	// map position of target
	IntVec2 targetMapPos = GetMapPosFromWorldPos( target->m_position );
	// how long does the ray travel
	float length = 0.f;
	// which direction will the ray go
	int stepX = 0;
	int stepY = 0;
	// which edge of the AABB tile box will the ray hit
	IntVec2 testEdge;
	float rayOrientationDegrees = ray.GetOrientationDegrees();
	if (rayOrientationDegrees > 0.f && rayOrientationDegrees <= 90.f) {
		stepX = 1;
		stepY = 1;
		testEdge = IntVec2( 1, 1 );
	}
	else if (rayOrientationDegrees > 90.f && rayOrientationDegrees <= 180.f) {
		stepX = -1;
		stepY = 1;
		testEdge = IntVec2( 0, 1 );
	}
	else if (rayOrientationDegrees > -180.f && rayOrientationDegrees <= -90.f) {
		stepX = -1;
		stepY = -1;
		testEdge = IntVec2( 0, 0 );
	}
	else if (rayOrientationDegrees > -90.f && rayOrientationDegrees <= 0.f) {
		stepX = 1;
		stepY = -1;
		testEdge = IntVec2( 1, 0 );
	}
	else {
		ERROR_RECOVERABLE( "Degrees not in range" );
	}

	outRayCastResult.fromPoint = startPoint;
	outRayCastResult.maxLength = ray.m_maxRange;
	outRayCastResult.orientationDegrees = rayOrientationDegrees;
	// infinite loop to find target
	for (;;) {
		// in the same tile
		if (nowMapPos == targetMapPos) {
			// how long left to max range
			float leftLength = ray.m_maxRange - length;
			// how long left to target
			float toTargetLength;
			if (iBasisNormal.y != 0.f) {
				toTargetLength = abs( targetPoint.y - lastHitEdgePoint.y ) * inversedAbsiBasisNormal.y;
			}
			else {
				toTargetLength = abs( targetPoint.x - lastHitEdgePoint.x ) * inversedAbsiBasisNormal.x;
			}
			// hit target!
			if (toTargetLength <= leftLength) {
				outRayCastResult.length = length;
				outRayCastResult.stopPoint = targetPoint;
				return true;
			}
			// target not in range
			else {
				outRayCastResult.length = ray.m_maxRange;
				outRayCastResult.stopPoint = lastHitEdgePoint + iBasisNormal * leftLength;
				return false;
			}
		}
		// calculate which edges to test
		IntVec2 curTestEdge = testEdge + nowMapPos;
		// first test y
		float testY = ray.GetYFromX( (float)curTestEdge.x );
		// if hit y(vertical) edge of current tile
		if (testY >= (float)(nowMapPos.y - 0) && testY <= (float)(nowMapPos.y + 1)) {
			// add travel length of this tile to length: be care of nan and inf
			if (iBasisNormal.y != 0.f) {
				length += abs( testY - lastHitEdgePoint.y ) * inversedAbsiBasisNormal.y;
			}
			else {
				length += abs( (float)curTestEdge.x - lastHitEdgePoint.x ) * inversedAbsiBasisNormal.x;
			}
			// goto next tile
			nowMapPos.x += stepX;
			// record last point to calculate length
			lastHitEdgePoint = Vec2( (float)curTestEdge.x, testY );
			// if meet solid or exceed max length: end ray cast
			if (IsTileSolid( nowMapPos ) || length > ray.m_maxRange) {
				if (length > ray.m_maxRange) {
					outRayCastResult.length = ray.m_maxRange;
				}
				else {
					outRayCastResult.length = length;
				}
				outRayCastResult.stopPoint = lastHitEdgePoint;
				return false;
			}
			continue;
		}
		// if not hit vertical then must hit horizontal
		float testX = ray.GetXFromY( (float)curTestEdge.y );
		// if hit x(horizontal) edge of current tile
		//if (testX >= (float)(nowMapPos.x - 0) && testX <= (float)(nowMapPos.x + 1)) {
			// add travel length of this tile to length: be care of nan and inf
		if (iBasisNormal.y != 0.f) {
			length += abs( (float)curTestEdge.y - lastHitEdgePoint.y ) * inversedAbsiBasisNormal.y;
		}
		else {
			length += abs( testX - lastHitEdgePoint.x ) * inversedAbsiBasisNormal.x;
		}
		// goto next tile
		nowMapPos.y += stepY;
		// record last point to calculate length
		lastHitEdgePoint = Vec2( testX, (float)curTestEdge.y );
		// if meet solid or exceed max length: end ray cast
		if (IsTileSolid( nowMapPos ) || length > ray.m_maxRange) {
			if (length > ray.m_maxRange) {
				outRayCastResult.length = ray.m_maxRange;
			}
			else {
				outRayCastResult.length = length;
			}
			outRayCastResult.stopPoint = lastHitEdgePoint;
			return false;
		}
		//	continue;
		//}
	}
	return false;*/
}

bool Map::HasLineOfSight( Vec2 const& startPos, Vec2 const& targetPos, RayCastResult2D& outRayCastResult, TileHeatMap const& tileHeatMap, float opaqueValue ) const
{
	RaycastVsTiles( Ray2D( startPos, targetPos ), outRayCastResult, tileHeatMap, opaqueValue );
	return !outRayCastResult.m_didImpact;
}

bool Map::PlaySound( AudioName name, Vec2 const& playPosition, float intervalTimeSeconds /*= 0.f*/, bool isLooped /*= false*/, float volume /*= 1.f*/, float speed /*= 1.f */ )
{
	Vec2 const& listenPos = m_playerTank->m_position;
	float balance = RangeMapClamped( GetFractionWithinRange( playPosition.x, listenPos.x - WORLD_SIZE_X * 0.5f, listenPos.x + WORLD_SIZE_X * 0.5f ), 0.f, 1.f, -1.f, 1.f );
	float distance = GetDistance2D( listenPos, playPosition );
	float newVolume = volume * RangeMapClamped( distance, 0.f, 15.f, 1.f, 0.f );
	return g_theApp->PlaySound( name, intervalTimeSeconds, isLooped, newVolume, balance, speed );
}

void Map::ChangeTileType(int x, int y, TileDefinition const& newType)
{
	m_tiles[(size_t)y * m_dimensions.x + x].ChangeType( newType );
}

void Map::ChangeTileType( IntVec2 const& pos, TileDefinition const& newType )
{
	m_tiles[(size_t)pos.y * m_dimensions.x + pos.x].ChangeType( newType );
}

void Map::TileGetDamage( IntVec2 mapPos, float damage )
{
	m_tiles[(size_t)mapPos.y * m_dimensions.x + mapPos.x].m_health -= damage;
}

Tile const& Map::GetTile( IntVec2 mapPos )
{
	return m_tiles[(size_t)mapPos.y * m_dimensions.x + mapPos.x];
}

IntVec2 const Map::GetXYPosFromArrayIndex( int i ) const
{
	int retY = i / m_dimensions.x;
	int retX = i - retY * m_dimensions.x;
	return IntVec2( retX, retY );
}

bool Map::IsIndexInBound( IntVec2 index ) const
{
	return index.x >= 0 && index.y >= 0 && index.x < m_dimensions.x && index.y < m_dimensions.y;
}

void Map::AllEntitiesCorrectCollisionWithEachOther( float deltaTime )
{
	UNUSED( deltaTime );
	for (int i = 0; i < (int)m_allEntityList.size(); i++) {
		if (!m_allEntityList[i]) {
			continue;
		}
		for (int j = i + 1; j < (int)m_allEntityList.size(); j++) {
			if (m_allEntityList[j]) {
				DealCorrectCollision( *m_allEntityList[i], *m_allEntityList[j] );
			}
		}
	}

	// check collision with rubble
	EntityList& rubbleList = m_entityListsByType[(int)EntityType::_RUBBLE];
	for (int i = 0; i < (int)m_allEntityList.size(); i++) {
		if (!m_allEntityList[i] || !m_allEntityList[i]->IsActor()) {
			continue;
		}
		bool isEffected = false;
		for (int j = 0; j < (int)rubbleList.size(); j++) {
			if (DoDiscsOverlap( m_allEntityList[i]->m_position, m_allEntityList[i]->m_physicsRadius, rubbleList[j]->m_position, 0.3f )) {
				if (!m_allEntityList[i]->m_isEffectedByRubble) {
					m_allEntityList[i]->m_isEffectedByRubble = true;
					m_allEntityList[i]->m_speed *= 0.4f;
					if (m_allEntityList[i]->m_type == EntityType::_GOOD_PLAYER) {
						m_doScreenShake = true;
						m_doControllerShake = true;
					}
				}
				isEffected = true;
				break;
			}
		}
		if (!isEffected && m_allEntityList[i]->m_isEffectedByRubble) {
			m_allEntityList[i]->m_isEffectedByRubble = false;
			m_allEntityList[i]->m_speed *= 2.5f;
			if (m_allEntityList[i]->m_type == EntityType::_GOOD_PLAYER) {
				m_doScreenShake = false;
				m_doControllerShake = false;
			}
		}
	}
}

void Map::DealCorrectCollision( Entity& a, Entity& b )
{
	bool canAPushedByB = a.m_isPushedByEntities && b.m_doesPushEntities;
	bool canBPushedByA = b.m_isPushedByEntities && a.m_doesPushEntities;
	if (!canBPushedByA && !canAPushedByB) {
		return;
	}
	else if (canAPushedByB && !canBPushedByA) {
		PushDiscOutOfFixedDisc2D( a.m_position, a.m_physicsRadius, b.m_position, b.m_physicsRadius );
	}
	else if (canBPushedByA && !canAPushedByB) {
		PushDiscOutOfFixedDisc2D( b.m_position, b.m_physicsRadius, a.m_position, a.m_physicsRadius );
	}
	else {
		PushDiscsOutOfEachOther2D( a.m_position, a.m_physicsRadius, b.m_position, b.m_physicsRadius );
	}
}

void Map::AllEntitiesCorrectCollisionWithWall()
{
	for (int i = 0; i < (int)EntityType::NUM; i++) {
		for (Entity* entity : m_entityListsByType[i]) {
			if (entity) {
				EntityCorrectCollisionWithWall( entity );
			}
		}
	}
}

void Map::EntityCorrectCollisionWithWall( Entity* entity )
{
	if (m_isNoClipMode && entity->m_type == EntityType::_GOOD_PLAYER) {
		return;
	}
	if (!(entity->m_isPushedByWalls) && !entity->IsBullet()) {
		return;
	}


	// extra check for bullet
	if (entity->IsBullet()) {
		BulletCorrectCollisionWithWall( (Bullet*)entity );
		return;
	}

	// collision with solid
	IntVec2 mapPos = GetMapPosFromWorldPos( entity->m_position );
	//Vec2 onTileLeftBottomWorldPos = GetWorldPosFromMapPosLeftBottom( mapPos );

	//mapPos.x = GetClamped( mapPos.x, 1, m_dimensions.x - 2 );
	//mapPos.y = GetClamped( mapPos.y, 1, m_dimensions.y - 2 );
	//EntityCorrectCollisionWithOneTile( entity, mapPos );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( -1, 0 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( 0, -1 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( 0, 1 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( 1, 0 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( 1, 1 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( -1, 1 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( -1, -1 ) );
	EntityCorrectCollisionWithOneTile( entity, mapPos + IntVec2( 1, -1 ) );
}

void Map::EntityCorrectCollisionWithOneTile( Entity* entity, IntVec2 const& tile )
{
	if (IsTileSolid( tile ) || (!(entity->m_isAmphibious) && IsTileWater( tile ))) {
		Vec2 LBTilePos = Vec2( (float)tile.x, (float)tile.y );
		PushDiscOutOfFixedAABB2D( entity->m_position, entity->m_physicsRadius, AABB2( LBTilePos, LBTilePos + Vec2(1.f, 1.f) ) );
	}
}

void Map::BulletCorrectCollisionWithWall( Bullet* entity )
{
	IntVec2 mapPos = GetMapPosFromWorldPos( entity->m_position );
	Vec2 onTileLeftBottomWorldPos = GetWorldPosFromMapPosLeftBottom( mapPos );

	// deal with destructible walls
	if (entity->IsAlive() && IsTileDestructible( mapPos )) {
		TileGetDamage( mapPos, entity->m_damage );
		Tile const& thisTile = GetTile( mapPos );
		if (thisTile.m_health <= 0.f) {
			PlaySound( AudioName::EnemyDied, GetWorldPosFromMapPosCenter( mapPos ) );
			ChangeTileType( mapPos, GetTileDef( thisTile.m_tileDefinition->m_tileTypeAfterDestruction ) );
			SpawnExplosion( GetWorldPosFromMapPosCenter( mapPos ), 2.f, 0.6f );
			BuildSolidTileHeatMap();
			BuildAmphibiousTileHeatMap();
		}
	}

	if (IsTileSolid( mapPos )) {
		if (entity->m_faction == EntityFaction::FACTION_GOOD && entity->m_type != EntityType::_FLAME_BULLET) {
			IntVec2 lastFrameMapPos = GetMapPosFromWorldPos( ((Bullet*)entity)->m_lastFramePosition );
			IntVec2 pushOutDirection = lastFrameMapPos - mapPos;
			if (pushOutDirection == IntVec2( 0, -1 )) {
				entity->m_position.y = (float)lastFrameMapPos.y + 1.f - entity->m_physicsRadius;
				((Bullet*)entity)->BounceOff( Vec2( 0, -1 ) );
			}
			else if (pushOutDirection == IntVec2( 0, 1 )) {
				entity->m_position.y = (float)lastFrameMapPos.y + entity->m_physicsRadius;
				((Bullet*)entity)->BounceOff( Vec2( 0, 1 ) );
			}
			else if (pushOutDirection == IntVec2( 1, 0 )) {
				entity->m_position.x = (float)lastFrameMapPos.x + entity->m_physicsRadius;
				((Bullet*)entity)->BounceOff( Vec2( 1, 0 ) );
			}
			else if (pushOutDirection == IntVec2( -1, 0 )) {
				entity->m_position.x = (float)lastFrameMapPos.x + 1.f - entity->m_physicsRadius;
				((Bullet*)entity)->BounceOff( Vec2( -1, 0 ) );
			}
			else if (pushOutDirection == IntVec2( -1, -1 )) {
				entity->m_position.x = (float)lastFrameMapPos.x + 1.f - entity->m_physicsRadius;
				entity->m_position.y = (float)lastFrameMapPos.y + 1.f - entity->m_physicsRadius;
				// not correct physics
				((Bullet*)entity)->BounceOff( Vec2( -1, -1 ).GetNormalized() );
			}
			else if (pushOutDirection == IntVec2( -1, 1 )) {
				entity->m_position.x = (float)lastFrameMapPos.x + 1.f - entity->m_physicsRadius;
				entity->m_position.y = (float)lastFrameMapPos.y + entity->m_physicsRadius;
				// not correct physics
				((Bullet*)entity)->BounceOff( Vec2( -1, 1 ).GetNormalized() );
			}
			else if (pushOutDirection == IntVec2( 1, -1 )) {
				entity->m_position.x = (float)lastFrameMapPos.x + entity->m_physicsRadius;
				entity->m_position.y = (float)lastFrameMapPos.y + 1.f - entity->m_physicsRadius;
				// not correct physics
				((Bullet*)entity)->BounceOff( Vec2( 1, -1 ).GetNormalized() );
			}
			else if (pushOutDirection == IntVec2( 1, 1 )) {
				entity->m_position.x = (float)lastFrameMapPos.x + entity->m_physicsRadius;
				entity->m_position.y = (float)lastFrameMapPos.y + entity->m_physicsRadius;
				// not correct physics
				((Bullet*)entity)->BounceOff( Vec2( 1, 1 ).GetNormalized() );
			}
			//PushDiscOutOfFixedAABB2D( entity->m_position, entity->m_physicsRadius, AABB2( onTileLeftBottomWorldPos, onTileLeftBottomWorldPos + Vec2( 1.f, 1.f ) ) );
		}
		else {
			entity->Die();
		}
	}

	return;
}

void Map::CheckBulletCollision()
{
	for (int p = (int)EntityType::_BULLET; p < (int)EntityType::_BULLET + NUM_OF_BULLET_TYPES; p++) {
		EntityList& bulletArray = m_entityListsByType[p];
		for (int i = 0; i < (int)bulletArray.size(); i++) {
			if (bulletArray[i]) {
				for (int j = 0; j < (int)EntityType::NUM; j++) {
					bool doBreak = false;
					EntityList& entityArray = m_entityListsByType[j];
					for (int k = 0; k < (int)entityArray.size(); k++) {
						if (entityArray[k] && entityArray[k]->m_isHitByBullets && bulletArray[i]->IsAlive()
							&& (bulletArray[i]->m_faction != entityArray[k]->m_faction)
							&& DoDiscsOverlap( entityArray[k]->m_position, entityArray[k]->m_physicsRadius,
								bulletArray[i]->m_position, bulletArray[i]->m_physicsRadius )) {
							entityArray[k]->GetAttackedByBullet( (Bullet*)bulletArray[i] );
							doBreak = true;
							break;
						}
					}
					if (doBreak) {
						break;
					}
				}
			}
		}
	}
}

Vec2 const Map::GetWorldPosFromMapPosLeftBottom( IntVec2 const& mapPos ) const
{
	return Vec2( (float)mapPos.x, (float)mapPos.y );
}

void Map::RenderPauseMode() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForAABB2D( verts, AABB2( Vec2( 0, 0 ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), Rgba8( 0, 0, 0, 100 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Map::RenderEnterExitMode() const
{
	if (m_curMapState == MapState::ENTER_MAP) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 );
		AddVertsForAABB2D( verts, AABB2( Vec2( 0, 0 ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), Rgba8( 0, 0, 0, (unsigned char)((1.f - m_enterExitTimer) * 255.f) ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
	else if (m_curMapState == MapState::EXIT_MAP) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 );
		AddVertsForAABB2D( verts, AABB2( Vec2( 0, 0 ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), Rgba8( 0, 0, 0, (unsigned char)(m_enterExitTimer * 255.f) ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
}
