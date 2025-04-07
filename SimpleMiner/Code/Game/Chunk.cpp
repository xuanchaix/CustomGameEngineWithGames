#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fileapi.h>

#include "Game/Chunk.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/BlockTemplates.hpp"

Chunk::Chunk( IntVec2 const& coords )
	:m_coords(coords)
{
	m_chunkOriginWorldCoords = IntVec3( coords.x << XBITS, coords.y << YBITS, 0 );
	m_bounds = AABB3( Vec3( (float)m_chunkOriginWorldCoords.x, (float)m_chunkOriginWorldCoords.y, (float)m_chunkOriginWorldCoords.z ),
		Vec3( float( m_chunkOriginWorldCoords.x + XSIZE ), float( m_chunkOriginWorldCoords.y + YSIZE ), float( m_chunkOriginWorldCoords.z + ZSIZE ) ) );
	m_worldCenterXY = Vec2( (float)m_chunkOriginWorldCoords.x + (XSIZE >> 1), (float)m_chunkOriginWorldCoords.y + (YSIZE >> 1) );
	m_blocks = new Block[BLOCK_COUNT_EACH_CHUNK];
}

Chunk::~Chunk()
{
	delete m_vertexBuffer;
	delete[] m_blocks;
}

void Chunk::StartUp()
{
	//double begin = GetCurrentTimeSeconds();
	m_state = ChunkState::ACTIVE;
	for (int y = 0; y < YSIZE; y++) {
		for (int x = 0; x < XSIZE; x++) {
			for (int z = ZSIZE - 1; z >= 0; z--) {
				int index = ::GetBlockIndex( IntVec3( x, y, z ) );
				Block& block = m_blocks[index];
				if (block.IsOpaque()) {
					break;
				}
				else {
					BlockIter thisIter( index, this );
					BlockIter iter = thisIter.GetEastNeighbor();
					if (iter.IsValid() && !iter.GetBlock()->IsOpaque() && !iter.GetBlock()->IsSky()) {
						g_theWorld->MarkLightingDirty( iter );
					}
					iter = thisIter.GetWestNeighbor();
					if (iter.IsValid() && !iter.GetBlock()->IsOpaque() && !iter.GetBlock()->IsSky()) {
						g_theWorld->MarkLightingDirty( iter );
					}
					iter = thisIter.GetNorthNeighbor();
					if (iter.IsValid() && !iter.GetBlock()->IsOpaque() && !iter.GetBlock()->IsSky()) {
						g_theWorld->MarkLightingDirty( iter );
					}
					iter = thisIter.GetSouthNeighbor();
					if (iter.IsValid() && !iter.GetBlock()->IsOpaque() && !iter.GetBlock()->IsSky()) {
						g_theWorld->MarkLightingDirty( iter );
					}
				}

			}
		}
	}
	
	for (int i = 0; i < BLOCK_COUNT_EACH_CHUNK; i++) {
		BlockIter iter( i, this );
		// mark edge blocks light dirty
		if (!m_blocks[i].IsOpaque() &&
			((iter.IsOnEastEdge() && iter.HasEastNeighborChunk())
				|| (iter.IsOnNorthEdge() && iter.HasNorthNeighborChunk())
				|| (iter.IsOnSouthEdge() && iter.HasSouthNeighborChunk())
				|| (iter.IsOnWestEdge() && iter.HasWestNeighborChunk()))) {
			g_theWorld->MarkLightingDirty( iter );
		}
		// mark light sources light dirty
		if (m_blocks[i].GetDefinition().m_lightStrength > 0) {
			g_theWorld->MarkLightingDirty( iter );
		}
	}

	// setup vertex array and buffer
	MarkDirty();
	m_needsToSave = false;

	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );
}

void Chunk::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	BuildVertexArrayAndBuffer();
	m_isDirty = false;	
}

void Chunk::Render() const
{
	if (m_vertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( g_theWorld->m_worldShader );
		g_theRenderer->BindTexture( &g_sprite->GetTexture() );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->SetCustomConstantBuffer( const_cast<ConstantBuffer*&>(g_theWorld->m_worldCBO), (void*)&g_theWorld->m_worldConstans, sizeof( g_theWorld->m_worldConstans ), SIMPLE_MINER_WORLD_CONSTANTS_SLOT );
		g_theRenderer->DrawVertexBuffer( m_vertexBuffer, m_vertexBuffer->GetVertexCount() );
	}
}

struct PerlinWormStartPosInfo {
	PerlinWormStartPosInfo( IntVec2 const& chunkCoords, IntVec3 startBlockLocalCoords ):m_chunkCoords(chunkCoords), m_startBlockLocalCoords(startBlockLocalCoords){}
	IntVec2 m_chunkCoords;
	IntVec3 m_startBlockLocalCoords;
};

void Chunk::GenerateBlocks()
{
	//double begin = GetCurrentTimeSeconds();
	unsigned int seed = Get2dNoiseUint( m_coords.x, m_coords.y, g_terrainSeed );
	RandomNumberGenerator rng( seed );
	unsigned char airType = BlockDefinition::GetDefinitionIndexByName( "air" );
	unsigned char grassType = BlockDefinition::GetDefinitionIndexByName( "grass" );
	unsigned char stoneType = BlockDefinition::GetDefinitionIndexByName( "stone" );
	unsigned char dirtType = BlockDefinition::GetDefinitionIndexByName( "dirt" );
	unsigned char waterType = BlockDefinition::GetDefinitionIndexByName( "water" );
	unsigned char coalType = BlockDefinition::GetDefinitionIndexByName( "coal" );
	unsigned char ironType = BlockDefinition::GetDefinitionIndexByName( "iron" );
	unsigned char goldType = BlockDefinition::GetDefinitionIndexByName( "gold" );
	unsigned char diamondType = BlockDefinition::GetDefinitionIndexByName( "diamond" );
	unsigned char sandType = BlockDefinition::GetDefinitionIndexByName( "sand" );
	unsigned char iceType = BlockDefinition::GetDefinitionIndexByName( "ice" );
	unsigned char beachType = BlockDefinition::GetDefinitionIndexByName( "beach" );
	BlockTemplate const& oakTemp = BlockTemplate::GetBlockTemplate( "oak" );
	BlockTemplate const& spruceTemp = BlockTemplate::GetBlockTemplate( "spruce" );
	BlockTemplate const& cactusTemp = BlockTemplate::GetBlockTemplate( "cactus" );

	constexpr int seaLevel = (int)((unsigned int)ZSIZE >> 1);
	constexpr int riverHeight = 5;
	constexpr int maxMountainHeight = ZSIZE - seaLevel - 20;
	constexpr float maxOceanDepth = 40.f;

	std::vector<int> allTerrainHeightZ;
	allTerrainHeightZ.resize( 1 << XBITS << YBITS );
	// randomly set up blocks
	for (int j = 0; j < YSIZE; j++) {
		for (int i = 0; i < XSIZE; i++) {
			IntVec3 worldPosXY = GetBlockWorldCoords( IntVec3( i, j, 0 ) );

			// calculate the hilliness of the terrain, if the hilliness is high, terrain height is more relevant
			float hilliness = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 2000.f, 3, 0.5f, 2.f, false, g_hillinessSeed );
			hilliness = SmoothStep3( SmoothStep3( hilliness ) );

			int terrainPerlinHeightZ;
			int rawPerlinTerrainHeight = int( (maxMountainHeight + riverHeight) * Absf( Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 150.f, 7, 0.5f, 2.f, g_terrainSeed ) ) );
			if (rawPerlinTerrainHeight < riverHeight) {
				terrainPerlinHeightZ = seaLevel - riverHeight + rawPerlinTerrainHeight;
			}
			else {
				terrainPerlinHeightZ = seaLevel + int( hilliness * (rawPerlinTerrainHeight - riverHeight) );
			}

			// calculate the oceanness to decide the ocean area
			float oceanness = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 500.f, 3, 0.5f, 2.f, false, g_oceannessSeed );
			oceanness = RangeMapClamped( oceanness, 0.6f, 0.8f, 0.f, 1.f );
			oceanness = SmoothStep3( SmoothStep3( oceanness ) );

			// calculate the humidity
			float humidity = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 800.f, 7, 0.5f, 2.f, false, g_humiditySeed );

			constexpr int maxSandDepth = 10;
			int sandHeightFromGround = int( RangeMapClamped( humidity, 0.f, 0.4f, 1.f, 0.f ) * maxSandDepth );

			// calculate the temperature
			float temperature = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 800.f, 7, 0.5f, 2.f, false, g_temperatureSeed );

			constexpr int maxIceDepth = 10;
			int iceHeightFromGround = int( RangeMapClamped( temperature, 0.f, 0.4f, 1.f, 0.f ) * maxIceDepth );

			// terrain height
			int terrainHeightZ = terrainPerlinHeightZ - int( oceanness * maxOceanDepth );
			allTerrainHeightZ[i + (j << XBITS)] = terrainHeightZ;
			int dirtDepth = rng.RollRandomIntInRange( 3, 4 );
			for (int k = 0; k < ZSIZE; k++) {
				int index = GetBlockIndex( IntVec3( i, j, k ) );
				if (k == terrainHeightZ) {
					if (k == seaLevel && humidity < 0.6f) {
						m_blocks[index].SetType( beachType );
					}
					else if (sandHeightFromGround > 0) {
						m_blocks[index].SetType( sandType );
					}
					else {
						m_blocks[index].SetType( grassType );
					}
				}
				else if (k > terrainHeightZ) {
					if (k <= seaLevel) {
						if (iceHeightFromGround > seaLevel - k) {
							m_blocks[index].SetType( iceType );
						}
						else {
							m_blocks[index].SetType( waterType );
						}
					}
					else {
						m_blocks[index].SetType( airType );
					}
				}
				else if (k < terrainHeightZ - dirtDepth) {
					if (sandHeightFromGround > terrainHeightZ - k - dirtDepth) {
						m_blocks[index].SetType( sandType );
					}
					else {
						float rnd = rng.RollRandomFloatZeroToOne();
						if (rnd < 0.05f) {
							m_blocks[index].SetType( coalType );
						}
						else {
							rnd = rng.RollRandomFloatZeroToOne();
							if (rnd < 0.02f) {
								m_blocks[index].SetType( ironType );
							}
							else {
								rnd = rng.RollRandomFloatZeroToOne();
								if (rnd < 0.005f) {
									m_blocks[index].SetType( goldType );
								}
								else {
									rnd = rng.RollRandomFloatZeroToOne();
									if (rnd < 0.001f) {
										m_blocks[index].SetType( diamondType );
									}
									else {
										m_blocks[index].SetType( stoneType );
									}
								}
							}
						}
					}
				}
				else {
					if (sandHeightFromGround > terrainHeightZ - k) {
						m_blocks[index].SetType( sandType );
					}
					else {
						m_blocks[index].SetType( dirtType );
					}
				}
			}
		}
	}

	// generate forests
	for (int y = -2; y < YSIZE + 2; y++) {
		for (int x = -2; x < XSIZE + 2; x++) {
			IntVec3 worldPosXY = GetBlockWorldCoords( IntVec3( x, y, 0 ) );
			int terrainHeightZ;
			if (x >= 0 && x < XSIZE && y >= 0 && y < YSIZE) {
				terrainHeightZ = allTerrainHeightZ[x + (y << XBITS)];
			}
			else {
				// calculate the hilliness of the terrain, if the hilliness is high, terrain height is more relevant
				float hilliness = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 2000.f, 3, 0.5f, 2.f, false, g_hillinessSeed );
				hilliness = SmoothStep3( SmoothStep3( hilliness ) );

				int terrainPerlinHeightZ;
				int rawPerlinTerrainHeight = int( (maxMountainHeight + riverHeight) * Absf( Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 150.f, 7, 0.5f, 2.f, g_terrainSeed ) ) );
				if (rawPerlinTerrainHeight < riverHeight) {
					terrainPerlinHeightZ = seaLevel - riverHeight + rawPerlinTerrainHeight;
				}
				else {
					terrainPerlinHeightZ = seaLevel + int( hilliness * (rawPerlinTerrainHeight - riverHeight) );
				}

				// calculate the oceanness to decide the ocean area
				float oceanness = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 500.f, 3, 0.5f, 2.f, false, g_oceannessSeed );
				oceanness = RangeMapClamped( oceanness, 0.6f, 0.8f, 0.f, 1.f );
				oceanness = SmoothStep3( SmoothStep3( oceanness ) );

				// terrain height
				terrainHeightZ = terrainPerlinHeightZ - int( oceanness * maxOceanDepth );
			}

			constexpr float densityThreshold = 0.6f;
			float forestness = 0.5f + 0.5f * Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 100.f, 7, 0.5f, 2.f, false, g_treeDensitySeed );
			float normalizedThreshold = RangeMapClamped( forestness, densityThreshold, 1.f, 1.f, 0.95f );
			if (terrainHeightZ >= seaLevel && forestness > densityThreshold) {
				float thisNoise = Get2dNoiseZeroToOne( worldPosXY.x, worldPosXY.y, g_treeSeed );
				if (thisNoise > normalizedThreshold && thisNoise == Maxf( thisNoise, Maxf( Get2dNoiseZeroToOne( worldPosXY.x - 1, worldPosXY.y - 1, g_treeSeed ),
					Maxf( Get2dNoiseZeroToOne( worldPosXY.x - 1, worldPosXY.y, g_treeSeed ),
						Maxf( Get2dNoiseZeroToOne( worldPosXY.x - 1, worldPosXY.y + 1, g_treeSeed ),
							Maxf( Get2dNoiseZeroToOne( worldPosXY.x, worldPosXY.y - 1, g_treeSeed ),
								Maxf( Get2dNoiseZeroToOne( worldPosXY.x, worldPosXY.y + 1, g_treeSeed ),
									Maxf( Get2dNoiseZeroToOne( worldPosXY.x + 1, worldPosXY.y - 1, g_treeSeed ),
										Maxf( Get2dNoiseZeroToOne( worldPosXY.x + 1, worldPosXY.y, g_treeSeed ), Get2dNoiseZeroToOne( worldPosXY.x + 1, worldPosXY.y + 1, g_treeSeed ) ) ) ) ) ) ) ) )) {
					if (Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 800.f, 7, 0.5f, 2.f, false, g_humiditySeed ) < -0.28f) {
						AddBlockTemplate( cactusTemp, IntVec3( x, y, terrainHeightZ + 1 ) );
					}
					else if (Compute2dPerlinNoise( float( worldPosXY.x ), float( worldPosXY.y ), 800.f, 7, 0.5f, 2.f, false, g_temperatureSeed ) < -0.2f) {
						AddBlockTemplate( spruceTemp, IntVec3( x, y, terrainHeightZ + 1 ) );
					}
					else {
						AddBlockTemplate( oakTemp, IntVec3( x, y, terrainHeightZ + 1 ) );
					}
				}
			}
		}
	}

	/*
	// like trees, have a worm density and a local max noise
			constexpr float wormDensityThreshold = 0.7f;
			float wormness = 0.5f + 0.5f * Compute2dPerlinNoise( float( chunkCoordsX ), float( chunkCoordsY ), 40.f, 5, 0.5f, 2.f, false, g_wormSeed );
			float normalizedWormThreshold = RangeMapClamped( wormness, wormDensityThreshold, 1.f, 1.f, 0.96f );
	*/

	constexpr float wormSphereBoundsMaxRadius = 500.f;
	constexpr float wormSphereBoundsMaxRadiusSquared = wormSphereBoundsMaxRadius * wormSphereBoundsMaxRadius;
	constexpr int wormMaxXRangeForChunkCoords = (int)(wormSphereBoundsMaxRadius / (float)XSIZE) + 2;
	constexpr int wormMaxYRangeForChunkCoords = (int)(wormSphereBoundsMaxRadius / (float)YSIZE) + 2;
	// generate perlin worm caves
	// assume that there may be only one worm each chunk
	// 1. Get all possible worms in range

	std::vector<PerlinWormStartPosInfo> wormStartPosInfo;

	for (int chunkCoordsY = m_coords.y - wormMaxYRangeForChunkCoords; chunkCoordsY <= m_coords.y + wormMaxYRangeForChunkCoords; chunkCoordsY++) {
		for (int chunkCoordsX = m_coords.x - wormMaxXRangeForChunkCoords; chunkCoordsX <= m_coords.x + wormMaxXRangeForChunkCoords; chunkCoordsX++) {
			float wormNoise = Get2dNoiseZeroToOne( chunkCoordsX, chunkCoordsY, g_wormSeed );
			if (wormNoise > 0.99f) {
				// calculate the start block
				int blockLocalX = RoundDownToInt( Interpolate( 0, XSIZE, Get2dNoiseZeroToOne( chunkCoordsX, chunkCoordsY, g_wormSeed + 1 ) ) );
				int blockLocalY = RoundDownToInt( Interpolate( 0, YSIZE, Get2dNoiseZeroToOne( chunkCoordsX, chunkCoordsY, g_wormSeed + 2 ) ) );
				int blockLocalZ = RoundDownToInt( Interpolate( 0, (float)allTerrainHeightZ[blockLocalX + (blockLocalY << XBITS)], Get2dNoiseZeroToOne( chunkCoordsX, chunkCoordsY, g_wormSeed + 3 ) ) );
				wormStartPosInfo.emplace_back( IntVec2( chunkCoordsX, chunkCoordsY ), IntVec3( blockLocalX, blockLocalY, blockLocalZ ) );
			}
		}
	}

	// 2. simulate every worm
	constexpr float wormStep = 1.f;
	constexpr float wormSphereMinRadius = 1.f;
	constexpr float wormSphereMaxRadius = 4.f;
	for (auto& posInfo : wormStartPosInfo) {
		IntVec3 startBlockWorldIntPos = IntVec3( posInfo.m_chunkCoords.x << XBITS, posInfo.m_chunkCoords.y << YBITS, 0 ) + posInfo.m_startBlockLocalCoords;
		Vec3 startBlockWorldPos = Vec3( startBlockWorldIntPos ) + Vec3( 0.5f, 0.5f, 0.5f );
		//ClearBlocksOverlapSphere( startBlockWorldPos, 2.f );
		unsigned int thisWormSeed = Get2dNoiseUint( posInfo.m_chunkCoords.x, posInfo.m_chunkCoords.y );
		float wormYaw = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( startBlockWorldPos.x, startBlockWorldPos.y, startBlockWorldPos.z, 50.f, 5, 0.5f, 2.f, false, thisWormSeed + 4 ));
		float wormPitch = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( startBlockWorldPos.x, startBlockWorldPos.y, startBlockWorldPos.z, 500.f, 3, 0.5f, 2.f, false, thisWormSeed + 5 ));
		Vec3 lastWormPos = startBlockWorldPos;
		Vec3 curWormPos = startBlockWorldPos + Vec3::MakeFromPolarDegrees( wormYaw, wormPitch, wormStep );
		float wormRadius = Interpolate( wormSphereMinRadius, wormSphereMaxRadius, 0.5f + 0.5f * Compute3dPerlinNoise( startBlockWorldPos.x, startBlockWorldPos.y, startBlockWorldPos.z, 20.f, 3, 0.5f, 2.f, false, thisWormSeed + 6 ) );
		while (GetDistanceSquared3D(startBlockWorldPos, curWormPos) < wormSphereBoundsMaxRadiusSquared) {
			ClearBlocksOverlapCapsule( lastWormPos, curWormPos, wormRadius );
			wormYaw = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( curWormPos.x, curWormPos.y, curWormPos.z, 50.f, 5, 0.5f, 2.f, false, thisWormSeed + 4 ));
			wormPitch = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( curWormPos.x, curWormPos.y, curWormPos.z, 500.f, 3, 0.5f, 2.f, false, thisWormSeed + 5 ));
			lastWormPos = curWormPos;
			curWormPos = curWormPos + Vec3::MakeFromPolarDegrees( wormYaw, wormPitch, wormStep );
			wormRadius = Interpolate( wormSphereMinRadius, wormSphereMaxRadius, 0.5f + 0.5f * Compute3dPerlinNoise( lastWormPos.x, lastWormPos.y, lastWormPos.z, 20.f, 3, 0.5f, 2.f, false, thisWormSeed + 6 ) );
		}
		wormYaw = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise(startBlockWorldPos.x, startBlockWorldPos.y, startBlockWorldPos.z, 50.f, 5, 0.5f, 2.f, false, thisWormSeed + 4));
		wormPitch = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( startBlockWorldPos.x, startBlockWorldPos.y, startBlockWorldPos.z, 500.f, 3, 0.5f, 2.f, false, thisWormSeed + 5 ));
		lastWormPos = startBlockWorldPos;
		curWormPos = startBlockWorldPos - Vec3::MakeFromPolarDegrees( wormYaw, wormPitch, wormStep );
		wormRadius = Interpolate( wormSphereMinRadius, wormSphereMaxRadius, 0.5f + 0.5f * Compute3dPerlinNoise( startBlockWorldPos.x, startBlockWorldPos.y, startBlockWorldPos.z, 20.f, 3, 0.5f, 2.f, false, thisWormSeed + 6 ) );
		while (GetDistanceSquared3D( startBlockWorldPos, curWormPos ) < wormSphereBoundsMaxRadiusSquared) {
			ClearBlocksOverlapCapsule( lastWormPos, curWormPos, wormRadius );
			wormYaw = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( curWormPos.x, curWormPos.y, curWormPos.z, 50.f, 5, 0.5f, 2.f, false, thisWormSeed + 4 ));
			wormPitch = 360.f * (0.5f + 0.5f * Compute3dPerlinNoise( curWormPos.x, curWormPos.y, curWormPos.z, 500.f, 3, 0.5f, 2.f, false, thisWormSeed + 5 ));
			lastWormPos = curWormPos;
			curWormPos = curWormPos - Vec3::MakeFromPolarDegrees( wormYaw, wormPitch, wormStep );
			wormRadius = Interpolate( wormSphereMinRadius, wormSphereMaxRadius, 0.5f + 0.5f * Compute3dPerlinNoise( lastWormPos.x, lastWormPos.y, lastWormPos.z, 20.f, 3, 0.5f, 2.f, false, thisWormSeed + 6 ) );
		}
		
	}

	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );
}

void Chunk::SetSkyLight()
{
	// set sky light
	for (int y = 0; y < YSIZE; y++) {
		for (int x = 0; x < XSIZE; x++) {
			for (int z = ZSIZE - 1; z >= 0; z--) {
				Block& block = m_blocks[::GetBlockIndex( IntVec3( x, y, z ) )];
				if (block.IsOpaque()) {
					break;
				}
				else {
					block.SetSky( true );
				}

			}
		}
	}
}

IntVec3 Chunk::GetBlockWorldCoords( IntVec3 const& localCoords ) const
{
	return m_chunkOriginWorldCoords + localCoords;
}

IntVec3 Chunk::GetBlockLocalCoords( IntVec3 const& worldCoords ) const
{
	return worldCoords - m_chunkOriginWorldCoords;
}

BlockDefinition const& Chunk::GetBlockDefinition( IntVec3 const& localCoords ) const
{
	return m_blocks[GetBlockIndex( localCoords )].GetDefinition();
}

Vec2 Chunk::GetWorldCenterXY() const
{
	return m_worldCenterXY;
}

void Chunk::DigTopBlockOfStack( IntVec2 const& localCoords )
{
	BlockDefinition const& airDef = BlockDefinition::GetDefinitionByName( "air" );
	for (int i = ZSIZE - 1; i >= 0; i--) {
		int index = GetBlockIndex( IntVec3( localCoords.x, localCoords.y, i ) );
		BlockDefinition const& def = m_blocks[index].GetDefinition();
		if (def.m_name != "air") {
			m_blocks[index].SetType( airDef.m_index );
			MarkDirty();
			m_needsToSave = true;
			return;
		}
	}
}

void Chunk::PutTopBlockOfStack( IntVec2 const& localCoords )
{
	BlockDefinition const& cobblestoneDef = BlockDefinition::GetDefinitionByName( "cobblestone" );
	for (int i = ZSIZE - 1; i >= 0; i--) {
		int index = GetBlockIndex( IntVec3( localCoords.x, localCoords.y, i ) );
		BlockDefinition const& def = m_blocks[index].GetDefinition();
		if (def.m_name != "air") {
			if (i == ZSIZE - 1) {
				return;
			}
			index = GetBlockIndex( IntVec3( localCoords.x, localCoords.y, i + 1 ) );
			m_blocks[index].SetType( cobblestoneDef.m_index );
			MarkDirty();
			m_needsToSave = true;
			return;
		}
	}
}

void Chunk::SetBlockType( int blockIndex, unsigned char blockType, bool isPut )
{
	BlockIter thisIter( blockIndex, this );
	if (isPut) {
		m_blocks[blockIndex].SetType( blockType );
		if (m_blocks[blockIndex].IsSky() && m_blocks[blockIndex].IsOpaque()) {
			m_blocks[blockIndex].SetSky( false );
			BlockIter iter = thisIter.GetDownNeighbor();
			while (iter.IsValid() && iter.GetBlock()->IsSky()) {
				iter.GetBlock()->SetSky( false );
				g_theWorld->MarkLightingDirty( iter );
				iter = iter.GetDownNeighbor();
			}
		}
	}

	MarkDirty();
	m_needsToSave = true;
	if (thisIter.IsOnEastEdge() && thisIter.HasEastNeighborChunk()) {
		m_eastNeighbor->MarkDirty();
	}
	if (thisIter.IsOnWestEdge() && thisIter.HasWestNeighborChunk()) {
		m_westNeighbor->MarkDirty();
	}
	if (thisIter.IsOnNorthEdge() && thisIter.HasNorthNeighborChunk()) {
		m_northNeighbor->MarkDirty();
	}
	if (thisIter.IsOnSouthEdge() && thisIter.HasSouthNeighborChunk()) {
		m_southNeighbor->MarkDirty();
	}

	// set light dirty
	if (m_blocks[blockIndex].GetDefinition().m_lightStrength > 0) {
		if (isPut) {
			//m_blocks[blockIndex].SetIndoorLightInfluence( (unsigned char)m_blocks[blockIndex].GetDefinition().m_lightStrength );
		}
		else {
			m_blocks[blockIndex].SetIndoorLightInfluence( (unsigned char)0 );
		}
		m_blocks[blockIndex].SetOutdoorLightInfluence( (unsigned char)0 );
	}
	else {
		m_blocks[blockIndex].SetIndoorLightInfluence( (unsigned char)0 );
		m_blocks[blockIndex].SetOutdoorLightInfluence( (unsigned char)0 );
	}
	g_theWorld->MarkLightingDirty( thisIter );

	if (!isPut) {
		m_blocks[blockIndex].SetType( blockType );
		BlockIter upperBlock = thisIter.GetUpNeighbor();
		if (upperBlock.IsValid() && upperBlock.GetBlock()->IsSky()) {
			m_blocks[blockIndex].SetSky( true );
			BlockIter iter = thisIter.GetDownNeighbor();
			while (iter.IsValid() && !iter.GetBlock()->IsOpaque()) {
				iter.GetBlock()->SetSky( true );
				g_theWorld->MarkLightingDirty( iter );
				iter = iter.GetDownNeighbor();
			}
		}
	}
}

int Chunk::GetBlockIndex( IntVec3 const& localCoords ) const
{
	return ::GetBlockIndex( localCoords );
	//return (((localCoords.z << YBITS) | localCoords.y) << XBITS) | localCoords.x;
	//return localCoords.x | (localCoords.y << XBITS) | (localCoords.z << ZBITS_SHIFT);
}

IntVec3 Chunk::GetBlockLocalCoordsByIndex( int index ) const
{
	return ::GetBlockLocalCoordsByIndex( index );
	//return IntVec3( index & XBITS_MASK, (int)((unsigned int)(index & YBITS_MASK) >> XBITS), (int)((unsigned int)index >> ZBITS_SHIFT) );
}

void Chunk::AddBlockTemplate( BlockTemplate const& temp, IntVec3 const& localOriginPos )
{
	for (auto& entry : temp.m_entries) {
		IntVec3 localPos = localOriginPos + entry.m_relativePos;
		if (IsCoordsInBounds( localPos )) {
			int blockIndex = GetBlockIndex( localPos );
			Block& block = m_blocks[blockIndex];
			if (block.GetDefinition().m_index == g_theWorld->m_airDef.m_index) {
				block.SetType( entry.m_type );
				//SetBlockType( blockIndex, entry.m_type, true );
			}
		}
	}
}

bool Chunk::IsCoordsInBounds( IntVec3 const& coords ) const
{
	return coords.x >= 0 && coords.x < XSIZE && coords.y >= 0 && coords.y < YSIZE && coords.z >= 0 && coords.z < ZSIZE;
}

void Chunk::BuildVertexArrayAndBuffer()
{
	m_verts.reserve( 100000 );
	m_verts.clear();
	for (int i = 0; i < BLOCK_COUNT_EACH_CHUNK; i++) {
		BlockDefinition const& def = m_blocks[i].GetDefinition();
		if (def.m_visible) {
			Vec3 worldPos = GetBlockWorldCoords( GetBlockLocalCoordsByIndex( i ) );
			IntVec3 coords = GetBlockLocalCoordsByIndex( i );
			BlockIter iter( i, this );
			// sides
			// south
			BlockIter southBlock = iter.GetSouthNeighbor();
			if (southBlock.IsValid() && !southBlock.GetBlock()->IsOpaque()) {
				AddVertsForQuad3D( m_verts, worldPos, worldPos + Vec3( 1.f, 0.f, 0.f ), 
					worldPos + Vec3( 1.f, 0.f, 1.f ), worldPos + Vec3( 0.f, 0.f, 1.f ),
					CalculateLightColorForBlock( southBlock ), def.m_sideUV );
			}
			// west
			BlockIter westBlock = iter.GetWestNeighbor();
			if (westBlock.IsValid() && !westBlock.GetBlock()->IsOpaque()) {
				AddVertsForQuad3D( m_verts, worldPos + Vec3( 0.f, 1.f, 0.f ), 
					worldPos, worldPos + Vec3( 0.f, 0.f, 1.f ), 
					worldPos + Vec3( 0.f, 1.f, 1.f ), CalculateLightColorForBlock( westBlock ), def.m_sideUV );
			}
			// north
			BlockIter northBlock = iter.GetNorthNeighbor();
			if (northBlock.IsValid() && !northBlock.GetBlock()->IsOpaque()) {
				AddVertsForQuad3D( m_verts, worldPos + Vec3( 1.f, 1.f, 0.f ),
					worldPos + Vec3( 0.f, 1.f, 0.f ), worldPos + Vec3( 0.f, 1.f, 1.f ), 
					worldPos + Vec3( 1.f, 1.f, 1.f ), CalculateLightColorForBlock( northBlock ), def.m_sideUV );
			}
			// east
			BlockIter eastBlock = iter.GetEastNeighbor();
			if (eastBlock.IsValid() && !eastBlock.GetBlock()->IsOpaque()) {
				AddVertsForQuad3D( m_verts, worldPos + Vec3( 1.f, 0.f, 0.f ), 
					worldPos + Vec3( 1.f, 1.f, 0.f ), worldPos + Vec3( 1.f, 1.f, 1.f ), 
					worldPos + Vec3( 1.f, 0.f, 1.f ), CalculateLightColorForBlock( eastBlock ), def.m_sideUV );
			}
			// top
			BlockIter topBlock = iter.GetUpNeighbor();
			if (topBlock.IsValid() && !topBlock.GetBlock()->IsOpaque()) {
				AddVertsForQuad3D( m_verts, worldPos + Vec3( 0.f, 0.f, 1.f ), 
					worldPos + Vec3( 1.f, 0.f, 1.f ), worldPos + Vec3( 1.f, 1.f, 1.f ), 
					worldPos + Vec3( 0.f, 1.f, 1.f ), CalculateLightColorForBlock( topBlock ), def.m_topUV );
			}
			// bottom
			BlockIter bottomBlock = iter.GetDownNeighbor();
			if (bottomBlock.IsValid() && !bottomBlock.GetBlock()->IsOpaque()) {
				AddVertsForQuad3D( m_verts, worldPos + Vec3( 0.f, 1.f, 0.f ), 
					worldPos + Vec3( 1.f, 1.f, 0.f ), worldPos + Vec3( 1.f, 0.f, 0.f ), 
					worldPos, CalculateLightColorForBlock( bottomBlock ), def.m_bottomUV );
			}
		}
	}

	delete m_vertexBuffer;
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( m_verts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( m_verts.data(), m_verts.size() * sizeof( Vertex_PCU ), m_vertexBuffer );
}

bool Chunk::IsBlockSurfaceHidden( IntVec3 const& coords, IntVec3 const& step ) const
{
	if (!g_hiddenSurfaceRemovalEnable) {
		return false;
	}
	IntVec3 nextToCoords = coords + step;
	// out of bounds - draw it
	if (nextToCoords.x < 0 || nextToCoords.x >= XSIZE || nextToCoords.y < 0 || nextToCoords.y >= YSIZE || nextToCoords.z < 0 || nextToCoords.z >= ZSIZE) {
		return false;
	}
	// bottom - do not draw it
	if (nextToCoords.z == 0) {
		return true;
	}
	BlockDefinition const& def = GetBlockDefinition( nextToCoords );
	// not air - do not draw it
	if (def.m_visible) {
		return true;
	}
	return false;
}

Rgba8 Chunk::CalculateLightColorForBlock( BlockIter const& blockIter ) const
{
	unsigned char indoorLightIntensity = 0;
	unsigned char outdoorLightIntensity = 0;
	indoorLightIntensity = blockIter.GetBlock()->GetIndoorLightInfluence();
	indoorLightIntensity = indoorLightIntensity * 17;
	outdoorLightIntensity = blockIter.GetBlock()->GetOutdoorLightInfluence();
	outdoorLightIntensity = outdoorLightIntensity * 17;
	return Rgba8( outdoorLightIntensity, indoorLightIntensity, 127 );
}

void Chunk::ClearBlocksOverlapSphere( Vec3 const& sphereCenter, float sphereRadius )
{
	// reject test
	if (DoSphereAndAABB3Overlap3D( sphereCenter, sphereRadius, m_bounds )) {
		unsigned char airType = BlockDefinition::GetDefinitionIndexByName( "air" );
		float squaredRadius = sphereRadius * sphereRadius;
		for (int j = 0; j < YSIZE; j++) {
			for (int i = 0; i < XSIZE; i++) {
				for (int k = 0; k < ZSIZE; k++) {
					if (GetDistanceSquared3D( Vec3( GetBlockWorldCoords(IntVec3(i, j, k))) + Vec3(0.5f, 0.5f, 0.5f), sphereCenter) <= squaredRadius) {
						m_blocks[GetBlockIndex( IntVec3( i, j, k ) )].SetType( airType );
					}
				}
			}
		}
	}
}

void Chunk::ClearBlocksOverlapCapsule( Vec3 const& capsuleStart, Vec3 const& capsuleEnd, float capsuleRadius )
{
	// reject test
	if (DoSphereAndAABB3Overlap3D( (capsuleStart + capsuleEnd) * 0.5f, Maxf((capsuleEnd - capsuleStart).GetLength() * 0.5f,capsuleRadius), m_bounds)) {
		unsigned char airType = BlockDefinition::GetDefinitionIndexByName( "air" );
		float squaredRadius = capsuleRadius * capsuleRadius;
		for (int j = 0; j < YSIZE; j++) {
			for (int i = 0; i < XSIZE; i++) {
				for (int k = 0; k < ZSIZE; k++) {
					if (GetPointDistanceToLineSegmentSquared3D( Vec3( GetBlockWorldCoords( IntVec3( i, j, k ) ) ) + Vec3( 0.5f, 0.5f, 0.5f ), capsuleStart, capsuleEnd ) <= squaredRadius) {
						m_blocks[GetBlockIndex( IntVec3( i, j, k ) )].SetType( airType );
					}
				}
			}
		}
	}
}

void Chunk::SaveToFile() const
{
	if (!g_saveModifiedChunks) {
		return;
	}
	std::vector<uint8_t> buffer;
	buffer.reserve( 10000 );
	buffer.push_back( 'G' );
	buffer.push_back( 'C' );
	buffer.push_back( 'H' );
	buffer.push_back( 'K' );
	buffer.push_back( (uint8_t)1 );
	buffer.push_back( (uint8_t)XBITS );
	buffer.push_back( (uint8_t)YBITS );
	buffer.push_back( (uint8_t)ZBITS );

	uint8_t* seedArray = (uint8_t*)&g_terrainSeed;

	buffer.push_back( seedArray[0] );
	buffer.push_back( seedArray[1] );
	buffer.push_back( seedArray[2] );
	buffer.push_back( seedArray[3] );

	uint8_t curBlockType = m_blocks[0].m_type;
	int numOfThisType = 1;
	for (int i = 1; i < BLOCK_COUNT_EACH_CHUNK; i++) {
		if (m_blocks[i].m_type != curBlockType || numOfThisType == 255) {
			// push back data
			buffer.push_back( curBlockType );
			buffer.push_back( (uint8_t)numOfThisType );
			numOfThisType = 1;
			curBlockType = m_blocks[i].m_type;
		}
		else {
			numOfThisType++;
		}
	}
	buffer.push_back( curBlockType );
	buffer.push_back( (uint8_t)numOfThisType );

	CreateDirectoryA( Stringf( "Saves/World%u", g_terrainSeed ).c_str(), NULL );
	BufferWriteToFile( buffer, Stringf( "Saves/World%u/Chunk(%d,%d).chunk", g_terrainSeed, m_coords.x, m_coords.y ) );
}

bool Chunk::ReadFromFile()
{
	std::vector<uint8_t> buffer;
	int res = FileReadToBuffer( buffer, Stringf( "Saves/World%u/Chunk(%d,%d).chunk", g_terrainSeed, m_coords.x, m_coords.y ) );
	if (res == -1) {
		return false;
	}
	GUARANTEE_OR_DIE( buffer[0] == 'G' && buffer[1] == 'C' && buffer[2] == 'H' && buffer[3] == 'K' && (int)buffer.size() % 2 == 0, "Error! Format of save file is wrong!" );
	
	if (buffer[5] != XBITS || buffer[6] != YBITS || buffer[7] != ZBITS) {
		return false;
	}

	unsigned int* seed = (unsigned int*)&buffer[8];

	if (*seed != g_terrainSeed) {
		return false;
	}
	
	int counter = 0;
	for (int i = 12; i < (int)buffer.size(); i+=2) {
		for (int k = 0; k < (int)buffer[i + 1]; k++) {
			m_blocks[counter].SetType( buffer[i] );
			counter++;
		}
	}

	GUARANTEE_OR_DIE( counter == BLOCK_COUNT_EACH_CHUNK, "Error! Save file is not in right format!" );
	return true;
}

void Chunk::MarkDirty()
{
	if (!m_isDirty) {
		m_isDirty = true;
		g_theWorld->m_dirtyChunks.push_back( this );
	}
}
