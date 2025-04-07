#include "Game/Map.hpp"
#include "Game/Building.hpp"
#include "Game/Game.hpp"
#include "Game/Conveyer.hpp"
#include "Game/Resource.hpp"
#include "Game/Logistics.hpp"
#include "Game/UISystem.hpp"
#include "Game/Widget.hpp"
#include "Game/PowerBuilding.hpp"
#include "Game/Factory.hpp"
#include "Game/Tower.hpp"
#include "Game/Projectile.hpp"
#include "Game/Enemy.hpp"

Map::Map()
	:testTimer(1.f)
{
	testTimer.Start();
	g_conveyorAnimTimer = new Timer( 1.f / CONVEY_SPEED, g_theGame->m_gameClock );
	g_conveyorAnimTimer->Start();
}

Map::~Map()
{
	delete m_tileMapVertexBuffer;
	delete g_conveyorAnimTimer;
	delete m_stoneTerrainVertexBuffer;
	delete m_ironTerrainVertexBuffer;
	delete m_copperTerrainVertexBuffer;
	delete m_coalTerrainVertexBuffer;
	delete m_catalystTerrainVertexBuffer;
	delete m_wallVertexBuffer;

	for (auto entity : m_entities) {
		if (entity && entity->m_entityType != EntityType::Building) {
			delete entity;
		}
	}

	std::vector<Building*> buildingDeleted;
	for (auto building : m_buildings) {
		if (std::find( buildingDeleted.begin(), buildingDeleted.end(), building.second ) == buildingDeleted.end()) {
			delete building.second;
			buildingDeleted.push_back( building.second );
		}
	}

	for (auto network : m_powerNetworks) {
		delete network;
	}


}

void Map::StartUp()
{
	SetupUI();

	m_mapRNG.SetSeed( 4 );
	m_dimensions = IntVec2( TILE_MAP_SIZE_X, TILE_MAP_SIZE_Y );
	m_tiles.reserve( m_dimensions.x * m_dimensions.y );
	for (int y = 0; y < m_dimensions.y; ++y) {
		for (int x = 0; x < m_dimensions.x; ++x) {
			m_tiles.push_back( Tile( TileDefinition::GetDef( "Land" ) ) );
		}
	}

	IntVec2 center = IntVec2( TILE_MAP_SIZE_X / 2, TILE_MAP_SIZE_Y / 2 );
	Base* newBase = new Base( center - IntVec2( 1, 1 ) );;
	m_buildings[center - IntVec2( 1, 1 )] = newBase;
	m_buildings[center] = newBase;
	m_buildings[center - IntVec2( 1, 0 )] = newBase;
	m_buildings[center - IntVec2( 0, 1 )] = newBase;
	m_buildings[center + IntVec2( 1, 0 )] = newBase;
	m_buildings[center + IntVec2( 0, 1 )] = newBase;
	m_buildings[center + IntVec2( 1, 1 )] = newBase;
	m_buildings[center + IntVec2( 1, -1 )] = newBase;
	m_buildings[center + IntVec2( -1, 1 )] = newBase;
	m_buildingList.push_back( newBase );
	m_entities.push_back( newBase );
	// add resources
	// add circle
	// make sure there is one near the base
	TileDefinition const& circleDef = TileDefinition::GetDef( "Iron" );

	for (int i = 0; i < TILE_MAP_SIZE_X * TILE_MAP_SIZE_Y / 5000; ++i) {
		// common and large
		IntVec2 circleCoords;
		int numOfSteps = 0;
		if (i == 0) {
			IntVec2 tileOffset;
			do {
				tileOffset = GetRandomTileCoordsInRange( IntVec2( 0, 0 ), IntVec2( 20, 20 ) );
			} while (tileOffset.x < 12 && tileOffset.y < 12);
			circleCoords = center + tileOffset;
			numOfSteps = m_mapRNG.RollRandomIntInRange( 60, 80 );
		}
		else {
			circleCoords = GetRandomTileCoordsOnMap();
			numOfSteps = m_mapRNG.RollRandomIntInRange( 25, 60 );
		}
		DoWormRandomTileGeneration( circleDef, circleCoords, numOfSteps );
	}

	// add triangle
	TileDefinition const& triangleDef = TileDefinition::GetDef( "Stone" );

	for (int i = 0; i < TILE_MAP_SIZE_X * TILE_MAP_SIZE_Y / 10000; ++i) {
		// rare
		IntVec2	curCoords;
		do {
			curCoords = GetRandomTileCoordsOnMap();
		} while (GetTileFromCoords( curCoords )->GetDef().m_tileType != TileType::Land);

		int	numOfSteps = m_mapRNG.RollRandomIntInRange( 20, 50 );
		DoWormRandomTileGeneration( triangleDef, curCoords, numOfSteps );
	}

	// add square
	TileDefinition const& squareDef = TileDefinition::GetDef( "Copper" );

	for (int i = 0; i < TILE_MAP_SIZE_X * TILE_MAP_SIZE_Y / 10000; ++i) {
		// rare
		IntVec2	curCoords;
		do {
			curCoords = GetRandomTileCoordsOnMap();
		} while (GetTileFromCoords( curCoords )->GetDef().m_tileType != TileType::Land);

		int	numOfSteps = m_mapRNG.RollRandomIntInRange( 20, 50 );
		DoWormRandomTileGeneration( squareDef, curCoords, numOfSteps );
	}

	// add hexagon
	TileDefinition const& hexagonDef = TileDefinition::GetDef( "Coal" );

	for (int i = 0; i < TILE_MAP_SIZE_X * TILE_MAP_SIZE_Y / 20000; ++i) {
		// rare
		IntVec2	curCoords;
		do {
			curCoords = GetRandomTileCoordsOnMap();
		} while (GetTileFromCoords( curCoords )->GetDef().m_tileType != TileType::Land || GetTaxicabDistance2D( curCoords, center ) < (TILE_MAP_SIZE_X + TILE_MAP_SIZE_Y) / 10);

		int	numOfSteps = m_mapRNG.RollRandomIntInRange( 6, 25 );
		DoWormRandomTileGeneration( hexagonDef, curCoords, numOfSteps );
	}

	// add pentagon
	TileDefinition const& pentagonDef = TileDefinition::GetDef( "Coal" );

	for (int i = 0; i < TILE_MAP_SIZE_X * TILE_MAP_SIZE_Y / 20000; ++i) {
		// rare
		IntVec2	curCoords;
		do {
			curCoords = GetRandomTileCoordsOnMap();
		} while (GetTileFromCoords( curCoords )->GetDef().m_tileType != TileType::Land);

		int	numOfSteps = m_mapRNG.RollRandomIntInRange( 10, 50 );
		DoWormRandomTileGeneration( pentagonDef, curCoords, numOfSteps );
	}

	// make walls
	TileDefinition const& wallDef = TileDefinition::GetDef( "Wall" );

	for (int i = 0; i < TILE_MAP_SIZE_X * TILE_MAP_SIZE_Y / 2500; ++i) {
		IntVec2 wallStartCoords = GetRandomTileCoordsOnMap();
		do {
			wallStartCoords = GetRandomTileCoordsOnMap();
		} while (GetTileFromCoords( wallStartCoords )->GetDef().m_tileType != TileType::Land);

		int	numOfSteps = m_mapRNG.RollRandomIntInRange( 100, 250 );
		DoPerfectWormRandomTileGeneration( wallDef, wallStartCoords, numOfSteps );
	}

	// do tile heat map calculation
	m_heatMapToBase = new TileHeatMap( m_dimensions );
	PopulateTileHeatMapDistanceField( *m_heatMapToBase, center, FLT_MAX, 0.f, false );

	// make all blocks that cannot reach to be wall
	for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++) {
		IntVec2 coord = GetCoordsFromTileIndex( i );
		if (m_heatMapToBase->GetTileValue( coord ) == FLT_MAX && !IsTileSolid( coord ) && !IsTileBuilding( coord )) {
			GetTileFromCoords( coord )->SetDef( wallDef );
		}
	}


	// for test
	GetTileFromCoords( IntVec2( 485, 493 ) )->SetDef( hexagonDef );
	GetTileFromCoords( IntVec2( 485, 492 ) )->SetDef( hexagonDef );
	GetTileFromCoords( IntVec2( 486, 492 ) )->SetDef( hexagonDef );

	GetTileFromCoords( IntVec2( 505, 487 ) )->SetDef( squareDef );
	GetTileFromCoords( IntVec2( 505, 486 ) )->SetDef( squareDef );
	GetTileFromCoords( IntVec2( 506, 486 ) )->SetDef( squareDef );

	std::vector<Vertex_PCU> tileMapVerts;
	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		int uvMinX = m_mapRNG.RollRandomIntLessThan( 100 );
		if (uvMinX >= 16) {
			uvMinX = m_mapRNG.RollRandomIntLessThan( 4 );
		}
		AddVertsForAABB2D( tileMapVerts, AABB2( coords, Vec2( coords ) + Vec2( 1.f, 1.f ) ), m_tiles[i].GetDef().m_tintColor, AABB2( Vec2( (float)uvMinX / 16.f, 0.f ), Vec2( (float)(uvMinX + 1) / 16.f, 0.44f ) ) );
	}
	m_tileMapVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_tileMapVertexBuffer );

	tileMapVerts.clear();
	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		if (m_tiles[i].GetDef().m_tileType == TileType::Catalyst) {
			int uvMinX = m_mapRNG.RollRandomIntLessThan( 4 );
			AddVertsForAABB2D( tileMapVerts, AABB2( coords, Vec2( coords ) + Vec2( 1.f, 1.f ) ), m_tiles[i].GetDef().m_tintColor, AABB2( Vec2( (float)uvMinX / 4.f, 0.f ), Vec2( (float)(uvMinX + 1) / 4.f, 1.f ) ) );
		}
	}

	if (tileMapVerts.size() > 0) {
		m_catalystTerrainVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_catalystTerrainVertexBuffer );
	}

	constexpr int numOfParticles = 20;
	constexpr float particleSize = 0.25f;
	tileMapVerts.clear();
	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		if (m_tiles[i].GetDef().m_tileType == TileType::Stone) {
			for (int j = 0; j < numOfParticles; ++j) {
				int uvMinX = m_mapRNG.RollRandomIntLessThan( 4 );
				Vec2 particleCenter = Vec2( m_mapRNG.RollRandomFloatZeroToOne(), m_mapRNG.RollRandomFloatZeroToOne() ) + Vec2( coords );
				AddVertsForAABB2D( tileMapVerts, AABB2( particleCenter - Vec2( particleSize, particleSize ), particleCenter + Vec2( particleSize, particleSize ) ), m_tiles[i].GetDef().m_tintColor, AABB2( Vec2( (float)uvMinX / 4.f, 0.f ), Vec2( (float)(uvMinX + 1) / 4.f, 1.f ) ) );
			}
		}
	}

	if (tileMapVerts.size() > 0) {
		m_stoneTerrainVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_stoneTerrainVertexBuffer );
	}
	tileMapVerts.clear();
	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		if (m_tiles[i].GetDef().m_tileType == TileType::Iron) {
			for (int j = 0; j < numOfParticles; ++j) {
				int uvMinX = m_mapRNG.RollRandomIntLessThan( 4 );
				Vec2 particleCenter = Vec2( m_mapRNG.RollRandomFloatZeroToOne(), m_mapRNG.RollRandomFloatZeroToOne() ) + Vec2( coords );
				AddVertsForAABB2D( tileMapVerts, AABB2( particleCenter - Vec2( particleSize, particleSize ), particleCenter + Vec2( particleSize, particleSize ) ), m_tiles[i].GetDef().m_tintColor, AABB2( Vec2( (float)uvMinX / 4.f, 0.f ), Vec2( (float)(uvMinX + 1) / 4.f, 1.f ) ) );
			}
		}
	}

	if (tileMapVerts.size() > 0) {
		m_ironTerrainVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_ironTerrainVertexBuffer );
	}
	tileMapVerts.clear();

	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		if (m_tiles[i].GetDef().m_tileType == TileType::Copper) {
			for (int j = 0; j < numOfParticles; ++j) {
				int uvMinX = m_mapRNG.RollRandomIntLessThan( 4 );
				Vec2 particleCenter = Vec2( m_mapRNG.RollRandomFloatZeroToOne(), m_mapRNG.RollRandomFloatZeroToOne() ) + Vec2( coords );
				AddVertsForAABB2D( tileMapVerts, AABB2( particleCenter - Vec2( particleSize, particleSize ), particleCenter + Vec2( particleSize, particleSize ) ), m_tiles[i].GetDef().m_tintColor, AABB2( Vec2( (float)uvMinX / 4.f, 0.f ), Vec2( (float)(uvMinX + 1) / 4.f, 1.f ) ) );
			}
		}
	}
	if (tileMapVerts.size() > 0) {
		m_copperTerrainVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_copperTerrainVertexBuffer );
	}
	tileMapVerts.clear();

	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		if (m_tiles[i].GetDef().m_tileType == TileType::Coal) {
			for (int j = 0; j < numOfParticles; ++j) {
				int uvMinX = m_mapRNG.RollRandomIntLessThan( 4 );
				Vec2 particleCenter = Vec2( m_mapRNG.RollRandomFloatZeroToOne(), m_mapRNG.RollRandomFloatZeroToOne() ) + Vec2( coords );
				AddVertsForAABB2D( tileMapVerts, AABB2( particleCenter - Vec2( particleSize, particleSize ), particleCenter + Vec2( particleSize, particleSize ) ), m_tiles[i].GetDef().m_tintColor, AABB2( Vec2( (float)uvMinX / 4.f, 0.f ), Vec2( (float)(uvMinX + 1) / 4.f, 1.f ) ) );
			}
		}
	}
	if (tileMapVerts.size() > 0) {
		m_coalTerrainVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_coalTerrainVertexBuffer );
	}
	tileMapVerts.clear();

	for (int i = 0; i < (int)m_tiles.size(); ++i) {
		IntVec2 coords = GetCoordsFromTileIndex( i );
		if (m_tiles[i].GetDef().m_tileType == TileType::Wall) {
			AddVertsForAABB2D( tileMapVerts, AABB2( coords, Vec2( coords ) + Vec2( 1.f, 1.f ) ), m_tiles[i].GetDef().m_tintColor );
		}
	}
	if (tileMapVerts.size() > 0) {
		m_wallVertexBuffer = g_theRenderer->CreateVertexBuffer( tileMapVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( tileMapVerts.data(), tileMapVerts.size() * sizeof( Vertex_PCU ), m_wallVertexBuffer );
	}

	// for test
	m_curPreBuildingDir = Direction::Down;
	BuildNewDrill( IntVec2( 498, 505 ) );
	BuildNewDrill( IntVec2( 499, 505 ) );
	BuildNewDrill( IntVec2( 513, 509 ) );
	BuildNewDrill( IntVec2( 514, 509 ) );

	for (int i = 498; i <= 508; ++i) {
		BuildNewConveyer( IntVec2( 513, i ) );
		BuildNewConveyer( IntVec2( 514, i ) );
	}
	BuildNewConveyer( IntVec2( 513, 494 ) );
	BuildNewConveyer( IntVec2( 513, 495 ) );
	BuildNewConveyer( IntVec2( 513, 496 ) );
	BuildNewConveyer( IntVec2( 513, 497 ) );

	BuildNewDrill( IntVec2( 485, 493 ) );
	m_curPreBuildingDir = Direction::Right;
	BuildNewDrill( IntVec2( 485, 492 ) );
	BuildNewDrill( IntVec2( 486, 492 ) );
	for (int i = 487; i < 506; ++i) {
		BuildNewConveyer( IntVec2( i, 492 ) );
	}
	BuildNewRouter( IntVec2( 492, 492 ) );
	BuildNewPowerPlant( IntVec2( 492, 493 ) );
	BuildNewRouter( IntVec2( 491, 492 ) );
	BuildNewPowerPlant( IntVec2( 491, 493 ) );

	BuildNewRefinery( IntVec2( 506, 495 ) );
	BuildNewRefinery( IntVec2( 508, 495 ) );

	BuildNewBlender( IntVec2( 506, 498 ) );
	BuildNewBlender( IntVec2( 508, 498 ) );

	m_curPreBuildingDir = Direction::Up;
	BuildNewDrill( IntVec2( 505, 487 ) );
	BuildNewDrill( IntVec2( 505, 486 ) );
	BuildNewDrill( IntVec2( 506, 486 ) );

	BuildNewConveyer( IntVec2( 505, 488 ) );
	BuildNewConveyer( IntVec2( 505, 489 ) );
	BuildNewConveyer( IntVec2( 505, 490 ) );
	BuildNewConveyer( IntVec2( 506, 487 ) );
	BuildNewConveyer( IntVec2( 506, 488 ) );
	BuildNewConveyer( IntVec2( 506, 489 ) );
	BuildNewConveyer( IntVec2( 506, 490 ) );

	BuildNewConveyer( IntVec2( 506, 500 ) );
	BuildNewConveyer( IntVec2( 506, 501 ) );
	BuildNewConveyer( IntVec2( 506, 502 ) );

	m_curPreBuildingDir = Direction::Left;
	BuildNewConveyer( IntVec2( 508, 500 ) );
	BuildNewConveyer( IntVec2( 507, 500 ) );

	BuildNewPowerNode( IntVec2( 496, 494 ) );
	BuildNewPowerNode( IntVec2( 500, 495 ) );
	BuildNewPowerNode( IntVec2( 503, 495 ) );
	BuildNewPowerNode( IntVec2( 503, 495 ) );
	BuildNewPowerNode( IntVec2( 503, 502 ) );
	BuildNewConveyer( IntVec2( 514, 498 ) );
	BuildNewConveyer( IntVec2( 513, 493 ) );
	BuildNewConveyer( IntVec2( 512, 493 ) );
	BuildNewConveyer( IntVec2( 511, 493 ) );
	BuildNewConveyer( IntVec2( 510, 493 ) );
	BuildNewConveyer( IntVec2( 509, 493 ) );
	BuildNewConveyer( IntVec2( 508, 493 ) );

	m_curPreBuildingDir = Direction::Up;
	BuildNewConveyer( IntVec2( 506, 492 ) );
	BuildNewConveyer( IntVec2( 506, 493 ) );
	BuildNewConveyer( IntVec2( 506, 494 ) );
	BuildNewConveyer( IntVec2( 506, 497 ) );
	BuildNewConveyer( IntVec2( 507, 493 ) );
	BuildNewConveyer( IntVec2( 507, 494 ) );


	m_curPreBuilding = BuildingType::ThreeDirectionsPike;
	BuildNewTower( IntVec2( 506, 503 ) );
	m_curPreBuilding = BuildingType::None;

// 	SpawnNewEnemy( Vec2( 480.f, 500.f ), 0.f, EnemyDefinition::GetDefinition( 0 ) );
// 	SpawnNewEnemy( Vec2( 480.f, 501.f ), 0.f, EnemyDefinition::GetDefinition( 0 ) );
// 	SpawnNewEnemy( Vec2( 480.f, 502.f ), 0.f, EnemyDefinition::GetDefinition( 0 ) );
// 	SpawnNewEnemy( Vec2( 480.f, 503.f ), 0.f, EnemyDefinition::GetDefinition( 0 ) );
}

void Map::Update()
{
	g_conveyorAnimTimer->DecrementPeriodIfElapsed();
	HandleResourceMovement();
	UpdateAllPowerNetworks();
	HandleProduction();
	UpdateEntities();
	CheckCollision();
	HandleKeys();
	RemoveAllGarbageResource();
	RemoveAllGarbageEntity();
}

void Map::Render() const
{
	if (m_tileMapVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/grass.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_tileMapVertexBuffer, m_tileMapVertexBuffer->GetVertexCount() );
	}

	if (m_catalystTerrainVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/catalyst.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_catalystTerrainVertexBuffer, m_catalystTerrainVertexBuffer->GetVertexCount() );
	}

	if (m_wallVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/big-rock.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_wallVertexBuffer, m_wallVertexBuffer->GetVertexCount() );
	}

	if (m_ironTerrainVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/iron-ore.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_ironTerrainVertexBuffer, m_ironTerrainVertexBuffer->GetVertexCount() );
	}

	if (m_stoneTerrainVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/stone.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_stoneTerrainVertexBuffer, m_stoneTerrainVertexBuffer->GetVertexCount() );
	}

	if (m_copperTerrainVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/copper-ore.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_copperTerrainVertexBuffer, m_copperTerrainVertexBuffer->GetVertexCount() );
	}

	if (m_coalTerrainVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain/coal.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexBuffer( m_coalTerrainVertexBuffer, m_coalTerrainVertexBuffer->GetVertexCount() );
	}

	for (auto const& pair : m_buildings) {
		pair.second->m_hasRendered = false;
	}

	for (auto const& pair : m_buildings) {
		if (pair.second->m_buildingType == BuildingType::Conveyer && !pair.second->m_hasRendered) {
			pair.second->Render();
			pair.second->m_hasRendered = true;
		}
	}

	for (auto resource : m_resources) {
		if (resource) {
			resource->Render();
		}
	}

	for (auto const& pair : m_buildings) {
		if (pair.second->m_buildingType != BuildingType::Conveyer && !pair.second->m_hasRendered) {
			pair.second->Render();
			pair.second->m_hasRendered = true;
		}
	}

	RenderEntities();

	std::vector<Vertex_PCU> verts;
	IntVec2 cursorCoords = GetTileCoordsFromPos( g_theGame->m_worldCamera.GetCursorWorldPosition( g_theInput->GetCursorNormalizedPosition() ) );

	for (auto const& pair : m_buildings) {
		pair.second->BuildingAddVertsForHealthBar( verts );
	}

	for (auto entity : m_entities) {
		if (entity) {
			entity->EntityAddVertsForHealthBar( verts );
		}
	}

	if (m_UIState != MapUIState::BuildingSettings) {
		AddVertForCursor( verts, cursorCoords );
	}
	
	if (m_curPreBuilding == BuildingType::Conveyer) {
		if (CheckIfConveyerCanBeBuilt( cursorCoords )) {
			DrawConveyer( nullptr, cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 150 );
		}
		else {
			DrawConveyer( nullptr, cursorCoords, m_curPreBuildingDir, Rgba8::RED, 150 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Drill) {
		if (CheckIfDrillCanBeBuilt( cursorCoords )) {
			AddVertsForDrill( verts, cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForDrill( verts, cursorCoords, m_curPreBuildingDir, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Selector) {
		if (CheckIfSelectorCanBeBuilt( cursorCoords )) {
			AddVertsForSelector( verts, cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForSelector( verts, cursorCoords, m_curPreBuildingDir, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Router) {
		if (CheckIfRouterCanBeBuilt( cursorCoords )) {
			AddVertsForRouter( verts, cursorCoords, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForRouter( verts, cursorCoords, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::OverflowGate) {
		if (CheckIfOverflowGateCanBeBuilt( cursorCoords )) {
			AddVertsForOverflowGate( verts, cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForOverflowGate( verts, cursorCoords, m_curPreBuildingDir, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::WareHouse) {
		if (CheckIfWareHouseCanBeBuilt( cursorCoords )) {
			DrawWareHouse( cursorCoords, Rgba8::WHITE, 50 );
		}
		else {
			DrawWareHouse( cursorCoords, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Exporter) {
		if (CheckIfExporterCanBeBuilt( cursorCoords )) {
			AddVertsForExporter( verts, cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForExporter( verts, cursorCoords, m_curPreBuildingDir, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Junction) {
		if (CheckIfJunctionCanBeBuilt( cursorCoords )) {
			AddVertsForJunction( verts, cursorCoords, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForJunction( verts, cursorCoords, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Bridge) {
		if (CheckIfBridgeCanBeBuilt( cursorCoords )) {
			DrawBridge( cursorCoords, m_curPreBuildingDir, true, Rgba8::WHITE, 50 );
			if (m_lastBuiltBridge) {
				AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 0.5f, 0.5f ), Vec2( m_lastBuiltBridge->m_leftBottomCoords ) + Vec2( 0.5f, 0.5f ), 0.3f, Rgba8( 0, 255, 0, 150 ) );
			}
		}
		else {
			DrawBridge( cursorCoords, m_curPreBuildingDir, true, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::PowerPlant) {
		if (CheckIfPowerPlantCanBeBuilt( cursorCoords )) {
			AddVertsForPowerPlant( verts, cursorCoords, Rgba8::WHITE, 50 );
		}
		else {
			AddVertsForPowerPlant( verts, cursorCoords, Rgba8::RED, 50 );
		}
	}
	else if (m_curPreBuilding == BuildingType::PowerNode) {
		if (CheckIfPowerNodeCanBeBuilt( cursorCoords )) {
			DrawPowerNode( cursorCoords, Rgba8::WHITE, 150 );
		}
		else {
			DrawPowerNode( cursorCoords, Rgba8::RED, 150 );
		}
		AddVertsForDisc2D( verts, Vec2( cursorCoords ) + Vec2( 0.5f, 0.5f ), PowerProviderRange, Rgba8( 255, 255, 0, 50 ) );
	}
	else if (m_curPreBuilding == BuildingType::Refinery) {
		if (CheckIfRefineryCanBeBuilt( cursorCoords )) {
			DrawRefinery( cursorCoords, Rgba8::WHITE, 100 );
		}
		else {
			DrawRefinery( cursorCoords, Rgba8::RED, 100 );
		}
	}
	else if (m_curPreBuilding == BuildingType::Blender) {
		if (CheckIfBlenderCanBeBuilt( cursorCoords )) {
			DrawBlender( nullptr, cursorCoords, Rgba8::WHITE, 100 );
		}
		else {
			DrawBlender( nullptr, cursorCoords, Rgba8::RED, 100 );
		}
	}
	else if (m_curPreBuilding == BuildingType::StraightArcher) {
		if (CheckIfTowerCanBeBuilt( cursorCoords )) {
			DrawStraightArcher( cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 100 );
		}
		else {
			DrawStraightArcher( cursorCoords, m_curPreBuildingDir, Rgba8::RED, 100 );
		}
		}
	else if (m_curPreBuilding == BuildingType::ThreeDirectionsPike) {
		if (CheckIfTowerCanBeBuilt( cursorCoords )) {
			DrawThreeDirectionPike( cursorCoords, m_curPreBuildingDir, Rgba8::WHITE, 100 );
		}
		else {
			DrawThreeDirectionPike( cursorCoords, m_curPreBuildingDir, Rgba8::RED, 100 );
		}
	}

	if (m_UIState == MapUIState::BuildingSettings) {
		AddVertsForAABB2D( verts, g_theGame->m_worldCamera.m_cameraBox, Rgba8( 100, 100, 100, 100 ) );
	}

	Building* curHoveringBuilding = GetBuildingFromCoords( m_cursorTileCoords );
	if (curHoveringBuilding) {
		PowerBuilding* provider = dynamic_cast<PowerBuilding*>(curHoveringBuilding);
		if (provider && provider->m_netWork) {
			for (auto otherProvider : provider->m_netWork->m_buildingInNetWork) {
				AddVertsForAABB2D( verts, AABB2( otherProvider->GetCenterPos() - Vec2( 0.25f, 0.25f ), otherProvider->GetCenterPos() + Vec2( 0.25f, 0.25f ) ), Rgba8( 255, 0, 255 ) );
			}
		}
	}
	
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr	);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( verts );
}

void Map::RenderUI() const
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;

	// draw top resource bar
	constexpr float MID_X = UI_SIZE_X * 0.5f;
	constexpr float ResourceItemSize = 80.f;
	constexpr float FontHeight = 20.f;
	int numOfKindOfResourcesHave = 0;
	for (int i = 0; i < NumOfProductTypes; ++i) {
		if (m_numOfResource[i] > 0 || i <= 4) {
			++numOfKindOfResourcesHave;
		}
	}
	float startX = MID_X - (float)numOfKindOfResourcesHave * 0.5f * ResourceItemSize;

	AddVertsForAABB2D( verts, AABB2( Vec2( startX, UI_SIZE_Y - 10.f - FontHeight ), Vec2( startX + numOfKindOfResourcesHave * ResourceItemSize, UI_SIZE_Y ) ), Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( verts );
	verts.clear();

	for (int i = 0; i < NumOfProductTypes; ++i) {
		if (m_numOfResource[i] > 0 || i <= 4) {
			DrawResource( Vec2( startX + FontHeight * 0.6f, UI_SIZE_Y - 5.f - FontHeight * 0.5f ), ProductDefinition::GetDefinition( i ), FontHeight * 0.6f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX + FontHeight * 1.5f, UI_SIZE_Y - 5.f - FontHeight ), Vec2( startX + ResourceItemSize, UI_SIZE_Y - 5.f ) ), FontHeight, Stringf( "%d", m_numOfResource[i] ), Rgba8::BLACK, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			startX += ResourceItemSize;
		}
	}

// 	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 772.f ), Vec2( 1600.f, 792.f ) ), 18.f, Stringf( "Circle:%d Triangle:%d Square:%d Hexagon:%d Pentagon:%d",
// 		m_numOfResource[0], m_numOfResource[1], m_numOfResource[2], m_numOfResource[3], m_numOfResource[4] ) );

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 10.f ), Vec2( 1600.f, 30.f ) ), 20.f, Stringf( "Hovering Pos:(%d, %d)", m_cursorTileCoords.x, m_cursorTileCoords.y ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ) );

	Building* buildingHoveringOn = GetBuildingFromCoords( m_cursorTileCoords );
	if (buildingHoveringOn && buildingHoveringOn->m_buildingType == BuildingType::WareHouse) {
		AddVertsForResourcePanel( verts, textVerts, ConvertWorldPosToScreen( m_cursorTileCoords + IntVec2( 1, 0 ) ), ((WareHouse*)buildingHoveringOn)->m_numOfResource );
	}
	PowerBuilding* asPowerBuilding = dynamic_cast<PowerBuilding*>(buildingHoveringOn);
	if (buildingHoveringOn && asPowerBuilding) {
		AddVertsForElectricityPanel( verts, textVerts, ConvertWorldPosToScreen( m_cursorTileCoords + IntVec2( 1, 0 ) ), asPowerBuilding->m_netWork );
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( textVerts );

	g_uiSystem->Render();

}

IntVec2 Map::GetCoordsFromTileIndex( int index ) const
{
	int y = index / m_dimensions.x;
	return IntVec2( index - y * m_dimensions.x, y );
}

Tile* Map::GetTileFromCoords( IntVec2 const& coords ) const
{
	if (coords.x >= 0 && coords.x < m_dimensions.x && coords.y >= 0 && coords.y < m_dimensions.y) {
		return const_cast<Tile*>(&m_tiles[coords.x + coords.y * m_dimensions.x]);
	}
	return nullptr;
}

Building* Map::GetBuildingFromCoords( IntVec2 const& coords ) const
{
	auto iter = m_buildings.find( coords );
	if (iter != m_buildings.end()) {
		return iter->second;
	}
	return nullptr;
}

bool Map::IsCoordsInBounds( IntVec2 const& coords ) const
{
	return coords.x >= 0 && coords.x < m_dimensions.x && coords.y > 0 && coords.y < m_dimensions.y;
}

bool Map::IsTileSolid( IntVec2 const& coords ) const
{
	return const_cast<Map*>(this)->GetTileFromCoords( coords )->GetDef().m_isSolid;
}

bool Map::IsTileBuilding( IntVec2 const& coords ) const
{
	UNUSED( coords );
	return false;
}

IntVec2 Map::GetMapCenter() const
{
	return IntVec2( TILE_MAP_SIZE_X / 2, TILE_MAP_SIZE_Y / 2 );
}

void Map::SetupUI()
{
	AABB2 const filterButtonSize = AABB2( Vec2( 0.f, 0.f ), Vec2( 40.f, 40.f ) );
/*	constexpr float middleX = UI_SIZE_X * 0.5f;*/
	constexpr float padding = 10.f;
	constexpr float buttonSize = 40.f;
	for (int i = 0; i < (int)ProductDefinition::s_definitions.size(); ++i) {
		ProductDefinition const& def = ProductDefinition::s_definitions[i];
		if (def.m_id != -1) {
			SFWidget* widget = g_uiSystem->AddWidgetToSystem( filterButtonSize, def.m_name, "ChooseFilter" );
			widget->m_args.SetValue( "productType", def.m_id );
			widget->m_type = SFWidgetType::Graph;
			widget->m_texture = def.m_texture;
			widget->m_uv = AABB2( Vec2( 0.f, 0.f ), Vec2( 0.5f, 1.f ) );
			int y = i / 10;
			int x = i - i / 10 * 10;
			widget->SetCenter( Vec2( 200.f + float( x + 0.5f ) * buttonSize + x * padding, 600.f - (y + 0.5f) * buttonSize - y * padding ) );
			g_filterUIs.push_back( widget );
		}
	}

	int n = 0;
	for (int i = 0; i < (int)ProductDefinition::s_definitions.size(); ++i) {
		ProductDefinition const& def = ProductDefinition::s_definitions[i];
		if (def.m_id != -1 && def.m_factory == BuildingType::Refinery) {
			SFWidget* widget = g_uiSystem->AddWidgetToSystem( filterButtonSize, def.m_name, "ChooseRefineryProduct" );
			widget->m_args.SetValue( "productType", def.m_id );
			widget->m_type = SFWidgetType::Graph;
			widget->m_texture = def.m_texture;
			widget->m_uv = AABB2( Vec2( 0.f, 0.f ), Vec2( 0.5f, 1.f ) );
			int y = n / 10;
			int x = n - n / 10 * 10;
			widget->SetCenter( Vec2( 200.f + float( x + 0.5f ) * buttonSize + x * padding, 600.f - (y + 0.5f) * buttonSize - y * padding ) );
			g_refineryUIs.push_back( widget );
			++n;
		}
	}

	n = 0;
	for (int i = 0; i < (int)ProductDefinition::s_definitions.size(); ++i) {
		ProductDefinition const& def = ProductDefinition::s_definitions[i];
		if (def.m_id != -1 && def.m_factory == BuildingType::Blender) {
			SFWidget* widget = g_uiSystem->AddWidgetToSystem( filterButtonSize, def.m_name, "ChooseBlenderProduct" );
			widget->m_args.SetValue( "productType", def.m_id );
			widget->m_type = SFWidgetType::Graph;
			widget->m_texture = def.m_texture;
			widget->m_uv = AABB2( Vec2( 0.f, 0.f ), Vec2( 0.5f, 1.f ) );
			int y = n / 10;
			int x = n - n / 10 * 10;
			widget->SetCenter( Vec2( 200.f + float( x + 0.5f ) * buttonSize + x * padding, 600.f - (y + 0.5f) * buttonSize - y * padding ) );
			g_blenderUIs.push_back( widget );
			++n;
		}
	}

	constexpr float constructionWidgetWidth = 80.f;
	constexpr float constructionWidgetHeight = 40.f;
	constexpr float panelButtonPadding = 10.f;
	AABB2 const constructionButtonSize = AABB2( Vec2( 0.f, 0.f ), Vec2( constructionWidgetWidth, constructionWidgetHeight ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Conveyor", "BeginBuildConveyor" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Drill", "BeginBuildDrill" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Selector", "BeginBuildSelector" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Router", "BeginBuildRouter" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "OverflowGate", "BeginBuildOverflowGate" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Ware House", "BeginBuildWarehouse" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Exporter", "BeginBuildExporter" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Bypass", "BeginBuildJunction" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Tunnel", "BeginBuildBridge" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Power Plant", "BeginBuildPowerPlant" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Power Node", "BeginBuildPowerNode" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Blender", "BeginBuildBlender" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Refinery", "BeginBuildRefinery" ) );

	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Wall", "BeginBuildWall" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Gun", "BeginBuildStraightArcher" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Missile", "BeginBuildGuidedMissile" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Pike", "BeginBuildThreeDirectionsPike" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Mortar", "BeginBuildMortar" ) );
	g_constructionPanelUI.push_back( g_uiSystem->AddWidgetToSystem( constructionButtonSize, "Laser", "BeginBuildLaser" ) );

	g_constructionPanelUI[0]->m_args.SetValue( "BuildingType", BuildingType::Conveyer );
	g_constructionPanelUI[1]->m_args.SetValue( "BuildingType", BuildingType::Drill );
	g_constructionPanelUI[2]->m_args.SetValue( "BuildingType", BuildingType::Selector );
	g_constructionPanelUI[3]->m_args.SetValue( "BuildingType", BuildingType::Router );
	g_constructionPanelUI[4]->m_args.SetValue( "BuildingType", BuildingType::OverflowGate );
	g_constructionPanelUI[5]->m_args.SetValue( "BuildingType", BuildingType::WareHouse );
	g_constructionPanelUI[6]->m_args.SetValue( "BuildingType", BuildingType::Exporter );
	g_constructionPanelUI[7]->m_args.SetValue( "BuildingType", BuildingType::Junction );
	g_constructionPanelUI[8]->m_args.SetValue( "BuildingType", BuildingType::Bridge );
	g_constructionPanelUI[9]->m_args.SetValue( "BuildingType", BuildingType::PowerPlant );
	g_constructionPanelUI[10]->m_args.SetValue( "BuildingType", BuildingType::PowerNode );
	g_constructionPanelUI[11]->m_args.SetValue( "BuildingType", BuildingType::Blender );
	g_constructionPanelUI[12]->m_args.SetValue( "BuildingType", BuildingType::Refinery );
	g_constructionPanelUI[13]->m_args.SetValue( "BuildingType", BuildingType::Wall );
	g_constructionPanelUI[14]->m_args.SetValue( "BuildingType", BuildingType::StraightArcher );
	g_constructionPanelUI[15]->m_args.SetValue( "BuildingType", BuildingType::GuidedMissile );
	g_constructionPanelUI[16]->m_args.SetValue( "BuildingType", BuildingType::ThreeDirectionsPike );
	g_constructionPanelUI[17]->m_args.SetValue( "BuildingType", BuildingType::Mortar );
	g_constructionPanelUI[18]->m_args.SetValue( "BuildingType", BuildingType::Laser );

	SubscribeEventCallbackFunction( "BeginBuildConveyor", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildDrill", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildSelector", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildRouter", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildOverflowGate", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildWarehouse", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildExporter", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildJunction", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildBridge", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildPowerPlant", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildPowerNode", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildBlender", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildRefinery", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildWall", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildStraightArcher", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildGuidedMissile", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildThreeDirectionsPike", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildMortar", UISelectionBuildBuilding );
	SubscribeEventCallbackFunction( "BeginBuildLaser", UISelectionBuildBuilding );

	g_constructionPanelUI[0]->SetCenter( Vec2( UI_SIZE_X - 5.f * panelButtonPadding - 4.5f * constructionWidgetWidth, 4.f * panelButtonPadding + 3.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[1]->SetCenter( Vec2( UI_SIZE_X - 4.f * panelButtonPadding - 3.5f * constructionWidgetWidth, 4.f * panelButtonPadding + 3.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[2]->SetCenter( Vec2( UI_SIZE_X - 3.f * panelButtonPadding - 2.5f * constructionWidgetWidth, 4.f * panelButtonPadding + 3.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[3]->SetCenter( Vec2( UI_SIZE_X - 2.f * panelButtonPadding - 1.5f * constructionWidgetWidth, 4.f * panelButtonPadding + 3.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[4]->SetCenter( Vec2( UI_SIZE_X - 1.f * panelButtonPadding - 0.5f * constructionWidgetWidth, 4.f * panelButtonPadding + 3.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[5]->SetCenter( Vec2( UI_SIZE_X - 5.f * panelButtonPadding - 4.5f * constructionWidgetWidth, 3.f * panelButtonPadding + 2.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[6]->SetCenter( Vec2( UI_SIZE_X - 4.f * panelButtonPadding - 3.5f * constructionWidgetWidth, 3.f * panelButtonPadding + 2.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[7]->SetCenter( Vec2( UI_SIZE_X - 3.f * panelButtonPadding - 2.5f * constructionWidgetWidth, 3.f * panelButtonPadding + 2.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[8]->SetCenter( Vec2( UI_SIZE_X - 2.f * panelButtonPadding - 1.5f * constructionWidgetWidth, 3.f * panelButtonPadding + 2.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[9]->SetCenter( Vec2( UI_SIZE_X - 1.f * panelButtonPadding - 0.5f * constructionWidgetWidth, 3.f * panelButtonPadding + 2.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[10]->SetCenter( Vec2( UI_SIZE_X - 5.f * panelButtonPadding - 4.5f * constructionWidgetWidth, 2.f * panelButtonPadding + 1.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[11]->SetCenter( Vec2( UI_SIZE_X - 4.f * panelButtonPadding - 3.5f * constructionWidgetWidth, 2.f * panelButtonPadding + 1.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[12]->SetCenter( Vec2( UI_SIZE_X - 3.f * panelButtonPadding - 2.5f * constructionWidgetWidth, 2.f * panelButtonPadding + 1.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[13]->SetCenter( Vec2( UI_SIZE_X - 2.f * panelButtonPadding - 1.5f * constructionWidgetWidth, 2.f * panelButtonPadding + 1.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[14]->SetCenter( Vec2( UI_SIZE_X - 1.f * panelButtonPadding - 0.5f * constructionWidgetWidth, 2.f * panelButtonPadding + 1.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[15]->SetCenter( Vec2( UI_SIZE_X - 5.f * panelButtonPadding - 4.5f * constructionWidgetWidth, 1.f * panelButtonPadding + 0.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[16]->SetCenter( Vec2( UI_SIZE_X - 4.f * panelButtonPadding - 3.5f * constructionWidgetWidth, 1.f * panelButtonPadding + 0.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[17]->SetCenter( Vec2( UI_SIZE_X - 3.f * panelButtonPadding - 2.5f * constructionWidgetWidth, 1.f * panelButtonPadding + 0.5f * constructionWidgetHeight ) );
	g_constructionPanelUI[18]->SetCenter( Vec2( UI_SIZE_X - 2.f * panelButtonPadding - 1.5f * constructionWidgetWidth, 1.f * panelButtonPadding + 0.5f * constructionWidgetHeight ) );

	g_constructionPanelUI[0]->m_isActive = true;
	g_constructionPanelUI[1]->m_isActive = true;
	g_constructionPanelUI[2]->m_isActive = true;
	g_constructionPanelUI[3]->m_isActive = true;
	g_constructionPanelUI[4]->m_isActive = true;
	g_constructionPanelUI[5]->m_isActive = true;
	g_constructionPanelUI[6]->m_isActive = true;
	g_constructionPanelUI[7]->m_isActive = true;
	g_constructionPanelUI[8]->m_isActive = true;
	g_constructionPanelUI[9]->m_isActive = true;
	g_constructionPanelUI[10]->m_isActive = true;
	g_constructionPanelUI[11]->m_isActive = true;
	g_constructionPanelUI[12]->m_isActive = true;
	g_constructionPanelUI[13]->m_isActive = true;
	g_constructionPanelUI[14]->m_isActive = true;
	g_constructionPanelUI[15]->m_isActive = true;
	g_constructionPanelUI[16]->m_isActive = true;
	g_constructionPanelUI[17]->m_isActive = true;
	g_constructionPanelUI[18]->m_isActive = true;
}

void Map::HandleKeys()
{
	Vec2 normalizedCursorPos = g_theInput->GetCursorNormalizedPosition();
	Vec2 cursorWorldPos = g_theGame->m_worldCamera.GetCursorWorldPosition( normalizedCursorPos );
	Vec2 cursorScreenPos = ConvertWorldPosToScreen( cursorWorldPos );
	m_cursorTileCoords = GetTileCoordsFromPos( cursorWorldPos );
// 	if (m_cursorLastFrameTileCoords != m_cursorTileCoords) {
// 		AABB2 bounds = AABB2( m_cursorTileCoords, Vec2( m_cursorTileCoords ) + Vec2( 1.f, 1.f ) );
// 		bounds.SetDimensions( bounds.GetDimensions() * 0.9f );
// 		if (!IsPointInsideAABB2D( cursorWorldPos, bounds )) {
// 			m_cursorTileCoords = m_cursorLastFrameTileCoords;
// 		}
// 	}
	Tile* cursorHoveringTile = GetTileFromCoords( m_cursorTileCoords );
	Building* cursorHoveringBuilding = GetBuildingFromCoords( m_cursorTileCoords );

	if (m_UIState == MapUIState::ConstructBuilding) {
		// UI will block any input
		if (g_theInput->WasKeyJustReleased( KEYCODE_LEFTMOUSE )) {
			if (g_uiSystem->ExecuteClickEvent( cursorScreenPos )) {
				return;
			}
		}
		if (cursorHoveringTile && g_theInput->WasKeyJustReleased( KEYCODE_LEFTMOUSE )) {
			DoConstruction( m_cursorTileCoords );
		}
		if (cursorHoveringTile && m_cursorTileCoords != m_cursorLastFrameTileCoords && g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
			Direction newDir = GetDir( m_cursorTileCoords - m_cursorLastFrameTileCoords );
			if (newDir != Direction::None) {
				m_curPreBuildingDir = newDir;
			}
			DoConstruction( m_cursorLastFrameTileCoords );
		}
		if (g_theInput->WasKeyJustPressed( 'R' ) && !m_lastBuiltBridge) {
			if (m_curPreBuildingDir == Direction::Down) {
				m_curPreBuildingDir = Direction::Left;
			}
			else if (m_curPreBuildingDir == Direction::Left) {
				m_curPreBuildingDir = Direction::Up;
			}
			else if (m_curPreBuildingDir == Direction::Up) {
				m_curPreBuildingDir = Direction::Right;
			}
			else if (m_curPreBuildingDir == Direction::Right) {
				m_curPreBuildingDir = Direction::Down;
			}
		}
		if (g_theInput->WasKeyJustPressed( '1' )) {
			m_curPreBuilding = BuildingType::Conveyer;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '2' )) {
			m_curPreBuilding = BuildingType::Drill;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '3' )) {
			m_curPreBuilding = BuildingType::Selector;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '4' )) {
			m_curPreBuilding = BuildingType::Router;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '5' )) {
			m_curPreBuilding = BuildingType::OverflowGate;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustReleased( KEYCODE_RIGHTMOUSE ) && m_curPreBuilding != BuildingType::None) {
			m_curPreBuilding = BuildingType::None;
			m_UIState = MapUIState::ViewingMap;
			m_lastBuiltBridge = nullptr;
		}
	}
	else if (m_UIState == MapUIState::ViewingMap) {
		// UI will block any input
		if (g_theInput->WasKeyJustReleased( KEYCODE_LEFTMOUSE )) {
			if (g_uiSystem->ExecuteClickEvent( cursorScreenPos )) {
				return;
			}
		}
		if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE ) && m_curPreBuilding == BuildingType::None) {
			RemoveBuilding( m_cursorTileCoords );
		}

		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) && cursorHoveringBuilding) {
			if (cursorHoveringBuilding->m_buildingType == BuildingType::Selector) {
				UnsubscribeAllEventCallbackFunctionByName( "ChooseFilter" );
				SubscribeEventCallbackFunction( "ChooseFilter", (Selector*)cursorHoveringBuilding, &Selector::ChooseFilter );
				for (int i = 0; i < (int)g_filterUIs.size(); ++i) {
					SetBuildingSettingsUIActive( g_filterUIs[i], g_filterUIs[i]->m_responseArea.GetCenter() );
				}
				m_UIState = MapUIState::BuildingSettings;
				SetConstructionPanelActive( false );
			}
			else if (cursorHoveringBuilding->m_buildingType == BuildingType::Exporter) {
				UnsubscribeAllEventCallbackFunctionByName( "ChooseFilter" );
				SubscribeEventCallbackFunction( "ChooseFilter", (Exporter*)cursorHoveringBuilding, &Exporter::ChooseFilter );
				for (int i = 0; i < (int)g_filterUIs.size(); ++i) {
					SetBuildingSettingsUIActive( g_filterUIs[i], g_filterUIs[i]->m_responseArea.GetCenter() );
				}
				m_UIState = MapUIState::BuildingSettings;
				SetConstructionPanelActive( false );
			}
			else if (cursorHoveringBuilding->m_buildingType == BuildingType::Refinery) {
				UnsubscribeAllEventCallbackFunctionByName( "ChooseRefineryProduct" );
				SubscribeEventCallbackFunction( "ChooseRefineryProduct", (Factory*)cursorHoveringBuilding, &Factory::ChooseProduct );
				for (int i = 0; i < (int)g_refineryUIs.size(); ++i) {
					SetBuildingSettingsUIActive( g_refineryUIs[i], g_refineryUIs[i]->m_responseArea.GetCenter() );
				}
				m_UIState = MapUIState::BuildingSettings;
				SetConstructionPanelActive( false );
			}
			else if (cursorHoveringBuilding->m_buildingType == BuildingType::Blender) {
				Vec2 basePos = ConvertWorldPosToScreen( cursorHoveringBuilding->GetCenterPos() );
				UnsubscribeAllEventCallbackFunctionByName( "ChooseBlenderProduct" );
				SubscribeEventCallbackFunction( "ChooseBlenderProduct", (Factory*)cursorHoveringBuilding, &Factory::ChooseProduct );
				for (int i = 0; i < (int)g_blenderUIs.size(); ++i) {
					SetBuildingSettingsUIActive( g_blenderUIs[i], g_blenderUIs[i]->m_responseArea.GetCenter() );
				}
				m_UIState = MapUIState::BuildingSettings;
				SetConstructionPanelActive( false );
			}
		}

		if (g_theInput->WasKeyJustPressed( '1' )) {
			m_curPreBuilding = BuildingType::Conveyer;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '2' )) {
			m_curPreBuilding = BuildingType::Drill;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '3' )) {
			m_curPreBuilding = BuildingType::Selector;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '4' )) {
			m_curPreBuilding = BuildingType::Router;
			m_UIState = MapUIState::ConstructBuilding;
		}
		if (g_theInput->WasKeyJustPressed( '5' )) {
			m_curPreBuilding = BuildingType::OverflowGate;
			m_UIState = MapUIState::ConstructBuilding;
		}
	}
	else if (m_UIState == MapUIState::BuildingSettings) {
		if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTMOUSE )) {
			DeActiveAllUIWidget();
			m_UIState = MapUIState::ViewingMap;
			SetConstructionPanelActive( true );
		}
		else if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			if (!g_uiSystem->ExecuteClickEvent( cursorScreenPos )) {
				DeActiveAllUIWidget();
				m_UIState = MapUIState::ViewingMap;
				SetConstructionPanelActive( true );
			}
		}
	}


	if (g_theInput->WasKeyJustPressed( 'M' )) {
		SpawnNewEnemy( cursorWorldPos, 0.f, EnemyDefinition::GetDefinition( 0 ) );
	}

	m_cursorLastFrameTileCoords = m_cursorTileCoords;
}

void Map::HandleResourceMovement()
{
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	// all resources pre-move
	for (auto resource : m_resources) {
		if (resource) {
			resource->PreMove( resource->m_conveyerOn, deltaSeconds );
		}
	}
	// check availability
	for (int i = 0; i < (int)m_resources.size(); ++i) {
		if (m_resources[i] && m_resources[i]->m_conveyerOn) {
			Resource* r1 = m_resources[i];
			Building* targetBuilding = GetBuildingFromCoords( GetTileCoordsFromPos( r1->m_movementTargetPos ) );
			// cannot enter building cases
			if (!targetBuilding) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->IsFull() && !targetBuilding->m_neverFull) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Selector && !((Selector*)targetBuilding)->CanResourceMoveInto(r1)){
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::OverflowGate && !((OverflowGate*)targetBuilding)->CanResourceMoveInto(r1)) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Drill && !((Drill*)targetBuilding)->CanResourceMoveInto( r1 )) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Bridge && !((Bridge*)targetBuilding)->CanResourceMoveInto( r1 )) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Conveyer) {
				if (((Conveyer*)targetBuilding)->m_dir != r1->m_conveyerOn->m_dir && ((Conveyer*)targetBuilding)->HasMoreThanOneEntrance()){
					for (auto resource : m_resources) {
						if (resource && resource->m_conveyerOn == targetBuilding) {
							if (DoDiscsOverlap( resource->m_centerPos, 0.f, targetBuilding->GetCenterPos(), RESOURCE_RADIUS * 1.6f )) {
								r1->ReconsiderMovement();
								break;
							}
						}
					}
				}
			}
			else if (targetBuilding->m_buildingType == BuildingType::Junction && !((Junction*)targetBuilding)->CanResourceMoveInto( r1 )) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::PowerPlant && !((PowerPlant*)targetBuilding)->CanResourceMoveInto( r1 )) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::PowerNode) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Blender && !((Factory*)targetBuilding)->CanResourceMoveInto(r1)) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Refinery && !((Factory*)targetBuilding)->CanResourceMoveInto( r1 )) {
				r1->ReconsiderMovement();
			}
			else if (targetBuilding->m_buildingType == BuildingType::Wall) {
				r1->ReconsiderMovement();
			}
			else if ((targetBuilding->m_buildingType == BuildingType::StraightArcher
				|| targetBuilding->m_buildingType == BuildingType::GuidedMissile
				|| targetBuilding->m_buildingType == BuildingType::ThreeDirectionsPike
				|| targetBuilding->m_buildingType == BuildingType::Mortar
				|| targetBuilding->m_buildingType == BuildingType::Laser) && !((TowerBase*)targetBuilding)->CanResourceMoveInto( r1 )) {
				r1->ReconsiderMovement();
			}
			
			for (int j = i + 1; j < (int)m_resources.size(); ++j) {
				if (m_resources[j] && m_resources[j]->m_conveyerOn) {
					Resource* r2 = m_resources[j];
					// method to save resources stop together
// 					if (DoDiscsOverlap( r1->m_centerPos, RESOURCE_RADIUS - 0.0001f, r2->m_centerPos, RESOURCE_RADIUS - 0.0001f )) {
// 						if ((r1->m_conveyerOn->m_dir == Direction::Up || r1->m_conveyerOn->m_dir == Direction::Down)
// 							&& r1->m_centerPos.x == (float)r1->m_conveyerOn->m_leftBottomCoords.x + 0.5f) {
// 							PushDiscOutOfFixedDisc2D( r2->m_centerPos, RESOURCE_RADIUS, r1->m_centerPos, RESOURCE_RADIUS );
// 						}
// 						else if ((r1->m_conveyerOn->m_dir == Direction::Left || r1->m_conveyerOn->m_dir == Direction::Right)
// 							&& r1->m_centerPos.y == (float)r1->m_conveyerOn->m_leftBottomCoords.y + 0.5f) {
// 							PushDiscOutOfFixedDisc2D( r2->m_centerPos, RESOURCE_RADIUS, r1->m_centerPos, RESOURCE_RADIUS );
// 						}
// 						else {
// 							PushDiscOutOfFixedDisc2D( r1->m_centerPos, RESOURCE_RADIUS, r2->m_centerPos, RESOURCE_RADIUS );
// 						}
// 					}
					if (DoDiscsOverlap( r1->m_movementTargetPos, RESOURCE_RADIUS, r2->m_movementTargetPos, RESOURCE_RADIUS )) {

						// straight going resource has the priority
// 						if (r1->m_hasPriority && !r2->m_hasPriority) {
// 							r2->ReconsiderMovement( r1 );
// 						}
// 						else if (!r1->m_hasPriority && r2->m_hasPriority) {
// 							r1->ReconsiderMovement( r2 );
// 						}
// 						else { // most eager to go to the exit one has the priority
							Vec2 exitPos = r1->m_conveyerOn->GetExitPos();
							if ((abs( exitPos.x - r1->m_movementTargetPos.x ) + abs( exitPos.y - r1->m_movementTargetPos.y )) >
								(abs( exitPos.x - r2->m_movementTargetPos.x ) + abs( exitPos.y - r2->m_movementTargetPos.y ))) {
								r1->ReconsiderMovement( r2 );
							}
							else {
								r2->ReconsiderMovement( r1 );
							}
						/*}*/

							/*if (DotProduct2D(DirectionUnitVec[(int)r1->m_conveyerOn->m_dir], r1->m_movementTargetPos - r1->m_centerPos) >
								DotProduct2D( DirectionUnitVec[(int)r2->m_conveyerOn->m_dir], r2->m_movementTargetPos - r2->m_centerPos )) {
								r2->ReconsiderMovement( r1 );
							}
							else {
								r1->ReconsiderMovement( r2 );
							}*/
/*						}*/
					}
				}
			}
		}
	}
	// resources move
	for (auto resource : m_resources) {
		if (resource) {
			resource->Move();
		}
	}
}

void Map::HandleProduction()
{
	for (auto building : m_buildingList) {
		if (building->m_canProduce) {
			building->Produce();
		}
	}
}

void Map::UpdateAllPowerNetworks()
{
	for (auto network : m_powerNetworks) {
		network->Update();
	}
}

void Map::UpdateEntities()
{
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	for (auto entity : m_entities) {
		if (entity) {
			entity->Update( deltaSeconds );
		}
	}
}

void Map::CheckCollision()
{
	for (int i = 0; i < (int)m_entities.size(); ++i) {
		if (!m_entities[i] || !m_entities[i]->IsAlive()) {
			continue;
		}
		for (int j = 0; j < (int)m_entities.size(); ++j) {
			if (!m_entities[j] || !m_entities[j]->IsAlive()) {
				continue;
			}
			if (DoEntitiesHaveInteraction( m_entities[i], m_entities[j] ) 
				&& DoAABB2sOverlap2D(m_entities[i]->GetWorldPhysicsBounds(), m_entities[j]->GetWorldPhysicsBounds())) {
				m_entities[i]->BeAttacked( m_entities[j]->m_damage );
				m_entities[j]->BeAttacked( m_entities[i]->m_damage );
			}
			if (m_entities[i]->m_entityType == EntityType::Enemy && m_entities[j]->m_entityType == EntityType::Enemy) {
				PushDiscsOutOfEachOther2D( m_entities[i]->m_position, m_entities[i]->m_physicsDiscRadius, m_entities[j]->m_position, m_entities[j]->m_physicsDiscRadius );
			}
		}
	}
}

bool Map::CheckResourceJumpQueue( Resource* resource ) const
{
	Conveyer* conveyer = resource->m_conveyerOn;
	if (resource->m_centerPos.x != conveyer->GetCenterPos().x && (conveyer->m_dir == Direction::Down || conveyer->m_dir == Direction::Up)) {
		return true;
	}
	else if (resource->m_centerPos.y != conveyer->GetCenterPos().y && (conveyer->m_dir == Direction::Left || conveyer->m_dir == Direction::Right)) {
		return true;
	}
	return false;
}

void Map::AddEntityToList( Entity* entity )
{
	for (size_t i = 0; i < m_entities.size(); ++i) {
		if (m_entities[i] == nullptr) {
			m_entities[i] = entity;
			return;
		}
	}
	m_entities.push_back( entity );
}

void Map::RemoveEntityFromList( Entity* entity )
{
	for (size_t i = 0; i < m_entities.size(); ++i) {
		if (m_entities[i] == entity) {
			m_entities[i] = nullptr;
			return;
		}
	}
}

void Map::RenderEntities() const
{
	for (auto entity : m_entities) {
		if (entity && entity->m_entityType != EntityType::Building) {
			entity->Render();
		}
	}
}

bool Map::CheckIfConveyerCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords )) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
		return true;
	}
	if (!buildingOnTile) {
		return true;
	}
	return false;
}

bool Map::CheckIfDrillCanBeBuilt( IntVec2 coords ) const
{
	TileDefinition const& tileDef = GetTileFromCoords( coords )->GetDef();
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (!buildingOnTile &&
		(tileDef.m_tileType == TileType::Iron
			|| tileDef.m_tileType == TileType::Coal
			|| tileDef.m_tileType == TileType::Catalyst
			|| tileDef.m_tileType == TileType::Copper
			|| tileDef.m_tileType == TileType::Stone)) {
		return true;
	}
	return false;
}

bool Map::CheckIfLogisticBuildingCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords )) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (!buildingOnTile) {
		return true;
	}
	if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
		return true;
	}
	return false;
}

bool Map::CheckIfSelectorCanBeBuilt( IntVec2 coords ) const
{
	return CheckIfLogisticBuildingCanBeBuilt( coords );
}

bool Map::CheckIfRouterCanBeBuilt( IntVec2 coords ) const
{
	return CheckIfLogisticBuildingCanBeBuilt( coords );
}

bool Map::CheckIfOverflowGateCanBeBuilt( IntVec2 coords ) const
{
	return CheckIfLogisticBuildingCanBeBuilt( coords );
}

bool Map::CheckIfWareHouseCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords )) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (!buildingOnTile) {
		return true;
	}
	return false;
}

bool Map::CheckIfExporterCanBeBuilt( IntVec2 coords ) const
{
	return CheckIfLogisticBuildingCanBeBuilt( coords );
}

bool Map::CheckIfJunctionCanBeBuilt( IntVec2 coords ) const
{
	return CheckIfLogisticBuildingCanBeBuilt( coords );
}

bool Map::CheckIfBridgeCanBeBuilt( IntVec2 coords ) const
{
	if (m_lastBuiltBridge == nullptr) {
		return CheckIfLogisticBuildingCanBeBuilt( coords );
	}
	else {
		if (m_lastBuiltBridge->m_dir == Direction::Down) {
			if (coords.x == m_lastBuiltBridge->m_leftBottomCoords.x && coords.y <= m_lastBuiltBridge->m_leftBottomCoords.y - 2 && coords.y >= m_lastBuiltBridge->m_leftBottomCoords.y - 9 ) {
				return CheckIfLogisticBuildingCanBeBuilt( coords );
			}
			return false;
		}
		else if (m_lastBuiltBridge->m_dir == Direction::Up) {
			if (coords.x == m_lastBuiltBridge->m_leftBottomCoords.x && coords.y >= m_lastBuiltBridge->m_leftBottomCoords.y + 2 && coords.y <= m_lastBuiltBridge->m_leftBottomCoords.y + 9 ) {
				return CheckIfLogisticBuildingCanBeBuilt( coords );
			}
			return false;
		}
		else if (m_lastBuiltBridge->m_dir == Direction::Left) {
			if (coords.y == m_lastBuiltBridge->m_leftBottomCoords.y && coords.x <= m_lastBuiltBridge->m_leftBottomCoords.x - 2 && coords.x >= m_lastBuiltBridge->m_leftBottomCoords.x - 9) {
				return CheckIfLogisticBuildingCanBeBuilt( coords );
			}
			return false;
		}
		else if (m_lastBuiltBridge->m_dir == Direction::Right) {
			if (coords.y == m_lastBuiltBridge->m_leftBottomCoords.y && coords.x >= m_lastBuiltBridge->m_leftBottomCoords.x + 2 && coords.x <= m_lastBuiltBridge->m_leftBottomCoords.x + 9) {
				return CheckIfLogisticBuildingCanBeBuilt( coords );
			}
			return false;
		}
	}
	return false;
}

bool Map::CheckIfPowerPlantCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords )) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (!buildingOnTile) {
		return true;
	}
	return false;
}

bool Map::CheckIfPowerNodeCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords )) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (!buildingOnTile) {
		return true;
	}
	return false;
}

bool Map::CheckIfBlenderCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords ) || IsTileSolid(coords + IntVec2(1, 1)) || IsTileSolid(coords + IntVec2(0, 1)) || IsTileSolid(coords + IntVec2(1, 0))) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (buildingOnTile) {
		return false;
	}
	buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 1, 1 ) );
	if (buildingOnTile) {
		return false;
	}
	buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 1, 0 ) );
	if (buildingOnTile) {
		return false;
	}
	buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 0, 1 ) );
	if (buildingOnTile) {
		return false;
	}
	return true;
}

bool Map::CheckIfRefineryCanBeBuilt( IntVec2 coords ) const
{
	if (IsTileSolid( coords ) || IsTileSolid( coords + IntVec2( 1, 1 ) ) || IsTileSolid( coords + IntVec2( 0, 1 ) ) || IsTileSolid( coords + IntVec2( 1, 0 ) )) {
		return false;
	}
	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (buildingOnTile) {
		return false;
	}
	buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 1, 1 ) );
	if (buildingOnTile) {
		return false;
	}
	buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 1, 0 ) );
	if (buildingOnTile) {
		return false;
	}
	buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 0, 1 ) );
	if (buildingOnTile) {
		return false;
	}
	return true;
}

bool Map::CheckIfTowerCanBeBuilt( IntVec2 coords ) const
{
	if (m_curPreBuilding == BuildingType::Mortar || m_curPreBuilding == BuildingType::GuidedMissile) {
		if (IsTileSolid( coords ) || IsTileSolid( coords + IntVec2( 1, 1 ) ) || IsTileSolid( coords + IntVec2( 0, 1 ) ) || IsTileSolid( coords + IntVec2( 1, 0 ) )) {
			return false;
		}
		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile) {
			return false;
		}
		buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 1, 1 ) );
		if (buildingOnTile) {
			return false;
		}
		buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 1, 0 ) );
		if (buildingOnTile) {
			return false;
		}
		buildingOnTile = GetBuildingFromCoords( coords + IntVec2( 0, 1 ) );
		if (buildingOnTile) {
			return false;
		}
		return true;
	}
	else {
		if (IsTileSolid( coords )) {
			return false;
		}
		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile) {
			return false;
		}
		return true;
	}
}

void Map::BuildNewConveyer( IntVec2 coords )
{
	if (!CheckIfConveyerCanBeBuilt( coords )) {
		return;
	}

	Building* buildingOnTile = GetBuildingFromCoords( coords );
	if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer && ((Conveyer*)buildingOnTile)->m_dir != m_curPreBuildingDir) {
		Building* buildingBackward = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetInversedDir( ((Conveyer*)buildingOnTile)->m_dir )] );
		Building* buildingBackwardOnConstruction = GetBuildingFromCoords( coords - DirectionUnitIntVec[(int)m_curPreBuildingDir] );
		if (((Conveyer*)buildingOnTile)->m_dir == GetInversedDir( m_curPreBuildingDir ) || buildingBackward == nullptr 
			|| buildingBackwardOnConstruction == nullptr || (buildingBackwardOnConstruction->m_buildingType == BuildingType::Conveyer && ((Conveyer*)buildingBackwardOnConstruction)->m_dir != m_curPreBuildingDir )) {
			RemoveBuilding( coords );
		}
		else {
			RemoveBuilding( coords );
			BuildNewJunction( coords );
			return;
		}
	}
	Conveyer* newConveyer = new Conveyer( coords );
	newConveyer->m_dir = m_curPreBuildingDir;
	Building* testConveyor = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)m_curPreBuildingDir] );
	if (testConveyor && testConveyor->m_buildingType == BuildingType::Conveyer && ((Conveyer*)testConveyor)->m_dir != GetInversedDir(m_curPreBuildingDir)) {
		newConveyer->m_next = (Conveyer*)testConveyor;
		if (((Conveyer*)testConveyor)->m_dir == m_curPreBuildingDir) {
			((Conveyer*)testConveyor)->m_rear = newConveyer;
		}
		else if (((Conveyer*)testConveyor)->m_dir == GetTurnLeftDir(m_curPreBuildingDir)) {
			((Conveyer*)testConveyor)->m_left = newConveyer;
		}
		else if (((Conveyer*)testConveyor)->m_dir == GetTurnRightDir( m_curPreBuildingDir )) {
			((Conveyer*)testConveyor)->m_right = newConveyer;
		}
	}
	testConveyor = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetInversedDir( m_curPreBuildingDir )] );
	if (testConveyor && testConveyor->m_buildingType == BuildingType::Conveyer && ((Conveyer*)testConveyor)->m_dir == m_curPreBuildingDir) {
		newConveyer->m_rear = (Conveyer*)testConveyor;
		((Conveyer*)testConveyor)->m_next = newConveyer;
	}
	testConveyor = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetTurnLeftDir( m_curPreBuildingDir )] );
	if (testConveyor && testConveyor->m_buildingType == BuildingType::Conveyer && ((Conveyer*)testConveyor)->m_dir == GetTurnRightDir( m_curPreBuildingDir )) {
		newConveyer->m_left = (Conveyer*)testConveyor;
		((Conveyer*)testConveyor)->m_next = newConveyer;
	}
	testConveyor = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetTurnRightDir( m_curPreBuildingDir )] );
	if (testConveyor && testConveyor->m_buildingType == BuildingType::Conveyer && ((Conveyer*)testConveyor)->m_dir == GetTurnLeftDir( m_curPreBuildingDir )) {
		newConveyer->m_right = (Conveyer*)testConveyor;
		((Conveyer*)testConveyor)->m_next = newConveyer;
	}
	m_buildings[coords] = newConveyer;
	m_buildingList.push_back( newConveyer );
	AddEntityToList( newConveyer );
}

void Map::BuildNewDrill( IntVec2 coords )
{
	if (CheckIfDrillCanBeBuilt( coords )) {
		Drill* newDrill = new Drill( coords );
 		newDrill->m_dir = m_curPreBuildingDir;

		TileDefinition const& tileDef = GetTileFromCoords( coords )->GetDef();
		if (tileDef.m_tileType == TileType::Iron){
			newDrill->m_drillingType = ProductDefinition::GetDefinition( "iron ore", false ).m_id;
		}
		else if (tileDef.m_tileType == TileType::Coal) {
			newDrill->m_drillingType = ProductDefinition::GetDefinition( "coal", false ).m_id;
		}
		else if (tileDef.m_tileType == TileType::Catalyst) {
			newDrill->m_drillingType = ProductDefinition::GetDefinition( "catalyst", false ).m_id;
		}
		else if (tileDef.m_tileType == TileType::Copper) {
			newDrill->m_drillingType = ProductDefinition::GetDefinition( "copper ore", false ).m_id;
		}
		else if (tileDef.m_tileType == TileType::Stone) {
			newDrill->m_drillingType = ProductDefinition::GetDefinition( "stone ore", false ).m_id;
		}

// 		Building* buildingProbe = nullptr;
// 		buildingProbe = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)m_curPreBuildingDir] );
// 		if (buildingProbe->m_buildingType == BuildingType::Conveyer) {
// 
// 		}
// 		newConveyer->m_rear = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetInversedDir( m_curPreBuildingDir )] );
// 		newConveyer->m_left = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetTurnLeftDir( m_curPreBuildingDir )] );
// 		newConveyer->m_right = GetBuildingFromCoords( coords + DirectionUnitIntVec[(int)GetTurnRightDir( m_curPreBuildingDir )] );
		m_buildings[coords] = newDrill;
		m_buildingList.push_back( newDrill );
		AddEntityToList( newDrill );
	}
}

void Map::BuildNewSelector( IntVec2 coords )
{
	if (CheckIfSelectorCanBeBuilt( coords )) {
		Selector* newSelector = new Selector( coords );
		newSelector->m_dir = m_curPreBuildingDir;

		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
			for (auto resource : m_resources) {
				if (resource && resource->m_conveyerOn == buildingOnTile) {
					newSelector->AddResource( resource );
				}
			}
			RemoveBuilding( coords );
		}

		newSelector->m_selectingTypeID = 0;
		m_buildings[coords] = newSelector;
		m_buildingList.push_back( newSelector );
		AddEntityToList( newSelector );
	}
}

void Map::BuildNewRouter( IntVec2 coords )
{
	if (CheckIfRouterCanBeBuilt( coords )) {
		Router* newRouter = new Router( coords );
		newRouter->m_dir = m_curPreBuildingDir;

		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
			for (auto resource : m_resources) {
				if (resource && resource->m_conveyerOn == buildingOnTile) {
					newRouter->AddResource( resource );
				}
			}
			RemoveBuilding( coords );
		}

		m_buildings[coords] = newRouter;
		m_buildingList.push_back( newRouter );
		AddEntityToList( newRouter );
	}
}

void Map::BuildNewOverflowGate( IntVec2 coords )
{
	if (CheckIfOverflowGateCanBeBuilt( coords )) {
		OverflowGate* newOverflowGate = new OverflowGate( coords );
		newOverflowGate->m_dir = m_curPreBuildingDir;

		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
			for (auto resource : m_resources) {
				if (resource && resource->m_conveyerOn == buildingOnTile) {
					newOverflowGate->AddResource( resource );
				}
			}
			RemoveBuilding( coords );
		}

		m_buildings[coords] = newOverflowGate;
		m_buildingList.push_back( newOverflowGate );
		AddEntityToList( newOverflowGate );
	}
}

void Map::BuildNewWareHouse( IntVec2 coords )
{
	if (CheckIfWareHouseCanBeBuilt( coords )) {
		WareHouse* newWareHouse = new WareHouse( coords );
		m_buildings[coords] = newWareHouse;
		m_buildingList.push_back( newWareHouse );
		AddEntityToList( newWareHouse );
	}
}

void Map::BuildNewExporter( IntVec2 coords )
{
	if (CheckIfExporterCanBeBuilt( coords )) {
		Exporter* newExporter = new Exporter( coords );
		newExporter->m_dir = m_curPreBuildingDir;

		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
			for (auto resource : m_resources) {
				if (resource && resource->m_conveyerOn == buildingOnTile) {
					newExporter->AddResource( resource );
				}
			}
			RemoveBuilding( coords );
		}

		m_buildings[coords] = newExporter;
		m_buildingList.push_back( newExporter );
		AddEntityToList( newExporter );
	}
}

void Map::BuildNewJunction( IntVec2 coords )
{
	if (CheckIfJunctionCanBeBuilt( coords )) {
		Junction* newJunction = new Junction( coords );
		newJunction->m_dir = m_curPreBuildingDir;

		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
			for (auto resource : m_resources) {
				if (resource && resource->m_conveyerOn == buildingOnTile) {
					newJunction->AddResource( resource );
				}
			}
			RemoveBuilding( coords );
		}

		m_buildings[coords] = newJunction;
		m_buildingList.push_back( newJunction );
		AddEntityToList( newJunction );
	}
}

void Map::BuildNewBridge( IntVec2 coords )
{
	if (CheckIfBridgeCanBeBuilt( coords )) {
		Bridge* newBridge = new Bridge( coords );
		newBridge->m_dir = m_curPreBuildingDir;

		Building* buildingOnTile = GetBuildingFromCoords( coords );
		if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
			for (auto resource : m_resources) {
				if (resource && resource->m_conveyerOn == buildingOnTile) {
					newBridge->AddResource( resource );
				}
			}
			RemoveBuilding( coords );
		}

		m_buildings[coords] = newBridge;
		m_buildingList.push_back( newBridge );
		AddEntityToList( newBridge );
		if (m_lastBuiltBridge == nullptr) { // first build bridge
			newBridge->m_isInput = true;
			newBridge->m_canProduce = false;
			m_lastBuiltBridge = newBridge;
			m_curPreBuildingDir = GetInversedDir( m_curPreBuildingDir );
		}
		else {
			newBridge->m_theOtherHead = m_lastBuiltBridge;
			newBridge->m_bridgeLength = GetTaxicabDistance2D( coords, m_lastBuiltBridge->m_leftBottomCoords ) + 1;
			newBridge->m_isInput = false;
			newBridge->m_canProduce = true;
			m_lastBuiltBridge->m_theOtherHead = newBridge;
			m_lastBuiltBridge->m_bridgeLength = newBridge->m_bridgeLength;
			m_lastBuiltBridge = nullptr;
		}
	}
}

void Map::BuildNewPowerPlant( IntVec2 coords )
{
	if (CheckIfPowerPlantCanBeBuilt( coords )) {
		PowerPlant* newPowerPlant = new PowerPlant( coords );
		BuildNewPowerBuilding( newPowerPlant );
		m_buildings[coords] = newPowerPlant;
		m_buildingList.push_back( newPowerPlant );
		AddEntityToList( newPowerPlant );
	}
}

void Map::BuildNewPowerNode( IntVec2 coords )
{
	if (CheckIfPowerNodeCanBeBuilt( coords )) {
		PowerNode* newPowerNode = new PowerNode( coords );
		BuildNewPowerBuilding( newPowerNode );
		m_buildings[coords] = newPowerNode;
		m_buildingList.push_back( newPowerNode );
		AddEntityToList( newPowerNode );
	}
}

void Map::BuildNewPowerBuilding( PowerBuilding* building )
{
	PowerNetwork* netWork = nullptr;
	for (auto buildingIter: m_buildingList) {
		if (buildingIter == (void*)building) {
			continue;
		}
		PowerBuilding* adjProvider = dynamic_cast<PowerBuilding*>(buildingIter);
		if (adjProvider) {
			float range = std::max( adjProvider->m_powerRange, building->m_powerRange );
			if (GetDistanceSquared2D( adjProvider->GetCenterPos(), building->GetCenterPos()) <= range * range) {
				// add connection
				adjProvider->m_otherPowerBuildingInRange.push_back( building );
				building->m_otherPowerBuildingInRange.push_back( adjProvider );
				if (netWork == nullptr) { // first find a network
					netWork = adjProvider->m_netWork;
					netWork->m_buildingInNetWork.push_back( building );
					building->m_netWork = netWork;
				}
				else if (netWork != adjProvider->m_netWork) { // connect two different net works
					PowerNetwork* networkToRemove = adjProvider->m_netWork;
					netWork->JoinNetwork( networkToRemove );
					RemoveNetwork( networkToRemove );
				}
			}
		}
	}

	// do not find any provider near by, create a new network
	if (netWork == nullptr) {
		PowerNetwork* newNetwork = new PowerNetwork();
		newNetwork->m_buildingInNetWork.push_back( building );
		building->m_netWork = newNetwork;
		m_powerNetworks.push_back( newNetwork );
	}
}

void Map::BuildNewBlender( IntVec2 coords )
{
	if (CheckIfBlenderCanBeBuilt( coords )) {
		Blender* newBlender = new Blender( coords );
		newBlender->m_dir = m_curPreBuildingDir;
		m_buildingList.push_back( newBlender );
		m_buildings[coords] = newBlender;
		m_buildings[coords + IntVec2( 0, 1 )] = newBlender;
		m_buildings[coords + IntVec2( 1, 0 )] = newBlender;
		m_buildings[coords + IntVec2( 1, 1 )] = newBlender;
		BuildNewPowerBuilding( newBlender );
		AddEntityToList( newBlender );
	}
}

void Map::BuildNewRefinery( IntVec2 coords )
{
	if (CheckIfRefineryCanBeBuilt( coords )) {
		Refinery* newRefinery = new Refinery( coords );
		newRefinery->m_dir = m_curPreBuildingDir;
		m_buildingList.push_back( newRefinery );
		m_buildings[coords] = newRefinery;
		m_buildings[coords + IntVec2( 0, 1 )] = newRefinery;
		m_buildings[coords + IntVec2( 1, 0 )] = newRefinery;
		m_buildings[coords + IntVec2( 1, 1 )] = newRefinery;
		BuildNewPowerBuilding( newRefinery );
		AddEntityToList( newRefinery );
	}
}

void Map::BuildNewTower( IntVec2 coords )
{
	TowerBase* newTower = nullptr;
	if (m_curPreBuilding == BuildingType::Mortar || m_curPreBuilding == BuildingType::GuidedMissile) {
		if (m_curPreBuilding == BuildingType::Mortar) {
			newTower = new Mortar( coords );
		}
		else if (m_curPreBuilding == BuildingType::GuidedMissile) {
			newTower = new GuidedMissile( coords );
		}
		newTower->m_dir = m_curPreBuildingDir;
		m_buildingList.push_back( newTower );
		m_buildings[coords] = newTower;
		m_buildings[coords + IntVec2( 0, 1 )] = newTower;
		m_buildings[coords + IntVec2( 1, 0 )] = newTower;
		m_buildings[coords + IntVec2( 1, 1 )] = newTower;
		BuildNewPowerBuilding( newTower );
		AddEntityToList( newTower );
	}
	else if (m_curPreBuilding == BuildingType::Wall) {
		Wall* newWall = new Wall( coords );
		newWall->m_dir = m_curPreBuildingDir;
		m_buildings[coords] = newWall;
		m_buildingList.push_back( newWall );
		AddEntityToList( newWall );
	}
	else {
		if (m_curPreBuilding == BuildingType::StraightArcher) {
			newTower = new StraightArcher( coords );
		}
		else if (m_curPreBuilding == BuildingType::ThreeDirectionsPike) {
			newTower = new ThreeDirectionsPike( coords );
		}
		else if (m_curPreBuilding == BuildingType::Laser) {
			newTower = new Laser( coords );
		}
		newTower->m_dir = m_curPreBuildingDir;
		BuildNewPowerBuilding( newTower );
		m_buildings[coords] = newTower;
		m_buildingList.push_back( newTower );
		AddEntityToList( newTower );
	}
}

void Map::RemoveNetwork( PowerNetwork* network )
{
	if (std::find( m_powerNetworks.begin(), m_powerNetworks.end(), network ) != m_powerNetworks.end()) {
		delete network;
		m_powerNetworks.erase( std::remove( m_powerNetworks.begin(), m_powerNetworks.end(), network ) );
	}
}

void Map::RemoveBuilding( IntVec2 coords, bool collectResource )
{
	Building* buildingToRemove = GetBuildingFromCoords( coords );
	if (!buildingToRemove || buildingToRemove->m_buildingType == BuildingType::Base) {
		return;
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Conveyer) {
		Conveyer* conveyer = (Conveyer*)buildingToRemove;
		if (conveyer->m_next && conveyer->m_next->m_buildingType == BuildingType::Conveyer) {
			if (((Conveyer*)conveyer->m_next)->m_rear == conveyer) {
				((Conveyer*)conveyer->m_next)->m_rear = nullptr;
			}
			else if (((Conveyer*)conveyer->m_next)->m_left == conveyer) {
				((Conveyer*)conveyer->m_next)->m_left = nullptr;
			}
			else if (((Conveyer*)conveyer->m_next)->m_right == conveyer) {
				((Conveyer*)conveyer->m_next)->m_right = nullptr;
			}
		}
		if (conveyer->m_left && conveyer->m_left->m_buildingType == BuildingType::Conveyer) {
			((Conveyer*)conveyer->m_left)->m_next = nullptr;
		}
		if (conveyer->m_right && conveyer->m_right->m_buildingType == BuildingType::Conveyer) {
			((Conveyer*)conveyer->m_right)->m_next = nullptr;
		}
		if (conveyer->m_rear && conveyer->m_rear->m_buildingType == BuildingType::Conveyer) {
			((Conveyer*)conveyer->m_rear)->m_next = nullptr;
		}
		if (collectResource) {
			for (auto& resource : m_resources) {
				if (resource && resource->m_conveyerOn == conveyer) {
					RemoveResourceAndAddToAmount( resource );
					resource = nullptr;
				}
			}
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Selector) {
		if (collectResource) {
			for (auto& resource : ((Selector*)buildingToRemove)->m_resources) {
				resource.m_resource->m_isGarbage = true;
				resource.m_resource->m_isCollected = true;
			}
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Router) {
		if (collectResource) {
			for (auto& resource : ((Router*)buildingToRemove)->m_resources) {
				resource.m_resource->m_isGarbage = true;
				resource.m_resource->m_isCollected = true;
			}
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::OverflowGate) {
		if (collectResource) {
			for (auto& resource : ((OverflowGate*)buildingToRemove)->m_resources) {
				resource.m_resource->m_isGarbage = true;
				resource.m_resource->m_isCollected = true;
			}
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Exporter) {
		if (collectResource) {
			for (auto& resource : ((Exporter*)buildingToRemove)->m_resources) {
				resource.m_resource->m_isGarbage = true;
				resource.m_resource->m_isCollected = true;
			}
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Junction) {
		if (collectResource) {
			for (auto& resource : ((Junction*)buildingToRemove)->m_resourcesForwardBack) {
				resource.m_resource->m_isGarbage = true;
				resource.m_resource->m_isCollected = true;
			}
			for (auto& resource : ((Junction*)buildingToRemove)->m_resourcesLeftRight) {
				resource.m_resource->m_isGarbage = true;
				resource.m_resource->m_isCollected = true;
			}
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Bridge) {
		if (collectResource) {
			if (((Bridge*)buildingToRemove)->m_isInput) {
				for (auto& resource : ((Bridge*)buildingToRemove)->m_resources) {
					resource.m_resource->m_isGarbage = true;
					resource.m_resource->m_isCollected = true;
				}
			}
		}
		if (((Bridge*)buildingToRemove)->m_theOtherHead) {
			((Bridge*)buildingToRemove)->m_theOtherHead->m_theOtherHead = nullptr;
			((Bridge*)buildingToRemove)->m_theOtherHead = nullptr;
		}
	}
	else if (buildingToRemove->m_buildingType == BuildingType::PowerPlant) {
		if (collectResource) {
			m_numOfResource[ProductDefinition::GetDefinition( "coal", false ).m_id] += ((PowerPlant*)buildingToRemove)->m_resourceCount;
		}
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::PowerNode) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Blender) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 0, 1 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords  + IntVec2( 1, 0 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords  + IntVec2( 1, 1 ) ) );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Refinery) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords  + IntVec2( 0, 1 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords  + IntVec2( 1, 0 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords  + IntVec2( 1, 1 ) ) );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Mortar) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 0, 1 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 1, 0 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 1, 1 ) ) );
		}
	else if (buildingToRemove->m_buildingType == BuildingType::GuidedMissile) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 0, 1 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 1, 0 ) ) );
		m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords + IntVec2( 1, 1 ) ) );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::StraightArcher) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::Laser) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
	}
	else if (buildingToRemove->m_buildingType == BuildingType::ThreeDirectionsPike) {
		RemovePowerBuilding( (PowerBuilding*)buildingToRemove );
	}
	RemoveEntityFromList( buildingToRemove );
	m_buildings.erase( m_buildings.find( buildingToRemove->m_leftBottomCoords ) );
	delete buildingToRemove;
	m_buildingList.erase( std::remove(m_buildingList.begin(), m_buildingList.end(), buildingToRemove), m_buildingList.end() );
}

void Map::RemoveResource( Resource* resourceToRemove )
{
	delete resourceToRemove;
}

void Map::RemoveResourceAndAddToAmount( Resource* resourceToRemove )
{
	m_numOfResource[int( resourceToRemove->m_def.m_id )]++;
	RemoveResource( resourceToRemove );
}

void Map::RemoveAllGarbageResource()
{
	for (auto& resource : m_resources) {
		if (resource && resource->m_isGarbage) {
			if (resource->m_isCollected) {
				RemoveResourceAndAddToAmount( resource );
			}
			else {
				RemoveResource( resource );
			}
			resource = nullptr;
		}
	}

	for (int i = 0; i < NumOfProductTypes; ++i) {
		m_numOfResource[i] = GetClamped( m_numOfResource[i], 0, 9999 );
	}
}

void Map::RemoveAllGarbageEntity()
{
	for (size_t i = 0; i < m_entities.size(); ++i) {
		if (m_entities[i] && m_entities[i]->m_isGarbage) {
			if (m_entities[i]->m_entityType == EntityType::Building) {
				RemoveBuilding( ((Building*)m_entities[i])->m_leftBottomCoords, false );
				m_entities[i] = nullptr;
			}
			else {
				delete m_entities[i];
				m_entities[i] = nullptr;
			}
		}
	}
}

void Map::RemovePowerBuilding( PowerBuilding* building )
{
	for (auto providerToRemove : building->m_otherPowerBuildingInRange) {
		providerToRemove->RemoveInRangeProvider( building );
	}
	building->m_netWork->RemoveProvider( building );
	if (building->m_netWork->IsEmpty()) {
		RemoveNetwork( building->m_netWork );
	}
	else {
		std::vector<PowerNetwork*> networks;
		if (building->m_netWork->SeparateNetwork( networks )) {
			for (auto network : networks) {
				m_powerNetworks.push_back( network );
			}
		}
	}
}

bool Map::CreateNewResource( int resourceID, Vec2 const& pos, Building* requestBuilding )
{
	Building* buildingOnTile = GetBuildingFromCoords( GetTileCoordsFromPos( pos ) );
	// #ToDo: Can be generated in buildings in addition to conveyers
	if (buildingOnTile && buildingOnTile->m_buildingType == BuildingType::Conveyer) {
		bool canSpawn = true;
		if (GetBuildingFromCoords( GetTileCoordsFromPos( buildingOnTile->GetCenterPos() + DirectionUnitVec[(int)((Conveyer*)buildingOnTile)->m_dir] ) ) == requestBuilding) {
			return false;
		}
		for (auto resource : m_resources) {
			if (resource && DoDiscsOverlap( resource->m_centerPos, RESOURCE_RADIUS, pos, RESOURCE_RADIUS )) {
				canSpawn = false;
			}
		}
		if (canSpawn) {
			Resource* newResource = new Resource(ProductDefinition::GetDefinition(resourceID));
			newResource->m_conveyerOn = (Conveyer*)buildingOnTile;
			newResource->m_centerPos = pos;
			AddResourceToList( newResource );
			return true;
		}
	}
	else if (buildingOnTile) {
		if (buildingOnTile->m_buildingType != BuildingType::Junction && buildingOnTile->AddResource( resourceID )) {
			return true;
		}
		else if (buildingOnTile->m_buildingType == BuildingType::Junction) {
			Vec2 displacement = buildingOnTile->GetCenterPos() - requestBuilding->GetCenterPos();
			if (abs( DotProduct2D( displacement, DirectionUnitVec[(int)((Junction*)buildingOnTile)->m_dir] ) ) > abs(DotProduct2D( displacement, DirectionUnitVec[(int)GetTurnLeftDir( ((Junction*)buildingOnTile)->m_dir)] ))) {
				if ((int)((Junction*)buildingOnTile)->m_resourcesForwardBack.size() >= LogisticBuildingMaxCapacity) {
					return false;
				}
				else {
					Resource* newResource = new Resource(ProductDefinition::GetDefinition(resourceID));
					newResource->m_centerPos = pos;
					if (((Junction*)buildingOnTile)->AddResource( newResource )) {
						AddResourceToList( newResource );
						return true;
					}
					else {
						delete newResource;
						return false;
					}
				}
			}
			else {
				if ((int)((Junction*)buildingOnTile)->m_resourcesLeftRight.size() >= LogisticBuildingMaxCapacity) {
					return false;
				}
				else {
					Resource* newResource = new Resource(ProductDefinition::GetDefinition(resourceID));
					newResource->m_centerPos = pos;
					if (((Junction*)buildingOnTile)->AddResource( newResource )) {
						AddResourceToList( newResource );
						return true;
					}
					else {
						delete newResource;
						return false;
					}
				}
			}
		}
	}
	return false;
}

void Map::AddResourceToList( Resource* resource )
{
	for (int i = 0; i < (int)m_resources.size(); ++i) {
		if (m_resources[i] == nullptr) {
			m_resources[i] = resource;
			return;
		}
	}
	m_resources.push_back( resource );
}

Projectile* Map::CreateNewProjectile( Vec2 const& pos, float orientationDegrees, ProjectileDefinition const& def )
{
	Projectile* proj = new Projectile( pos, def );
	proj->m_orientationDegrees = orientationDegrees;
	AddEntityToList( proj );
	return proj;
}

EnemyBase* Map::SpawnNewEnemy( Vec2 const& pos, float orientationDegrees, EnemyDefinition const& def )
{
	EnemyBase* enemy = new EnemyBase( pos, def );
	enemy->m_orientationDegrees = orientationDegrees;
	AddEntityToList( enemy );
	return enemy;
}

IntVec2 Map::GetTileCoordsFromPos( Vec2 const& pos ) const
{
	return IntVec2( RoundDownToInt( pos.x ), RoundDownToInt( pos.y ) );
}

IntVec2 Map::GetRandomTileCoordsOnMap()
{
	return IntVec2( m_mapRNG.RollRandomIntInRange( 0, m_dimensions.x - 1 ), m_mapRNG.RollRandomIntInRange( 0, m_dimensions.y - 1 ) );
}

IntVec2 Map::GetRandomTileCoordsInRange( IntVec2 const& minInclusive, IntVec2 const& maxInclusive )
{
	return IntVec2( m_mapRNG.RollRandomIntInRange( minInclusive.x, maxInclusive.x ), m_mapRNG.RollRandomIntInRange( minInclusive.y, maxInclusive.y ) );
}

void Map::DoWormRandomTileGeneration( TileDefinition const& tile, IntVec2 const& startCoords, int numOfSteps )
{
	IntVec2 curCoords = startCoords;
	GetTileFromCoords( curCoords )->SetDef( tile );
	for (int j = 0; j < numOfSteps; ++j) {
		int dir = m_mapRNG.RollRandomIntInRange( 0, 3 );
		if (dir == 0) {
			if (IsCoordsInBounds( curCoords + IntVec2( -1, 0 ) )) {
				curCoords += IntVec2( -1, 0 );
				GetTileFromCoords( curCoords )->SetDef( tile );
			}
		}
		else if (dir == 1) {
			if (IsCoordsInBounds( curCoords + IntVec2( 1, 0 ) )) {
				curCoords += IntVec2( 1, 0 );
				GetTileFromCoords( curCoords )->SetDef( tile );
			}
		}
		else if (dir == 2) {
			if (IsCoordsInBounds( curCoords + IntVec2( 0, 1 ) )) {
				curCoords += IntVec2( 0, 1 );
				GetTileFromCoords( curCoords )->SetDef( tile );
			}
		}
		else if (dir == 3) {
			if (IsCoordsInBounds( curCoords + IntVec2( 0, -1 ) )) {
				curCoords += IntVec2( 0, -1 );
				GetTileFromCoords( curCoords )->SetDef( tile );
			}
		}
	}
}

void Map::DoPerfectWormRandomTileGeneration( TileDefinition const& tile, IntVec2 const& startCoords, int numOfSteps )
{
	IntVec2 curCoords = startCoords;
	GetTileFromCoords( curCoords )->SetDef( tile );
	for (int j = 0; j < numOfSteps;) {
		int dir = m_mapRNG.RollRandomIntInRange( 0, 3 );
		if (dir == 0) {
			if (IsCoordsInBounds( curCoords + IntVec2( -1, 0 ) )) {
				curCoords += IntVec2( -1, 0 );
				Tile* thisTile = GetTileFromCoords( curCoords );
				if (thisTile->GetDef().m_tileType != TileType::Wall) {
					thisTile->SetDef( tile );
					++j;
				}
			}
		}
		else if (dir == 1) {
			if (IsCoordsInBounds( curCoords + IntVec2( 1, 0 ) )) {
				curCoords += IntVec2( 1, 0 );
				Tile* thisTile = GetTileFromCoords( curCoords );
				if (thisTile->GetDef().m_tileType != TileType::Wall) {
					thisTile->SetDef( tile );
					++j;
				}
			}
		}
		else if (dir == 2) {
			if (IsCoordsInBounds( curCoords + IntVec2( 0, 1 ) )) {
				curCoords += IntVec2( 0, 1 );
				Tile* thisTile = GetTileFromCoords( curCoords );
				if (thisTile->GetDef().m_tileType != TileType::Wall) {
					thisTile->SetDef( tile );
					++j;
				}
			}
		}
		else if (dir == 3) {
			if (IsCoordsInBounds( curCoords + IntVec2( 0, -1 ) )) {
				curCoords += IntVec2( 0, -1 );
				Tile* thisTile = GetTileFromCoords( curCoords );
				if (thisTile->GetDef().m_tileType != TileType::Wall) {
					thisTile->SetDef( tile );
					++j;
				}
			}
		}
	}
}

bool Map::DoEntitiesHaveInteraction( Entity* a, Entity* b ) const
{
	if (a->m_entityType == EntityType::Enemy && b->m_entityType == EntityType::Projectile) {
		return true;
	}
	return false;
}

void Map::PopulateTileHeatMapDistanceField( TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost, float minCost, bool treatBuildingsAsSolid ) const
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
				&& !IsTileSolid( nextTile ) && !(treatBuildingsAsSolid && IsTileBuilding( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.y >= 1) {
			IntVec2 nextTile = thisTile + IntVec2( 0, -1 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost
				&& !IsTileSolid( nextTile ) && !(treatBuildingsAsSolid && IsTileBuilding( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.x < m_dimensions.x - 1) {
			IntVec2 nextTile = thisTile + IntVec2( 1, 0 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost
				&& !IsTileSolid( nextTile ) && !(treatBuildingsAsSolid && IsTileBuilding( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.y < m_dimensions.y - 1) {
			IntVec2 nextTile = thisTile + IntVec2( 0, 1 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost
				&& !IsTileSolid( nextTile ) && !(treatBuildingsAsSolid && IsTileBuilding( nextTile ))) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		start++;//= (start + 1) % maxQueueLength;
	}
	delete[] queue;
}

void Map::DoConstruction( IntVec2 const& coords )
{
	if (m_curPreBuilding == BuildingType::Conveyer) {
		BuildNewConveyer( coords );
	}
	else if (m_curPreBuilding == BuildingType::Drill) {
		BuildNewDrill( coords );
	}
	else if (m_curPreBuilding == BuildingType::Selector) {
		BuildNewSelector( coords );
	}
	else if (m_curPreBuilding == BuildingType::Router) {
		BuildNewRouter( coords );
	}
	else if (m_curPreBuilding == BuildingType::OverflowGate) {
		BuildNewOverflowGate( coords );
	}
	else if (m_curPreBuilding == BuildingType::WareHouse) {
		BuildNewWareHouse( coords );
	}
	else if (m_curPreBuilding == BuildingType::Exporter) {
		BuildNewExporter( coords );
	}
	else if (m_curPreBuilding == BuildingType::Junction) {
		BuildNewJunction( coords );
	}
	else if (m_curPreBuilding == BuildingType::Bridge) {
		BuildNewBridge( coords );
	}
	else if (m_curPreBuilding == BuildingType::PowerPlant) {
		BuildNewPowerPlant( coords );
	}
	else if (m_curPreBuilding == BuildingType::PowerNode) {
		BuildNewPowerNode( coords );
	}
	else if (m_curPreBuilding == BuildingType::Blender) {
		BuildNewBlender( coords );
	}
	else if (m_curPreBuilding == BuildingType::Refinery) {
		BuildNewRefinery( coords );
	}
	else if (m_curPreBuilding == BuildingType::Wall 
		|| m_curPreBuilding == BuildingType::StraightArcher
		|| m_curPreBuilding == BuildingType::Mortar
		|| m_curPreBuilding == BuildingType::Laser
		|| m_curPreBuilding == BuildingType::ThreeDirectionsPike
		|| m_curPreBuilding == BuildingType::GuidedMissile) {
		BuildNewTower( coords );
	}
}

Vec2 Map::ConvertWorldPosToScreen( Vec2 const& worldPos ) const
{
	return g_theGame->m_screenCamera.m_cameraBox.GetPointAtUV( g_theGame->m_worldCamera.m_cameraBox.GetUVForPoint( worldPos ) );
}

void Map::SetBuildingSettingsUIActive( SFWidget* widget, Vec2 const& center )
{
	widget->m_isActive = true;
	widget->SetCenter( center );
	m_curActiveWidget.push_back( widget );
}

void Map::DeActiveAllUIWidget()
{
	for (auto widget : m_curActiveWidget) {
		widget->m_isActive = false;
	}
	m_curActiveWidget.clear();
}

void Map::SetConstructionPanelActive( bool active )
{
	for (auto widget : g_constructionPanelUI) {
		widget->m_isActive = active;
	}
}

bool Map::UISelectionBuildBuilding( EventArgs& args )
{
	Map* map = GetCurMap();
	if (map->m_UIState == MapUIState::ViewingMap || map->m_UIState == MapUIState::ConstructBuilding) {
		map->m_UIState = MapUIState::ConstructBuilding;
		map->m_curPreBuilding = args.GetValue( "BuildingType", BuildingType::None );
	}
	return true;
}

bool operator<( Vec2 const& a, Vec2 const& b )
{
	if (a.y > b.y) {
		return false;
	}
	else if (a.y < b.y) {
		return true;
	}
	else {
		return a.x < b.x;
	}
}
