#include "Game/World.hpp"
#include "Game/Chunk.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include <filesystem>

unsigned int g_terrainSeed = 0;
unsigned int g_hillinessSeed = 0;
unsigned int g_humiditySeed = 0;
unsigned int g_temperatureSeed = 0;
unsigned int g_oceannessSeed = 0;
unsigned int g_treeDensitySeed = 0;
unsigned int g_treeSeed = 0;
unsigned int g_wormSeed = 0;

World::World()
	:m_airDef(BlockDefinition::GetDefinitionByName("air"))
{
	m_chunkActivationRange = g_chunkActivationDist;
	m_chunkDeactivationRange = m_chunkActivationRange + XSIZE + YSIZE;

	m_maxChunksRadiusX = 1 + int( m_chunkActivationRange ) / XSIZE;
	m_maxChunksRadiusY = 1 + int( m_chunkActivationRange ) / YSIZE;
	m_maxChunks = (2 * m_maxChunksRadiusX) * (2 * m_maxChunksRadiusY);

	m_worldCBO = g_theRenderer->CreateConstantBuffer( sizeof( SimplerMinerConstants ) );

	m_worldConstans.m_fogNearDist = m_chunkActivationRange * 0.5f;
	m_worldConstans.m_fogFarDist = m_chunkActivationRange - 16.f;
	//m_worldConstans.m_fogNearDist = 1000000.f;
	//m_worldConstans.m_fogFarDist = 1000000.f;

	m_days = 0.4f;
	g_terrainSeed = (unsigned int)g_gameConfigBlackboard.GetValue( "worldSeed", (int)g_terrainSeed );
	g_hillinessSeed = g_terrainSeed - 3;
	g_humiditySeed = g_terrainSeed - 2;
	g_temperatureSeed = g_terrainSeed + 1;
	g_oceannessSeed = g_terrainSeed + 2;
	g_treeDensitySeed = g_terrainSeed + 3;
	g_treeSeed = g_terrainSeed - 1;
	g_wormSeed = g_terrainSeed + 4;
}

World::~World()
{
	std::vector<Chunk*> chunks;
	chunks.reserve( 100 );
	for (auto& chunkPair : m_activeChunks) {
		chunks.push_back( chunkPair.second );
	}
	for (auto chunk : chunks) {
		if (chunk->m_needsToSave) {
			chunk->SaveToFile();
		}
		DeconstructChunk( chunk );
		//DeactivateChunk( chunk );
	}
	delete m_worldCBO;
}

void World::StartUp()
{
	m_rayMaxDist = 8.f;
	m_player = new Player( g_theGame );
	m_player->m_position = Vec3( 0.f, 0.f, 70.f );
	m_player->m_orientation = EulerAngles( 0.f, 60.f, 0.f );
	m_player->m_camera.SetPerspectiveView( g_window->GetAspect(), 60.f, 0.1f, 1000.f );
	m_player->m_camera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	m_player->m_camera.SetTransform( m_player->m_position, m_player->m_orientation );
	g_theGame->AddEntityToEntityArries( m_player );

	m_worldShader = g_theRenderer->CreateShader( "Data/Shaders/World" );
}

void World::Update()
{
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();

	if (g_theInput->IsKeyDown( 'Y' )) {
		m_worldTimeScale = 10000.f;
	}
	else {
		m_worldTimeScale = 200.f;
	}

	m_days += ((deltaSeconds * m_worldTimeScale) / (24.f * 60.f * 60.f));

	if (!m_rayLock) {
		m_rayStartPos = m_player->m_position;
		m_rayForwardNormal = m_player->GetForwardNormal();
	}
	m_hasRayHitRes = RayCastVsWorld( m_hitRes, m_rayStartPos, m_rayForwardNormal, m_rayMaxDist );

	if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
		//DigTheTopBlock();
		if (m_hasRayHitRes) {
			m_hitRes.m_res.m_chunk->SetBlockType( m_hitRes.m_res.m_blockIndex, m_airDef.m_index, false );
		}
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTMOUSE )) {
		PutBlockToPos();
	}
	if (g_theInput->WasKeyJustPressed( 'R' )) {
		m_rayLock = !m_rayLock;
	}

	ChangeCurrentBlockType();

	ProcessDirtyLighting();

	//if (g_theInput->WasKeyJustPressed( 'L' )) {
	DoChunkDynamicActivation();
	//}

	//double begin = GetCurrentTimeSeconds();
	int retrieveCount = 0;
	for (int i = 0; i < (int)m_chunkGenerationJobs.size(); i++) {
		if (m_chunkGenerationJobs[i]->m_status == JobStatus::Completed && (int)m_activeChunks.size() < m_maxChunks) {
			retrieveCount++;
			g_theJobSystem->RetrieveJob( m_chunkGenerationJobs[i] );
			StartUpChunk( m_chunkGenerationJobs[i]->m_chunk );
			m_queuedGenerateChunks.erase( m_chunkGenerationJobs[i]->m_chunk->m_coords );
			delete m_chunkGenerationJobs[i];
			m_chunkGenerationJobs.erase( m_chunkGenerationJobs.begin() + i );
			i--;
			//if (retrieveCount >= 100) {
			//	break;
			//}
		}
	}
	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );

	for (int i = 0; i < (int)m_chunkSaveJobs.size(); i++) {
		if (m_chunkSaveJobs[i]->m_status == JobStatus::Completed) {
			g_theJobSystem->RetrieveJob( m_chunkSaveJobs[i] );
			DeconstructChunk( m_chunkSaveJobs[i]->m_chunk );
			delete m_chunkSaveJobs[i];
			m_chunkSaveJobs.erase( m_chunkSaveJobs.begin() + i );
			i--;
		}
	}

	float minDist = FLT_MAX;
	float secondMinDist = FLT_MAX;
	Chunk* firstMin = nullptr;
	Chunk* secondMin = nullptr;

	for (auto chunk : m_dirtyChunks) {
		//if (chunk->m_westNeighbor && chunk->m_eastNeighbor && chunk->m_northNeighbor && chunk->m_southNeighbor) {
			float dist = GetDistanceSquared2D( GetChunkWorldCenterXY( chunk->m_coords ), m_player->m_position );
			// smaller than the first
			if (dist < minDist) {
				// set the first to be the second
				secondMinDist = minDist;
				secondMin = firstMin;
				// set it to be the first
				minDist = dist;
				firstMin = chunk;
			}
			else if (dist < secondMinDist) {
				secondMinDist = dist;
				secondMin = chunk;
			}
		//}
	}

	if (firstMin) {
		firstMin->Update( deltaSeconds );
	}
	if (secondMin) {
		secondMin->Update( deltaSeconds );
	}

	for (int i = 0; i < (int)m_dirtyChunks.size(); i++) {
		if (m_dirtyChunks[i] == firstMin) {
			m_dirtyChunks.erase( m_dirtyChunks.begin() + i );
			i--;
		}
		if (i >= 0 && m_dirtyChunks[i] == secondMin) {
			m_dirtyChunks.erase( m_dirtyChunks.begin() + i );
			i--;
		}
	}
	//for (auto& pair : m_activeChunks) {
	//	pair.second->Update( deltaSeconds );
	//}

	UpdateTimeAndSkyColor();
}

void World::Render() const
{
	//double begin = GetCurrentTimeSeconds();

	for (auto chunkPair : m_activeChunks) {
		chunkPair.second->Render();
	}

	if (m_hasRayHitRes) {
		std::vector<Vertex_PCU> verts;
		Vec3 blockCenter = m_hitRes.m_res.GetWorldCenter();
		Vec3 BLPos = m_hitRes.m_impactNormal * 0.03f;
		Vec3 BRPos = m_hitRes.m_impactNormal * 0.03f;
		Vec3 TLPos = m_hitRes.m_impactNormal * 0.03f;
		Vec3 TRPos = m_hitRes.m_impactNormal * 0.03f;
		if (m_hitRes.m_impactNormal.x == 1.f) {
			BLPos += blockCenter + Vec3( 0.5f, -0.5f, -0.5f );
			BRPos += blockCenter + Vec3( 0.5f, 0.5f, -0.5f );
			TLPos += blockCenter + Vec3( 0.5f, -0.5f, 0.5f );
			TRPos += blockCenter + Vec3( 0.5f, 0.5f, 0.5f );
		}
		else if (m_hitRes.m_impactNormal.x == -1.f) {
			BLPos += blockCenter + Vec3( -0.5f, 0.5f, -0.5f );
			BRPos += blockCenter + Vec3( -0.5f, -0.5f, -0.5f );
			TLPos += blockCenter + Vec3( -0.5f, 0.5f, 0.5f );
			TRPos += blockCenter + Vec3( -0.5f, -0.5f, 0.5f );
		}
		else if (m_hitRes.m_impactNormal.y == 1.f) {
			BLPos += blockCenter + Vec3( 0.5f, 0.5f, -0.5f );
			BRPos += blockCenter + Vec3( -0.5f, 0.5f, -0.5f );
			TLPos += blockCenter + Vec3( 0.5f, 0.5f, 0.5f );
			TRPos += blockCenter + Vec3( -0.5f, 0.5f, 0.5f );
		}
		else if (m_hitRes.m_impactNormal.y == -1.f) {
			BLPos += blockCenter + Vec3( -0.5f, -0.5f, -0.5f );
			BRPos += blockCenter + Vec3( 0.5f, -0.5f, -0.5f );
			TLPos += blockCenter + Vec3( -0.5f, -0.5f, 0.5f );
			TRPos += blockCenter + Vec3( 0.5f, -0.5f, 0.5f );
		}
		else if (m_hitRes.m_impactNormal.z == 1.f) {
			BLPos += blockCenter + Vec3( -0.5f, -0.5f, 0.5f );
			BRPos += blockCenter + Vec3( 0.5f, -0.5f, 0.5f );
			TLPos += blockCenter + Vec3( -0.5f, 0.5f, 0.5f );
			TRPos += blockCenter + Vec3( 0.5f, 0.5f, 0.5f );
		}
		else {
			BLPos += blockCenter + Vec3( -0.5f, 0.5f, -0.5f );
			BRPos += blockCenter + Vec3( 0.5f, 0.5f, -0.5f );
			TLPos += blockCenter + Vec3( -0.5f, -0.5f, -0.5f );
			TRPos += blockCenter + Vec3( 0.5f, -0.5f, -0.5f );
		}

		AddVertsForWiredQuad3D( verts, BLPos, BRPos, TRPos, TLPos, Rgba8( 255, 0, 255 ) );
		if (m_rayLock) {
			AddVertsForLineSegment3D( verts, m_rayStartPos, m_hitRes.m_impactPos, 0.02f, Rgba8( 255, 0, 0 ), 8 );

			std::vector<Vertex_PCU> rayLockVerts;
			AddVertsForLineSegment3D( rayLockVerts, m_hitRes.m_impactPos, m_rayStartPos + m_rayForwardNormal * m_rayMaxDist, 0.02f, Rgba8( 100, 100, 100 ), 8 );
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::ENABLED );
			g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
			g_theRenderer->DrawVertexArray( rayLockVerts );
			rayLockVerts.clear();

			AddVertsForLineSegment3D( rayLockVerts, m_hitRes.m_impactPos, m_rayStartPos + m_rayForwardNormal * m_rayMaxDist, 0.02f, Rgba8( 150, 150, 150, 150 ), 8 );
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
			g_theRenderer->DrawVertexArray( rayLockVerts );
		}

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexArray( verts );

	}

	if (m_rayLock && !m_hasRayHitRes) {
		std::vector<Vertex_PCU> verts;
		AddVertsForLineSegment3D( verts, m_rayStartPos, m_rayStartPos + m_rayForwardNormal * m_rayMaxDist, 0.02f, Rgba8( 0, 255, 0 ), 8 );

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->DrawVertexArray( verts );
	}

	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );
}

void World::RenderUI() const
{
	std::vector<Vertex_PCU> textVerts;

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 10.f ), Vec2( 1600.f, 50.f ) ), 40.f, Stringf( "%s", BlockDefinition::GetDefinitionByIndex( (unsigned char)m_curBlockIDToPut ).m_name.c_str() ) );

	// render threading test game
	/*std::vector<Vertex_PCU> verts;
	Vec2 LeftTopPos( 25.f, 787.5f );
	constexpr float totalWidth = 1550.f;
	constexpr float totalHeight = 775.f;
	constexpr float widthPerBlock = totalWidth / 40.f;
	constexpr float heightPerBlock = totalHeight / 20.f;
	constexpr float margin = 0.1f;

	std::vector<SimpleMinerJob*> const& m_jobList = m_chunkGenerationJobs;
	for (int i = (int)m_jobList.size() > 800 ? (int)m_jobList.size() - 800 : 0; i < (int)m_jobList.size(); i++) {
		int k = i;
		if ((int)m_jobList.size() > 800) {
			k = k + 800 - (int)m_jobList.size();
		}
		int y = k / 40;
		int x = k - y * 40;
		Rgba8 color;
		if (m_jobList[i]->m_status == JobStatus::Queued) {
			color = Rgba8( 255, 0, 0 );
		}
		else if (m_jobList[i]->m_status == JobStatus::Executing) {
			color = Rgba8( 255, 255, 0 );
		}
		else if (m_jobList[i]->m_status == JobStatus::Completed) {
			color = Rgba8( 0, 255, 0 );
		}
		else if (m_jobList[i]->m_status == JobStatus::Retrieved) {
			color = Rgba8( 0, 0, 255 );
		}
		else if (m_jobList[i]->m_status == JobStatus::NoRecord) {
			color = Rgba8( 255, 0, 255 );
		}
		Vec2 leftBottomPos = LeftTopPos + Vec2( widthPerBlock * (float)x, -heightPerBlock * (float)(y + 1) );
		AddVertsForAABB2D( verts, AABB2( leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * margin, leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * (1.f - margin) ), color );
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( verts );
	*/

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( textVerts );
}

Player* World::GetPlayer() const
{
	return m_player;
}

Chunk* World::CreateChunk( IntVec2 const& chunkCoords )
{
	Chunk* chunk = new Chunk( chunkCoords );


	return chunk;
}

IntVec2 World::GetChunkCoordsByWorldPos( Vec2 const& worldXYPos ) const
{
	int x = RoundDownToInt( worldXYPos.x );
	int y = RoundDownToInt( worldXYPos.y );
	x = (x >> XBITS);
	y = (y >> YBITS);
	return IntVec2( x, y );
}

size_t World::GetVertsCount() const
{
	size_t res = 0;
	for (auto& pair : m_activeChunks) {
		res += pair.second->m_verts.size();
	}
	return res;
}

IntVec3 World::GetBlockCoordsByWorldPosition( Vec3 const& worldPos ) const
{
	return IntVec3( RoundDownToInt( worldPos.x ), RoundDownToInt( worldPos.y ), RoundDownToInt( worldPos.z ) );
}

BlockIter World::CreatBlockIter( Vec3 const& worldPos ) const
{
	Chunk* chunk = GetChunkByCoords( GetChunkCoordsByWorldPos( worldPos ) );
	if (chunk == nullptr || worldPos.z >= (float)ZSIZE || worldPos.z < 0.f) {
		return BlockIter();
	}
	return BlockIter( ::GetBlockIndex( chunk->GetBlockLocalCoords( GetBlockCoordsByWorldPosition( worldPos ) ) ), chunk );
}

void World::DoChunkDynamicActivation()
{
	bool m_isAChunkActivated = false;
	IntVec2 playerChunkCoords = GetPlayerCurrentChunkCoords();
	//double begin = GetCurrentTimeSeconds();
	if ((int)m_activeChunks.size() < m_maxChunks) {
		// activate is priority
		if (g_autoCreateChunks) {
			//constexpr int queueMaxSize = 4;
			//while ((int)m_chunkGenerationJobs.size() < queueMaxSize) {
			//	if (ActivateTheNearestInRangeChunk( playerChunkCoords )) {
			//		m_isAChunkActivated = true;
			//	}
			//}
			m_isAChunkActivated = ActivateAllInRangeChunks( playerChunkCoords );
		}
	}
	if (!m_isAChunkActivated) {
		// deactivate the farthest chunk
		DeactivateTheFarthestOutRangeChunk( playerChunkCoords );
	}
	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );
}

bool World::ActivateTheNearestInRangeChunk( IntVec2 const& playerChunkCoords )
{
	bool findNearest = false;
	IntVec2 coords = playerChunkCoords;
	float minDistSquared = m_chunkActivationRange * m_chunkActivationRange;
	Chunk* chunkPtr = nullptr;
	for (int y = playerChunkCoords.y - m_maxChunksRadiusY + 1; y <= playerChunkCoords.y + m_maxChunksRadiusY - 1; y++) {
		for (int x = playerChunkCoords.x - m_maxChunksRadiusX + 1; x <= playerChunkCoords.x + m_maxChunksRadiusX - 1; x++) {
			if (!GetChunkByCoords( IntVec2( x, y ), &chunkPtr ) && m_queuedGenerateChunks.find( IntVec2( x ,y ) ) == m_queuedGenerateChunks.end()
				&& GetDistanceSquared2D( GetChunkWorldCenterXY( IntVec2( x, y ) ), m_player->m_position ) < minDistSquared) {
				float dist = GetDistanceSquared2D( GetChunkWorldCenterXY( IntVec2( x, y ) ), m_player->m_position );
				if (dist < minDistSquared) {
					minDistSquared = dist;
					coords = IntVec2( x, y );
					findNearest = true;
				}
			}
		}
	}
	if (findNearest) {
		Chunk* chunk = CreateChunk( coords );
		m_queuedGenerateChunks.insert( coords );
		Job* job = nullptr;
		if (std::filesystem::exists( Stringf( "Saves/World%u/Chunk(%d,%d).chunk", g_terrainSeed, coords.x, coords.y ) )) {
			job = new ChunkLoadJob( chunk );
		}
		else {
			job = new ChunkGenerateJob( chunk );
		}
		m_chunkGenerationJobs.push_back( (SimpleMinerJob*)job );
		g_theJobSystem->AddJob( job );
		return true;
	}
	else {
		return false;
	}
}

bool World::ActivateAllInRangeChunks( IntVec2 const& playerChunkCoords )
{
	bool findNearest = false;
	IntVec2 coords = playerChunkCoords;
	float minDistSquared = m_chunkActivationRange * m_chunkActivationRange;
	Chunk* chunkPtr = nullptr;
	for (int y = playerChunkCoords.y - m_maxChunksRadiusY + 1; y <= playerChunkCoords.y + m_maxChunksRadiusY - 1; y++) {
		for (int x = playerChunkCoords.x - m_maxChunksRadiusX + 1; x <= playerChunkCoords.x + m_maxChunksRadiusX - 1; x++) {
			if (!GetChunkByCoords( IntVec2( x, y ), &chunkPtr ) && m_queuedGenerateChunks.find( IntVec2( x, y ) ) == m_queuedGenerateChunks.end()
				&& GetDistanceSquared2D( GetChunkWorldCenterXY( IntVec2( x, y ) ), m_player->m_position ) < minDistSquared) {
				coords = IntVec2( x, y );
				findNearest = true;
				Chunk* chunk = CreateChunk( coords );
				m_queuedGenerateChunks.insert( coords );
				Job* job = nullptr;
				if (std::filesystem::exists( Stringf( "Saves/World%u/Chunk(%d,%d).chunk", g_terrainSeed, coords.x, coords.y ) )) {
					job = new ChunkLoadJob( chunk );
				}
				else {
					job = new ChunkGenerateJob( chunk );
				}
				m_chunkGenerationJobs.push_back( (SimpleMinerJob*)job );
				g_theJobSystem->AddJob( job );
			}
		}
	}
	if (findNearest) {
		return true;
	}
	else {
		return false;
	}
}

bool World::DeactivateTheFarthestOutRangeChunk( IntVec2 const& playerChunkCoords )
{
	bool findFarthest = false;
	IntVec2 coords = playerChunkCoords;
	float maxDistSquared = m_chunkDeactivationRange * m_chunkDeactivationRange;
	Chunk* chunkPtr = nullptr;
	std::map<IntVec2, Chunk*> copyedMap = m_activeChunks;
	for (auto iter = copyedMap.begin(); iter != copyedMap.end(); ++iter) {
		float dist = GetDistanceSquared2D( iter->second->GetWorldCenterXY(), m_player->m_position );
		if (dist > maxDistSquared) {
			chunkPtr = iter->second;
			m_activeChunks.erase( iter->first );
			DeactivateChunk( chunkPtr );
			findFarthest = true;
		}
		
	}
	if (findFarthest) {
		return true;
	}
	else {
		return false;
	}
}

//void World::ActivateChunk( IntVec2 const& coords )
//{
	//CreateChunk( coords );
//}

void World::DeactivateChunk( Chunk* chunk )
{
	for (int i = 0; i < (int)m_dirtyChunks.size(); i++) {
		if (m_dirtyChunks[i] == chunk) {
			m_dirtyChunks.erase( i + m_dirtyChunks.begin() );
			break;
		}
	}

	if (chunk->m_eastNeighbor) {
		chunk->m_eastNeighbor->m_westNeighbor = nullptr;
	}
	if (chunk->m_westNeighbor) {
		chunk->m_westNeighbor->m_eastNeighbor = nullptr;
	}
	if (chunk->m_northNeighbor) {
		chunk->m_northNeighbor->m_southNeighbor = nullptr;
	}
	if (chunk->m_southNeighbor) {
		chunk->m_southNeighbor->m_northNeighbor = nullptr;
	}

	if (chunk->m_needsToSave) {
		ChunkSaveJob* job = new ChunkSaveJob( chunk );
		//chunk->SaveToFile();
		m_chunkSaveJobs.push_back( job );
		g_theJobSystem->AddJob( job );
	}
	else {
		DeconstructChunk( chunk );
	}
}

void World::StartUpChunk( Chunk* chunk )
{
	Chunk* neighbor = nullptr;
	IntVec2 const& chunkCoords = chunk->m_coords;
	neighbor = GetChunkByCoords( chunkCoords + IntVec2( 1, 0 ) );
	if (neighbor) {
		chunk->m_eastNeighbor = neighbor;
		neighbor->m_westNeighbor = chunk;
	}
	neighbor = GetChunkByCoords( chunkCoords + IntVec2( -1, 0 ) );
	if (neighbor) {
		chunk->m_westNeighbor = neighbor;
		neighbor->m_eastNeighbor = chunk;
	}
	neighbor = GetChunkByCoords( chunkCoords + IntVec2( 0, 1 ) );
	if (neighbor) {
		chunk->m_northNeighbor = neighbor;
		neighbor->m_southNeighbor = chunk;
	}
	neighbor = GetChunkByCoords( chunkCoords + IntVec2( 0, -1 ) );
	if (neighbor) {
		chunk->m_southNeighbor = neighbor;
		neighbor->m_northNeighbor = chunk;
	}

	m_activeChunks[chunkCoords] = chunk;
	chunk->StartUp();
}

bool World::GetChunkByCoords( IntVec2 const& coords, Chunk** out_chunkPtr ) const
{
	auto iter = m_activeChunks.find( coords );
	if (iter != m_activeChunks.end()) {
		*out_chunkPtr = iter->second;
		return true;
	}
	return false;
}

Chunk* World::GetChunkByCoords( IntVec2 const& coords ) const
{
	auto iter = m_activeChunks.find( coords );
	if (iter != m_activeChunks.end()) {
		return iter->second;
	}
	return nullptr;
}

IntVec2 World::GetPlayerCurrentChunkCoords() const
{
	Vec2 XYPosition = Vec2( m_player->m_position );
	return GetChunkCoordsByWorldPos( XYPosition );
}

Vec2 World::GetChunkWorldCenterXY( IntVec2 const& chunkCoords ) const
{
	IntVec3 chunkOriginWorldCoords = IntVec3( chunkCoords.x << XBITS, chunkCoords.y << YBITS, 0 );
	return Vec2( (float)chunkOriginWorldCoords.x + (XSIZE >> 1), (float)chunkOriginWorldCoords.y + (YSIZE >> 1) );
}

void World::DigTheTopBlock()
{
	IntVec2 chunkCoords = GetChunkCoordsByWorldPos( m_player->m_position );
	auto iter = m_activeChunks.find( chunkCoords );
	if (iter == m_activeChunks.end()) {
		return;
	}
	Chunk* chunk = iter->second;
	IntVec3 blockWorldCoords = GetBlockCoordsByWorldPosition( m_player->m_position );
	IntVec3 blockLocalCoords = chunk->GetBlockLocalCoords( blockWorldCoords );
	chunk->DigTopBlockOfStack( blockLocalCoords );
}

void World::PutTheTopBlock()
{
	IntVec2 chunkCoords = GetChunkCoordsByWorldPos( m_player->m_position );
	auto iter = m_activeChunks.find( chunkCoords );
	if (iter == m_activeChunks.end()) {
		return;
	}
	Chunk* chunk = iter->second;
	IntVec3 blockWorldCoords = GetBlockCoordsByWorldPosition( m_player->m_position );
	IntVec3 blockLocalCoords = chunk->GetBlockLocalCoords( blockWorldCoords );
	chunk->PutTopBlockOfStack( blockLocalCoords );
}

void World::PutBlockToPos()
{
	if (m_hasRayHitRes) {
		BlockIter iter;
		if (m_hitRes.m_impactNormal.x == -1.f) {
			iter = m_hitRes.m_res.GetWestNeighbor();
		}
		else if (m_hitRes.m_impactNormal.x == 1.f) {
			iter = m_hitRes.m_res.GetEastNeighbor();
		}
		else if (m_hitRes.m_impactNormal.y == -1.f) {
			iter = m_hitRes.m_res.GetSouthNeighbor();
		}
		else if (m_hitRes.m_impactNormal.y == 1.f) {
			iter = m_hitRes.m_res.GetNorthNeighbor();
		}
		else if (m_hitRes.m_impactNormal.z == -1.f) {
			iter = m_hitRes.m_res.GetDownNeighbor();
		}
		else {
			iter = m_hitRes.m_res.GetUpNeighbor();
		}

		if (iter.m_chunk && iter.GetBlock()->GetType() == m_airDef.m_index) {
			iter.m_chunk->SetBlockType( iter.m_blockIndex, m_curBlockIDToPut );
		}
	}
}

void World::ChangeCurrentBlockType()
{
	if (g_theInput->WasKeyJustPressed( '1' )) {
		m_curBlockIDToPut = 1;
	}
	if (g_theInput->WasKeyJustPressed( '2' )) {
		m_curBlockIDToPut = 2;
	}
	if (g_theInput->WasKeyJustPressed( '3' )) {
		m_curBlockIDToPut = 3;
	}
	if (g_theInput->WasKeyJustPressed( '4' )) {
		m_curBlockIDToPut = 4;
	}
	if (g_theInput->WasKeyJustPressed( '5' )) {
		m_curBlockIDToPut = 5;
	}
	if (g_theInput->WasKeyJustPressed( '6' )) {
		m_curBlockIDToPut = 6;
	}
	if (g_theInput->WasKeyJustPressed( '7' )) {
		m_curBlockIDToPut = 7;
	}
	if (g_theInput->WasKeyJustPressed( '8' )) {
		m_curBlockIDToPut = 8;
	}
	if (g_theInput->WasKeyJustPressed( '9' )) {
		m_curBlockIDToPut = 9;
	}

}

void World::ProcessDirtyLighting()
{
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Num of dirty light this frame %d", m_blockLightingQueue.size() ) );
	while (!m_blockLightingQueue.empty()) {
		ProcessNextDirtyLightBlock();
	}
}

void World::ProcessNextDirtyLightBlock()
{
	BlockIter thisIter = m_blockLightingQueue.front();
	m_blockLightingQueue.pop_front();
	// compute lighting

	// indoor lighting
	// calculate how much light strength it should be
	unsigned char indoorLightStrength = 0;

	// light source?
	if (thisIter.GetBlock()->GetDefinition().m_lightStrength > 0) {
		indoorLightStrength = (unsigned char)thisIter.GetBlock()->GetDefinition().m_lightStrength;
	}

	BlockIter westIter = thisIter.GetWestNeighbor();
	if (westIter.m_chunk && (!westIter.GetBlock()->IsOpaque() || westIter.GetBlock()->GetDefinition().m_lightStrength > 0)) {
		indoorLightStrength = westIter.GetBlock()->GetIndoorLightInfluence() - 1 > indoorLightStrength ? westIter.GetBlock()->GetIndoorLightInfluence() - 1 : indoorLightStrength;
	}
	BlockIter eastIter = thisIter.GetEastNeighbor();
	if (eastIter.m_chunk && (!eastIter.GetBlock()->IsOpaque() || eastIter.GetBlock()->GetDefinition().m_lightStrength > 0)) {
		indoorLightStrength = eastIter.GetBlock()->GetIndoorLightInfluence() - 1 > indoorLightStrength ? eastIter.GetBlock()->GetIndoorLightInfluence() - 1 : indoorLightStrength;
	}
	BlockIter southIter = thisIter.GetSouthNeighbor();
	if (southIter.m_chunk && (!southIter.GetBlock()->IsOpaque() || southIter.GetBlock()->GetDefinition().m_lightStrength > 0)) {
		indoorLightStrength = southIter.GetBlock()->GetIndoorLightInfluence() - 1 > indoorLightStrength ? southIter.GetBlock()->GetIndoorLightInfluence() - 1 : indoorLightStrength;
	}
	BlockIter northIter = thisIter.GetNorthNeighbor();
	if (northIter.m_chunk && (!northIter.GetBlock()->IsOpaque() || northIter.GetBlock()->GetDefinition().m_lightStrength > 0)) {
		indoorLightStrength = northIter.GetBlock()->GetIndoorLightInfluence() - 1 > indoorLightStrength ? northIter.GetBlock()->GetIndoorLightInfluence() - 1 : indoorLightStrength;
	}
	BlockIter upIter = thisIter.GetUpNeighbor();
	if (upIter.m_chunk && (!upIter.GetBlock()->IsOpaque() || upIter.GetBlock()->GetDefinition().m_lightStrength > 0)) {
		indoorLightStrength = upIter.GetBlock()->GetIndoorLightInfluence() - 1 > indoorLightStrength ? upIter.GetBlock()->GetIndoorLightInfluence() - 1 : indoorLightStrength;
	}
	BlockIter downIter = thisIter.GetDownNeighbor();
	if (downIter.m_chunk && (!downIter.GetBlock()->IsOpaque() || downIter.GetBlock()->GetDefinition().m_lightStrength > 0)) {
		indoorLightStrength = downIter.GetBlock()->GetIndoorLightInfluence() - 1 > indoorLightStrength ? downIter.GetBlock()->GetIndoorLightInfluence() - 1 : indoorLightStrength;
	}

	unsigned char outdoorLightStrength = 0;
	if (thisIter.GetBlock()->IsSky()) {
		outdoorLightStrength = 15;
	}
	else {
		if (westIter.m_chunk) {
			outdoorLightStrength = westIter.GetBlock()->GetOutdoorLightInfluence() - 1 > outdoorLightStrength ? westIter.GetBlock()->GetOutdoorLightInfluence() - 1 : outdoorLightStrength;
		}
		if (eastIter.m_chunk) {
			outdoorLightStrength = eastIter.GetBlock()->GetOutdoorLightInfluence() - 1 > outdoorLightStrength ? eastIter.GetBlock()->GetOutdoorLightInfluence() - 1 : outdoorLightStrength;
		}
		if (southIter.m_chunk) {
			outdoorLightStrength = southIter.GetBlock()->GetOutdoorLightInfluence() - 1 > outdoorLightStrength ? southIter.GetBlock()->GetOutdoorLightInfluence() - 1 : outdoorLightStrength;
		}
		if (northIter.m_chunk) {
			outdoorLightStrength = northIter.GetBlock()->GetOutdoorLightInfluence() - 1 > outdoorLightStrength ? northIter.GetBlock()->GetOutdoorLightInfluence() - 1 : outdoorLightStrength;
		}
		if (upIter.m_chunk) {
			outdoorLightStrength = upIter.GetBlock()->GetOutdoorLightInfluence() - 1 > outdoorLightStrength ? upIter.GetBlock()->GetOutdoorLightInfluence() - 1 : outdoorLightStrength;
		}
		if (downIter.m_chunk) {
			outdoorLightStrength = downIter.GetBlock()->GetOutdoorLightInfluence() - 1 > outdoorLightStrength ? downIter.GetBlock()->GetOutdoorLightInfluence() - 1 : outdoorLightStrength;
		}
	}

	// need to reset the light strength?
	if (indoorLightStrength != thisIter.GetBlock()->GetIndoorLightInfluence() || outdoorLightStrength != thisIter.GetBlock()->GetOutdoorLightInfluence()) {
		thisIter.GetBlock()->SetIndoorLightInfluence( indoorLightStrength );
		if (thisIter.GetBlock()->IsOpaque()) {
			thisIter.GetBlock()->SetOutdoorLightInfluence( 0 );
		}
		else {
			thisIter.GetBlock()->SetOutdoorLightInfluence( outdoorLightStrength );
		}
		// propagate
		if (westIter.IsValid() && !westIter.GetBlock()->IsOpaque()) {
			MarkLightingDirty( westIter );
		}
		if (eastIter.IsValid() && !eastIter.GetBlock()->IsOpaque()) {
			MarkLightingDirty( eastIter );
		}
		if (northIter.IsValid() && !northIter.GetBlock()->IsOpaque()) {
			MarkLightingDirty( northIter );
		}
		if (southIter.IsValid() && !southIter.GetBlock()->IsOpaque()) {
			MarkLightingDirty( southIter );
		}
		if (upIter.IsValid() && !upIter.GetBlock()->IsOpaque()) {
			MarkLightingDirty( upIter );
		}
		if (downIter.IsValid() && !downIter.GetBlock()->IsOpaque()) {
			MarkLightingDirty( downIter );
		}

		// mark every neighbor dirty
		if (westIter.IsValid() && thisIter.IsOnWestEdge()) {
			westIter.m_chunk->MarkDirty();
		}
		if (eastIter.IsValid() && thisIter.IsOnEastEdge()) {
			eastIter.m_chunk->MarkDirty();
		}
		if (northIter.IsValid() && thisIter.IsOnNorthEdge()) {
			northIter.m_chunk->MarkDirty();
		}
		if (southIter.IsValid() && thisIter.IsOnSouthEdge()) {
			southIter.m_chunk->MarkDirty();
		}
	}

	thisIter.GetBlock()->SetLightDirty( false );
}

void World::UndirtyAllBlocksInChunk( Chunk* chunk )
{
	for (auto iter = m_blockLightingQueue.begin(); iter != m_blockLightingQueue.end(); iter++) {
		if (iter->m_chunk == chunk) {
			m_blockLightingQueue.erase( iter );
			iter--;
		}
	}
}

void World::MarkLightingDirty( BlockIter const& iter )
{
	if (!iter.GetBlock()->IsLightDirty()) {
		iter.GetBlock()->SetLightDirty( true );
		iter.m_chunk->MarkDirty();
		m_blockLightingQueue.push_back( iter );
	}
}

float World::GetDayTimeFraction() const
{
	return m_days - (float)RoundDownToInt( m_days );
}

bool World::IsTimeInNight() const
{
	float curTime = GetDayTimeFraction();
	if (curTime < 0.25f || curTime > 0.75f) {
		return true;
	}
	else {
		return false;
	}
}

std::string World::GetCurDayTimeText() const
{
	int day = RoundDownToInt( m_days );
	float hours = m_days - (float)day;
	float normalizedHours = hours * 24.f;
	int actualHours = RoundDownToInt( normalizedHours );
	float remainMinutes = normalizedHours - (float)actualHours;
	int actualMinutes = RoundDownToInt( remainMinutes * 60.f );
	return Stringf( "Day:%d  %02d:%02d", day, actualHours, actualMinutes );
}

bool World::RayCastVsWorld( GameRayCast3DRes& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist ) const
{
	// record the ray information
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;
	// calculate current block
	BlockIter curBlock = CreatBlockIter( startPos );
	// IntVec2 curTile = RoundDownPos( startPos );
	if (curBlock.m_chunk == nullptr) {
		out_rayCastRes.m_didImpact = false;
		return false;
	}
	// if current tile is solid, cannot achieve in real game
	if (curBlock.GetBlock()->IsOpaque()) {
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		return true;
	}
	// calculate essential variables
	float fwdDistPerXCrossing = 1 / abs( forwardNormal.x );
	int tileStepDirectionX = forwardNormal.x < 0 ? -1 : 1;
	float xAtFirstXCrossing = curBlock.GetBlockWorldX() + ((float)tileStepDirectionX + 1.f) * 0.5f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - startPos.x;
	float fwdDistAtNextXCrossing = abs( xDistToFirstXCrossing ) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = 1 / abs( forwardNormal.y );
	int tileStepDirectionY = forwardNormal.y < 0 ? -1 : 1;
	float yAtFirstYCrossing = curBlock.GetBlockWorldY() + ((float)tileStepDirectionY + 1.f) * 0.5f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - startPos.y;
	float fwdDistAtNextYCrossing = abs( yDistToFirstYCrossing ) * fwdDistPerYCrossing;

	float fwdDistPerZCrossing = 1 / abs( forwardNormal.z );
	int tileStepDirectionZ = forwardNormal.z < 0 ? -1 : 1;
	float zAtFirstZCrossing = curBlock.GetBlockWorldZ() + ((float)tileStepDirectionZ + 1.f) * 0.5f;
	float zDistToFirstZCrossing = zAtFirstZCrossing - startPos.z;
	float fwdDistAtNextZCrossing = abs( zDistToFirstZCrossing ) * fwdDistPerZCrossing;

	for (;;) {
		// if first hit y(vertical) side
		if (fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing && fwdDistAtNextXCrossing <= fwdDistAtNextZCrossing) {
			// if reach max distance
			if (fwdDistAtNextXCrossing > maxDist) {
				out_rayCastRes.m_didImpact = false;
				out_rayCastRes.m_impactDist = maxDist;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, 0.f );
				out_rayCastRes.m_impactPos = startPos + forwardNormal * maxDist;
				return false;
			}
			// forward step
			if (tileStepDirectionX == -1) {
				curBlock = curBlock.GetWestNeighbor();
			}
			else if (tileStepDirectionX == 1) {
				curBlock = curBlock.GetEastNeighbor();
			}
			// no east/west block
			if (curBlock.m_chunk == nullptr) {
				out_rayCastRes.m_didImpact = false;
				return false;
			}
			// ray cast hit opaque!
			if (curBlock.GetBlock()->IsOpaque()) {
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = fwdDistAtNextXCrossing;
				out_rayCastRes.m_impactNormal = Vec3( -(float)tileStepDirectionX, 0.f, 0.f );
				out_rayCastRes.m_impactPos = forwardNormal * out_rayCastRes.m_impactDist + startPos;
				out_rayCastRes.m_res = curBlock;
				return true;
			}
			// pass this tile
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		// if first hit x(horizontal) side
		else if(fwdDistAtNextYCrossing <= fwdDistAtNextXCrossing && fwdDistAtNextYCrossing <= fwdDistAtNextZCrossing){
			// if reach max distance
			if (fwdDistAtNextYCrossing > maxDist) {
				out_rayCastRes.m_didImpact = false;
				out_rayCastRes.m_impactDist = maxDist;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, 0.f );
				out_rayCastRes.m_impactPos = startPos + forwardNormal * maxDist;
				return false;
			}
			// forward step
			if (tileStepDirectionY == -1) {
				curBlock = curBlock.GetSouthNeighbor();
			}
			else if (tileStepDirectionY == 1) {
				curBlock = curBlock.GetNorthNeighbor();
			}
			// no north/south block
			if (curBlock.m_chunk == nullptr) {
				out_rayCastRes.m_didImpact = false;
				return false;
			}
			// ray cast hit opaque!
			if (curBlock.GetBlock()->IsOpaque()) {
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = fwdDistAtNextYCrossing;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, -(float)tileStepDirectionY, 0.f );
				out_rayCastRes.m_impactPos = forwardNormal * out_rayCastRes.m_impactDist + startPos;
				out_rayCastRes.m_res = curBlock;
				return true;
			}
			// pass this tile
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
		// Z
		else {
			// if reach max distance
			if (fwdDistAtNextZCrossing > maxDist) {
				out_rayCastRes.m_didImpact = false;
				out_rayCastRes.m_impactDist = maxDist;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, 0.f );
				out_rayCastRes.m_impactPos = startPos + forwardNormal * maxDist;
				return false;
			}
			// forward step
			if (tileStepDirectionZ == -1) {
				curBlock = curBlock.GetDownNeighbor();
			}
			else if (tileStepDirectionZ == 1) {
				curBlock = curBlock.GetUpNeighbor();
			}
			// no north/south block
			if (curBlock.m_chunk == nullptr) {
				out_rayCastRes.m_didImpact = false;
				return false;
			}
			// ray cast hit opaque!
			if (curBlock.GetBlock()->IsOpaque()) {
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = fwdDistAtNextZCrossing;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, -(float)tileStepDirectionZ );
				out_rayCastRes.m_impactPos = forwardNormal * out_rayCastRes.m_impactDist + startPos;
				out_rayCastRes.m_res = curBlock;
				return true;
			}
			// pass this tile
			fwdDistAtNextZCrossing += fwdDistPerZCrossing;
		}
	}
}

void World::UpdateTimeAndSkyColor()
{
	if (IsTimeInNight()) {
		m_skyColor = Rgba8( 20, 20, 40 );
	}
	else {
		float timeFraction = GetDayTimeFraction();
		float acutalFraction = 1.f;
		if (timeFraction < 0.5f) {
			acutalFraction = RangeMap( timeFraction, 0.25f, 0.5f, 0.f, 1.f );
		}
		else {
			acutalFraction = RangeMap( timeFraction, 0.75f, 0.5f, 0.f, 1.f );
		}
		m_skyColor = Rgba8::Interpolate( Rgba8( 20, 20, 40 ), Rgba8( 200, 230, 255 ), acutalFraction );
	}

	float lightningPerlin = Compute1dPerlinNoise( m_days, 1.f, 9, 0.5f, 2.f, false );
	m_lightningStrength = RangeMapClamped( lightningPerlin, 0.6f, 0.9f, 0.f, 1.f );
	m_skyColor = Rgba8::Interpolate( m_skyColor, Rgba8::WHITE, m_lightningStrength );
	Rgba8 outDoorLightColor = Rgba8::Interpolate( m_skyColor, Rgba8::WHITE, m_lightningStrength );

	float glowPerlin = Compute1dPerlinNoise( m_days, 1.f, 9, 0.5f, 2.f, false );
	m_glowStrength = RangeMapClamped( glowPerlin, -1.f, 1.f, 0.8f, 1.f );
	Rgba8 inDoorLightColor = Rgba8::WHITE * m_glowStrength;

	m_worldConstans.m_cameraWorldPos = Vec4( m_player->m_position, 1.f );
	outDoorLightColor.GetAsFloats( &m_worldConstans.m_outdoorLightColor.x );
	inDoorLightColor.GetAsFloats( &m_worldConstans.m_indoorLightColor.x );
	m_skyColor.GetAsFloats( &m_worldConstans.m_skyColor.x );
}

void World::DeconstructChunk( Chunk* chunk )
{
	chunk->m_state = ChunkState::DECONSTRUCTING;
	delete chunk;
}

bool operator<( IntVec2 const& a, IntVec2 const& b ) {
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

ChunkGenerateJob::ChunkGenerateJob( Chunk* chunk )
	:SimpleMinerJob(chunk)
{
}

void ChunkGenerateJob::Execute()
{
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATING;
	m_chunk->GenerateBlocks();
	m_chunk->SetSkyLight();
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATE_COMPLETED;
}

SimpleMinerJob::SimpleMinerJob( Chunk* chunk )
	:m_chunk(chunk)
{
	m_chunk->m_state = ChunkState::ACTIVATING_QUEUED_GENERATE;
}

ChunkSaveJob::ChunkSaveJob( Chunk* chunk )
	:SimpleMinerJob(chunk)
{
	m_type = saveLoadWorkerType;
}

void ChunkSaveJob::Execute()
{
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATING;
	m_chunk->SaveToFile();
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATE_COMPLETED;
}

ChunkLoadJob::ChunkLoadJob( Chunk* chunk )
	:SimpleMinerJob(chunk)
{
	m_type = saveLoadWorkerType;
}

void ChunkLoadJob::Execute()
{
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATING;
	m_chunk->ReadFromFile();
	m_chunk->SetSkyLight();
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATE_COMPLETED;
}
