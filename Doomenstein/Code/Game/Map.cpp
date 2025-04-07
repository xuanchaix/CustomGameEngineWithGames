#include "Game/Map.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Image.hpp"

Map::Map( MapDefinition const& mapDef )
	:m_mapDef(&mapDef)
{
	if (mapDef.m_hasDirectionalLight) {
		m_light.SetOrthoView( Vec2(), 
			Vec2( 100.f, 50.f ), 0.f, 100.f );
		m_light.SetTransform( mapDef.m_directionalLightPosition, mapDef.m_directionalLightOrientation );
	}
	else if (mapDef.m_hasPointLight) {
		m_light.SetPerspectiveView( 1.f, 120.f, 0.01f, 100.f );
		m_light.SetTransform( mapDef.m_pointLightPosition, mapDef.m_pointLightOrientation );
	}
}

Map::~Map()
{
	for (int i = 0; i < (int)m_actors.size(); i++) {
		delete m_actors[i];
		m_actors[i] = nullptr;
	}
	delete m_mapImage;
	delete m_terrainSpriteSheet;
	delete m_mapTileIBO;
	delete m_mapTileVBO;
}

void Map::Startup()
{
	m_name = m_mapDef->m_mapName;
	m_mapImage = new Image( m_mapDef->m_imagePath.c_str() );
	m_shader = g_theRenderer->CreateShader( m_mapDef->m_shaderName.c_str(), VertexType::PCUTBN );
	Texture* terrainTexture = g_theRenderer->CreateOrGetTextureFromFile( m_mapDef->m_spriteTexturePath.c_str() );
	m_terrainSpriteSheet = new SpriteSheet( *terrainTexture, m_mapDef->m_spriteSheetCellCount );
	PopulateMap();
	MakeMapVerts();

	SetupActorsToMap();

	m_dirLightConsts.m_ambientIntensity = 0.6f;
	m_dirLightConsts.m_sunIntensity = 0.5f;
	m_dirLightConsts.m_sunDirection = Vec3( 2.f, -1.f, -1.f );

}

void Map::Update()
{
	UpdateAllActors();
	if (!m_blockUpdate) {
		CollideAllActorsWithEachOther();
		CollideAllActorsWithMap();
		DeleteGarbageActors();
	}
	//m_dlc.m_ambientIntensity = GetClamped( m_dlc.m_ambientIntensity, 0.f, 1.f );
	//m_dlc.m_sunIntensity = GetClamped( m_dlc.m_sunIntensity, 0.f, 1.f );
	//m_dlc.m_sunDirection.z = -1.f;
}

void Map::RenderWorld( Camera const& renderCamera ) const
{
	RenderShadowMap( renderCamera );
	RenderTiles( renderCamera );
	RenderAllActors( renderCamera );
}

std::string const& Map::GetTileType( int x, int y ) const
{
	return m_tiles[(size_t)y * m_dimensions.x + x].m_tileDefinition->m_tileType;
}

std::string const& Map::GetTileType( IntVec2 const& mapPos ) const
{
	return GetTileType( mapPos.x, mapPos.y );
}

TileDefinition const& Map::GetTileDef( int x, int y ) const
{
	return *m_tiles[(size_t)y * m_dimensions.x + x].m_tileDefinition;
}

TileDefinition const& Map::GetTileDef( IntVec2 const& mapPos ) const
{
	return GetTileDef( mapPos.x, mapPos.y );
}

TileDefinition const& Map::GetTileDef( std::string const& tileType ) const
{
	return TileDefinition::GetTileDefinition( tileType );
}

TileDefinition const& Map::GetTileDef( Rgba8 const& tileImageColor ) const
{
	return TileDefinition::GetTileDefinition( tileImageColor );
}

int Map::AddActorToMap( Actor* a )
{
	for (int i = 0; i < (int)m_actors.size(); i++) {
		if (m_actors[i] == nullptr) {
			m_actors[i] = a;
			return i;
		}
	}
	m_actors.push_back( a );
	return (int)m_actors.size() - 1;
}

void Map::RemoveActorToMap( Actor* a )
{
	for (int i = 0; i < (int)m_actors.size(); i++) {
		if (m_actors[i] == a) {
			m_actors[i] = nullptr;
			return;
		}
	}
}

ActorUID Map::SpawnActorToMap( ActorDefinition const& actorDef, Vec3 const& postion /*= Vec3( 0.f, 0.f, 0.f )*/, EulerAngles const& orientation /*= EulerAngles() */, Vec3 const& velocity )
{
	Actor* retActor = new Actor( this, actorDef );

	if (retActor) {
		retActor->m_position = postion;
		retActor->m_orientation = orientation;
		retActor->m_velocity = velocity;
		int index = AddActorToMap( retActor );
		retActor->m_uid = ActorUID( index, m_actorSalt );
		retActor->BeginPlay();
		m_actorSalt += 2;
		if (actorDef.m_dieOnSpawn) {
			retActor->Die();
		}
	}
	return retActor->m_uid;
}


ActorUID Map::GetRandomEnemySpawnPoint()
{
	int rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)m_enemySpawnPoints.size() );
	return m_enemySpawnPoints[rnd];
}

void Map::SetupActorsToMap()
{
	// add actor to map by map definition
	for (auto const& spawnInfo : m_mapDef->m_spawnInfos) {
		ActorUID uid = SpawnActorToMap( ActorDefinition::GetActorDefinition( spawnInfo.m_actorName ), spawnInfo.m_position, spawnInfo.m_orientation, spawnInfo.m_velocity );
		if (uid->m_def.m_AIEnabled) {
			AIController* thisAIController = g_theGame->CreateNewAIController( uid->m_def.m_aiBehavior );
			thisAIController->Possess( uid );
		}
		if (spawnInfo.m_actorName == "SpawnPoint") {
			m_playerStart.push_back( uid );
		}
		if (spawnInfo.m_actorName == "EnemySpawnPoint") {
			m_enemySpawnPoints.push_back( uid );
		}
	}
	// spawn player onto map
	if ((int)m_playerStart.size() > 0) {
		for (int i = 0; i < g_theGame->m_numOfPlayers; i++) {
			int rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)m_playerStart.size() );
			ActorUID spawnPointUID = m_playerStart[rnd];
			ActorUID marineID = SpawnActorToMap( ActorDefinition::GetActorDefinition( "Marine" ), spawnPointUID->m_position, spawnPointUID->m_orientation, spawnPointUID->m_velocity );
			g_theGame->GetPlayerController( i )->Possess( marineID );
			m_curPlayerActorIndex = marineID.GetIndex();
		}
	}
	else {
		ERROR_AND_DIE( Stringf( "Error! There is no spawn point in map definition %s", m_mapDef->m_mapName.c_str() ) );
	}
}

Actor* Map::GetActorByUID( ActorUID const uid )
{
	if (uid.IsValid()) {
		return m_actors[uid.GetIndex()];
	}
	return nullptr;
}

Actor* Map::GetNearestVisibleEnemy( ActorFaction enemyFaction, Actor* inquiryActor )
{
	Actor* resActor = nullptr;
	float squaredMinDist = FLT_MAX;
	for (auto actor : m_actors) {
		if (actor && actor->IsAlive() && actor->m_def.m_faction == enemyFaction) {
			Vec3 displacement = actor->m_position - inquiryActor->m_position;
			float length = displacement.GetLength();
			Vec3 normal = displacement / length;
			RayCastResultDoomenstein rayRes = inquiryActor->m_map->RayCastAll( inquiryActor->m_position, normal, inquiryActor->m_def.m_sightRadius, inquiryActor );
			if (DoDiscOverlapDirectedSector2D( actor->m_position, actor->m_physicsRadius, inquiryActor->m_position, inquiryActor->GetForwardNormal(), inquiryActor->m_def.m_sightAngle, inquiryActor->m_def.m_sightRadius )
				&& rayRes.m_didHitActor && rayRes.m_uid == actor->m_uid) {
				float squaredDist = GetDistanceSquared2D( actor->m_position, inquiryActor->m_position );
				if (squaredDist < squaredMinDist) {
					squaredMinDist = squaredDist;
					resActor = actor;
				}
			}
		}
	}
	return resActor;
}

Actor* Map::GetNearestEnemy( ActorFaction enemyFaction, Actor* inquiryActor )
{
	Actor* resActor = nullptr;
	float squaredMinDist = FLT_MAX;
	for (auto actor : m_actors) {
		if (actor && actor->IsAlive() && actor->m_def.m_faction == enemyFaction) {
			float squaredDist = GetDistanceSquared2D( actor->m_position, inquiryActor->m_position );
			if (squaredDist < squaredMinDist) {
				squaredMinDist = squaredDist;
				resActor = actor;
			}
		}
	}
	return resActor;
}

Actor* Map::GetNearestEnemyInSector( ActorFaction enemyFaction, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	Actor* resActor = nullptr;
	float squaredMinDist = FLT_MAX;
	for (auto actor : m_actors) {
		if (actor && actor->IsAlive() && actor->m_def.m_faction == enemyFaction) {
			if (DoDiscOverlapDirectedSector2D( actor->m_position, actor->m_physicsRadius, sectorTip, sectorForwardNormal, sectorApertureDegrees, sectorRadius )) {
				float squaredDist = GetDistanceSquared2D( actor->m_position, sectorTip );
				if (squaredDist < squaredMinDist) {
					squaredMinDist = squaredDist;
					resActor = actor;
				}
			}
		}
	}
	return resActor;
}

void Map::PopulateHeatMapDistanceField( TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost /*= FLT_MAX*/, float minCost /*= 0.f */ ) const
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
				&& !IsCoordInBounds( nextTile )) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.y >= 1) {
			IntVec2 nextTile = thisTile + IntVec2( 0, -1 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost
				&& !IsCoordInBounds( nextTile )) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.x < m_dimensions.x - 1) {
			IntVec2 nextTile = thisTile + IntVec2( 1, 0 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost
				&& !IsCoordInBounds( nextTile )) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		if (thisTile.y < m_dimensions.y - 1) {
			IntVec2 nextTile = thisTile + IntVec2( 0, 1 );
			if (out_distanceField.GetTileValue( nextTile ) == maxCost && out_distanceField.GetTileValue( nextTile ) != minCost
				&& !IsCoordInBounds( nextTile )) {
				queue[end] = nextTile;
				end = (end + 1);//% maxQueueLength;
				out_distanceField.SetTileValue( nextTile, thisTileValue + 1.f );
			}
		}
		start++;//= (start + 1) % maxQueueLength;
	}
	delete[] queue;
}

void Map::DebugPossessNext()
{
	if (g_theGame->m_numOfPlayers == 1) {
		do {
			m_curPlayerActorIndex = (m_curPlayerActorIndex + 1) % (int)m_actors.size();
		} while (m_actors[m_curPlayerActorIndex] == nullptr || !m_actors[m_curPlayerActorIndex]->m_def.m_canBePossessed);
		g_theGame->GetPlayerController( 0 )->Possess( m_actors[m_curPlayerActorIndex]->m_uid );
	}
}

void Map::DebugKillAllExceptSelf( ActorUID self )
{
	for (auto actor : m_actors) {
		if (actor && !(actor->m_uid == self)) {
			actor->Die();
		}
	}
}

bool Map::IsPositionInBounds( Vec3 const& position, float const tolerance /*= 0.f */ ) const
{
	UNUSED( tolerance );
	IntVec2 coords = GetMapPosFromWorldPos( position );
	return IsCoordInBounds( coords );
}

bool Map::IsCoordInBounds( IntVec2 const& coords ) const
{
	if (coords.x >= 0 && coords.x < m_dimensions.x && coords.y >= 0 && coords.y < m_dimensions.y) {
		return GetTile( coords ).m_tileDefinition->m_isSolid;
	}
	else {
		return false;
	}
}

bool Map::IsCoordInBounds( int x, int y ) const
{
	if (x >= 0 && x < m_dimensions.x && y >= 0 && y < m_dimensions.y) {
		return GetTile( x, y ).m_tileDefinition->m_isSolid;
	}
	else {
		return false;
	}
}

Tile const& Map::GetTile( IntVec2 const& coords ) const
{
	return m_tiles[(size_t)coords.x + (size_t)coords.y * m_dimensions.x];
}

Tile const& Map::GetTile( int x, int y ) const
{
	return m_tiles[(size_t)x + (size_t)y * m_dimensions.x];
}

IntVec2 const Map::GetMapPosFromWorldPos( Vec3 worldPos ) const
{
	return IntVec2( RoundDownToInt( worldPos.x ), RoundDownToInt( worldPos.y ) );
}

void Map::PopulateMap()
{
	m_dimensions = m_mapImage->GetDimensions();
	for (int j = 0; j < m_dimensions.y; j++) {
		for (int i = 0; i < m_dimensions.x; i++) {
			IntVec2 coords = IntVec2( i, j );
			TileDefinition const& thisDef = TileDefinition::GetTileDefinition( m_mapImage->GetTexelColor( coords ) );
			m_tiles.push_back( Tile( thisDef, coords ) );
		}
	}
	m_tileHeatMap = new TileHeatMap( m_dimensions );
	m_tileHeatMap->SetAllValues( 0.f );
	for (int i = 0; i < m_dimensions.x; i++) {
		for (int j = 0; j < m_dimensions.y; j++) {
			if (IsCoordInBounds( i, j )) {
				m_tileHeatMap->SetTileValue( IntVec2( i, j ), FLT_MAX );
			}
		}
	}
}

void Map::MakeMapVerts()
{
	m_vertices.reserve( 10000 );
	m_indexes.reserve( 10000 );
	for (int j = 0; j < m_dimensions.y; j++) {
		for (int i = 0; i < m_dimensions.x; i++) {
			IntVec2 coords = IntVec2( i, j );
			TileDefinition const& thisDef = GetTileDef( coords );
			if (thisDef.m_isSolid) {
				IntVec2 uvCoords = thisDef.m_coordsInTextureForWall;
				int uvIndex = uvCoords.x + uvCoords.y * m_mapDef->m_spriteSheetCellCount.x;
				AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)i, (float)j, 0.f ), Vec3( (float)(i + 1), (float)j, 0.f ), Vec3( (float)(i + 1), (float)j, 1.f ), Vec3( (float)i, (float)j, 1.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndex ) );
				AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)i, (float)(j + 1), 0.f ), Vec3( (float)i, (float)j, 0.f ), Vec3( (float)i, (float)j, 1.f ), Vec3( (float)i, (float)(j + 1), 1.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndex ) );
				AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)(i + 1), (float)j, 0.f ), Vec3( (float)(i + 1), (float)(j + 1), 0.f ), Vec3( (float)(i + 1), (float)(j + 1), 1.f ), Vec3( (float)(i + 1), (float)j, 1.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndex ) );
				AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)(i + 1), (float)(j + 1), 0.f ), Vec3( (float)i, (float)(j + 1), 0.f ), Vec3( (float)i, (float)(j + 1), 1.f ), Vec3( (float)(i + 1), (float)(j + 1), 1.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndex ) );

				IntVec2 uvCoordsForCeiling = thisDef.m_coordsInTextureForCeiling;
				int uvIndexForCeiling = uvCoordsForCeiling.x + uvCoordsForCeiling.y * m_mapDef->m_spriteSheetCellCount.x;
				AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)(i + 1), (float)(j + 1), 1.f ), Vec3( (float)i, (float)(j + 1), 1.f ), Vec3( (float)i, (float)j, 1.f ), Vec3( (float)(i + 1), (float)j, 1.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndexForCeiling ) );
			}
			else {
				//IntVec2 uvCoordsForCeiling = thisDef.m_coordsInTextureForCeiling;
				//int uvIndexForCeiling = uvCoordsForCeiling.x + uvCoordsForCeiling.y * m_mapDef->m_spriteSheetCellCount.x;
				IntVec2 uvCoordsForFloor = thisDef.m_coordsInTextureForFloor;
				int uvIndexForFloor = uvCoordsForFloor.x + uvCoordsForFloor.y * m_mapDef->m_spriteSheetCellCount.x;
				//AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)i, (float)j, 1.f ), Vec3( (float)i, (float)(j + 1), 1.f ), Vec3( (float)(i + 1), (float)(j + 1), 1.f ), Vec3( (float)(i + 1), (float)j, 1.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndexForCeiling ) );
				AddVertsForQuad3D( m_vertices, m_indexes, Vec3( (float)i, (float)(j + 1), 0.f ), Vec3( (float)i, (float)j, 0.f ), Vec3( (float)(i + 1), (float)j, 0.f ), Vec3( (float)(i + 1), (float)(j + 1), 0.f ), thisDef.m_tintColor, m_terrainSpriteSheet->GetSpriteUVs( uvIndexForFloor ) );
			}
		}
	}

	m_mapTileVBO = g_theRenderer->CreateVertexBuffer( m_vertices.size(), sizeof( Vertex_PCUTBN ) );
	g_theRenderer->CopyCPUToGPU( m_vertices.data(), m_vertices.size() * sizeof( Vertex_PCUTBN ), m_mapTileVBO );
	m_mapTileIBO = g_theRenderer->CreateIndexBuffer( m_indexes.size() );
	g_theRenderer->CopyCPUToGPU( m_indexes.data(), m_indexes.size() * sizeof( int ), m_mapTileIBO );

}

void Map::UpdateAllActors()
{
	for (int i = 0; i < (int)m_actors.size(); i++) {
		Actor* actor = m_actors[i];
		if (actor && !m_blockUpdate) {
			actor->Update();
		}
	}
}

void Map::DeleteGarbageActors()
{
	for (auto& actor : m_actors) {
		if (actor && actor->m_isGarbage) {
			delete actor;
			actor = nullptr;
		}
	}
}

void Map::RenderShadowMap( Camera const& renderCamera ) const
{
	//Camera directionalLight;
	//directionalLight.SetPerspectiveView( 1.f, 90.f, 0.01f, 100.f );
	//directionalLight.SetTransform( Vec3( 28.f, 14.f, 0.5f ), EulerAngles( 0.f, 0.f, 90.f ) );

	DirectionalLightConstants normalizeDlc = m_dirLightConsts;
	normalizeDlc.m_sunDirection.Normalize();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( m_shader );
	g_theRenderer->BindTexture( &m_terrainSpriteSheet->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::POINT_SHADOW );
	//g_theRenderer->SetDirectionalLightConstants( normalizeDlc );
	g_theRenderer->SetLightConstants( m_light.m_position, m_mapDef->m_lightAmbient, m_light.GetViewMatrix(), m_light.GetProjectionMatrix() );
	g_theRenderer->RenderShadowMap( m_mapTileVBO, m_mapTileIBO, (int)m_mapTileIBO->GetSize() / sizeof( int ), 0 );
	//g_theRenderer->DrawVertexArray( m_vertices, m_indexes );
	//g_theRenderer->DrawVertexIndexed( m_mapTileVBO, m_mapTileIBO, (int)m_mapTileIBO->GetSize() / sizeof( int ) );

	for (auto actor : m_actors) {
		if (actor) {
			actor->Render( &renderCamera, true );
		}
	}
}

void Map::RenderAllActors( Camera const& renderCamera ) const
{
	for (auto actor : m_actors) {
		if (actor) {
			actor->Render( &renderCamera );
		}
	}
}

void Map::RenderTiles( Camera const& renderCamera ) const
{
	UNUSED( renderCamera );
	Camera directionalLight;
	directionalLight.SetPerspectiveView( 2.f, 60.f, 0.1f, 100.f );
	directionalLight.SetTransform( Vec3( 28.f, 15.f, 0.f ), EulerAngles( 0.f, 0.f, 0.f ) );

	DirectionalLightConstants normalizeDlc = m_dirLightConsts;
	normalizeDlc.m_sunDirection.Normalize();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( m_shader );
	g_theRenderer->BindTexture( &m_terrainSpriteSheet->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::POINT_SHADOW );
	//g_theRenderer->SetDirectionalLightConstants( normalizeDlc );
	//g_theRenderer->SetLightConstants( directionalLight.m_position, 0.8f, directionalLight.GetViewMatrix(), directionalLight.GetProjectionMatrix() );

	//g_theRenderer->DrawVertexArray( m_vertices, m_indexes );
	g_theRenderer->DrawVertexIndexed( m_mapTileVBO, m_mapTileIBO, (int)m_mapTileIBO->GetSize() / sizeof(int) );
}

void Map::RenderUI() const
{

}

void Map::CollideAllActorsWithEachOther()
{
	for (int i = 0; i < (int)m_actors.size(); i++) {
		if (!m_actors[i] || !m_actors[i]->IsAlive() || m_actors[i]->m_physicsRadius == 0.f) {
			continue;
		}
		for (int j = i + 1; j < (int)m_actors.size(); j++) {
			if (!m_actors[j] || !m_actors[j]->IsAlive() || m_actors[i]->m_physicsRadius == 0.f) {
				continue;
			}
			CollideTwoActors( m_actors[i], m_actors[j] );
		}
	}
}

void Map::CollideTwoActors( Actor* a, Actor* b )
{
	// actor do not collide with their owner
	if (a->m_owner == b->m_uid || b->m_owner == a->m_uid) {
		return;
	}
	//if (!DoDiscsOverlap( a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius )) {
	//	return;
	//}
	if (!DoZCylindersOverlap3D( a->m_position, a->m_physicsRadius, a->m_position.z, a->m_position.z + a->m_height, b->m_position, b->m_physicsRadius, b->m_position.z, b->m_position.z + b->m_height )) {
		return;
	}
	if (a->m_def.m_dieOnCollide) {
		a->Die();
		bool isLethal = false;
		b->BeAttacked( g_theGame->m_randNumGen->RollRandomFloatInRange( a->m_def.m_damageOnColiide ), isLethal );
		if (b->m_controller) {
			b->m_controller->Damagedby( a->m_owner, isLethal );
		}
		b->AddImpulse( a->m_velocity.GetNormalized() * a->m_def.m_impulseOnCollide );
#ifdef DEBUG_SHOW_HIT
		DebugAddWorldWireSphere( a->m_position + Vec3( 0.f, 0.f, a->m_def.m_physicsHeight * 0.5f ), 0.06f, 5.f, Rgba8::WHITE, Rgba8::WHITE );
		DebugAddWorldArrow( a->m_position + Vec3( 0.f, 0.f, a->m_def.m_physicsHeight * 0.5f ), a->m_position + Vec3( 0.f, 0.f, a->m_def.m_physicsHeight * 0.5f ) - a->GetForwardNormal() * 0.1f, 0.01f, 5.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ) );
#endif
		return;
	}
	if (b->m_def.m_dieOnCollide) {
		b->Die();
		bool isLethal = false;
		a->BeAttacked( g_theGame->m_randNumGen->RollRandomFloatInRange( b->m_def.m_damageOnColiide ), isLethal );
		if (a->m_controller) {
			a->m_controller->Damagedby( b->m_owner, isLethal );
		}
		a->AddImpulse( b->m_velocity.GetNormalized() * b->m_def.m_impulseOnCollide );
#ifdef DEBUG_SHOW_HIT
		DebugAddWorldWireSphere( b->m_position + Vec3( 0.f, 0.f, b->m_def.m_physicsHeight * 0.5f ), 0.06f, 5.f, Rgba8::WHITE, Rgba8::WHITE );
		DebugAddWorldArrow( b->m_position + Vec3( 0.f, 0.f, b->m_def.m_physicsHeight * 0.5f ), b->m_position + Vec3( 0.f, 0.f, b->m_def.m_physicsHeight * 0.5f ) - b->GetForwardNormal() * 0.1f, 0.01f, 5.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ) );
#endif
		return;
	}
	if (!a->IsAlive() || !b->IsAlive()) {
		return;
	}

	if (a->m_def.m_collidesWithActors && b->m_def.m_collidesWithActors) {
		Vec2 aNewPos = a->m_position;
		Vec2 bNewPos = b->m_position;
		PushDiscsOutOfEachOther2D( aNewPos, a->m_physicsRadius, bNewPos, b->m_physicsRadius );
		a->m_position.x = aNewPos.x;
		a->m_position.y = aNewPos.y;
		b->m_position.x = bNewPos.x;
		b->m_position.y = bNewPos.y;
	}
	else if (a->m_def.m_collidesWithActors && !b->m_def.m_collidesWithActors) {
		Vec2 aNewPos = a->m_position;
		PushDiscOutOfFixedDisc2D( aNewPos, a->m_physicsRadius, b->m_position, b->m_physicsRadius );
		a->m_position.x = aNewPos.x;
		a->m_position.y = aNewPos.y;
	}
	else if (!a->m_def.m_collidesWithActors && b->m_def.m_collidesWithActors) {
		Vec2 bNewPos = b->m_position;
		PushDiscOutOfFixedDisc2D( bNewPos, b->m_physicsRadius, a->m_position, a->m_physicsRadius );
		b->m_position.x = bNewPos.x;
		b->m_position.y = bNewPos.y;
	}
}

void Map::CollideAllActorsWithMap()
{
	for (auto actor : m_actors) {
		if (actor && actor->IsAlive() && actor->m_def.m_collidesWithWorld) {
			CollideActorWithMap( actor );
		}
	}
}

void Map::CollideActorWithMap( Actor* a )
{
	IntVec2 mapPos = GetMapPosFromWorldPos( a->m_position );
	CollideActorWithBlock( a, mapPos + IntVec2( 1, 0 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( -1, 0 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( 0, 1 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( 0, -1 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( 1, 1 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( -1, 1 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( 1, -1 ) );
	CollideActorWithBlock( a, mapPos + IntVec2( -1, -1 ) );
	if (a->m_position.z < 0.f) {
		a->m_position.z = 0.f;
		if (a->m_def.m_dieOnCollide) {
			a->Die();
#ifdef DEBUG_SHOW_HIT
			DebugAddWorldWireSphere( a->m_position, 0.06f, 5.f, Rgba8::WHITE, Rgba8::WHITE );
			DebugAddWorldArrow( a->m_position, a->m_position - a->GetForwardNormal() * 0.1f, 0.01f, 5.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ) );
#endif
			return;
		}
	}
	if (a->m_position.z + a->m_height > 1.f) {
		a->m_position.z = 1.f - a->m_height;
		if (a->m_def.m_dieOnCollide) {
			a->Die();
#ifdef DEBUG_SHOW_HIT
			DebugAddWorldWireSphere( a->m_position, 0.06f, 5.f, Rgba8::WHITE, Rgba8::WHITE );
			DebugAddWorldArrow( a->m_position, a->m_position - a->GetForwardNormal() * 0.1f, 0.01f, 5.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ) );
#endif
			return;
		}
	}
}

void Map::CollideActorWithBlock( Actor* a, IntVec2 const& coords )
{
	if (!IsCoordInBounds( coords )) {
		return;
	}
	Vec2 aNewPos = a->m_position;

	if (PushDiscOutOfFixedAABB2D( aNewPos, a->m_physicsRadius, AABB2( Vec2( (float)coords.x, (float)coords.y ), Vec2( (float)(coords.x + 1), (float)(coords.y + 1) ) ) )) {
		a->m_position.x = aNewPos.x;
		a->m_position.y = aNewPos.y;
		if (a->m_def.m_dieOnCollide) {
			a->Die();
#ifdef DEBUG_SHOW_HIT
			// ToDo:
			Vec2 ca = Vec2( a->m_position ) - Vec2( (float)coords.x + 0.5f, (float)coords.y + 0.5f );
			Vec3 impactPosition, impactNormal;
			if (ca.y >= ca.x && ca.y >= -ca.x) {
				impactPosition = Vec3( a->m_position.x, (float)coords.y + 1.f, a->m_position.z );
				impactNormal = Vec3( 0.f, 1.f, 0.f );
			}
			else if (ca.y <= ca.x && ca.y >= -ca.x) {
				impactPosition = Vec3( (float)coords.x + 1.f, a->m_position.y, a->m_position.z );
				impactNormal = Vec3( 1.f, 0.f, 0.f );
			}
			else if (ca.y <= ca.x && ca.y <= -ca.x) {
				impactPosition = Vec3( a->m_position.x, (float)coords.y, a->m_position.z );
				impactNormal = Vec3( 0.f, -1.f, 0.f );
			}
			else {
				impactPosition = Vec3( (float)coords.x, a->m_position.y, a->m_position.z );
				impactNormal = Vec3( -1.f, 0.f, 0.f );
			}
			DebugAddWorldWireSphere( impactPosition, 0.06f, 5.f, Rgba8::WHITE, Rgba8::WHITE );
			DebugAddWorldArrow( impactPosition, impactPosition + impactNormal * 0.1f, 0.01f, 5.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ) );
#endif
			return;
		}
	}
}

RayCastResultDoomenstein const Map::RayCastAll( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Actor* ignoreActor, ActorFaction ignoreFaction ) const
{
	RayCastResult3D finalRes;
	finalRes.m_impactDist = FLT_MAX;
	RayCastResult3D curRes;
	RayCastResultDoomenstein curResD;

	if (startPos.z > 1.f || startPos.z < 0.f) {
		return curResD;
	}

	curRes = RayCastWorldZ( startPos, forwardNormal, maxDist );
	if (curRes.m_didImpact && curRes.m_impactDist < finalRes.m_impactDist) {
		finalRes = curRes;
	}

	curRes = RayCastWorldXY( startPos, forwardNormal, maxDist );
	if (curRes.m_didImpact && curRes.m_impactDist < finalRes.m_impactDist) {
		finalRes = curRes;
	}


	curResD = RayCastWorldAllActors( startPos, forwardNormal, maxDist, ignoreActor, ignoreFaction );
	if (curResD.m_didImpact && curResD.m_impactDist < finalRes.m_impactDist) {
		return curResD;
	}
	else {
		curResD.m_didImpact = finalRes.m_didImpact;
		curResD.m_didHitActor = false;
		curResD.m_impactDist = finalRes.m_impactDist;
		curResD.m_impactNormal = finalRes.m_impactNormal;
		curResD.m_impactPos = finalRes.m_impactPos;
		curResD.m_rayForwardNormal = finalRes.m_rayForwardNormal;
		curResD.m_rayMaxLength = finalRes.m_rayMaxLength;
		curResD.m_rayStartPos = finalRes.m_rayStartPos;
		return curResD;
	}
}

RayCastResult3D const Map::RayCastWorldXY( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist ) const
{
	RayCastResult2D res2D;
	RayCastResult3D res3D;
	float forwardLengthXY = Vec2( forwardNormal ).GetLength();
	Vec2 forwardNormal2D = Vec2( forwardNormal ) / forwardLengthXY;
	float distToBound = 0.f;
	Vec3 startPosDisplacement = Vec3();
	if (startPos.z >= 1.f || startPos.z <= 0.f) {
		if (startPos.z >= 1.f) {
			if (forwardNormal.z >= 0.f) {
				return res3D;
			}
			distToBound = -(startPos.z - 1.f) / forwardNormal.z;
		}
		else {
			if (forwardNormal.z <= 0.f) {
				return res3D;
			}
			distToBound = -startPos.z / forwardNormal.z;
		}
		startPosDisplacement = distToBound * forwardNormal;
		IntVec2 mapPos = GetMapPosFromWorldPos( startPos + startPosDisplacement );
		if (IsCoordInBounds( mapPos )) {
			float distToMinX = -(startPos.x + startPosDisplacement.x - (float)mapPos.x) / forwardNormal.x > 0.f
				? distToMinX = -(startPos.x + startPosDisplacement.x - (float)mapPos.x) / forwardNormal.x + 0.0001f : FLT_MAX;
			float distToMinY = -(startPos.y + startPosDisplacement.y - (float)mapPos.y) / forwardNormal.y > 0.f
				? distToMinY = -(startPos.y + startPosDisplacement.y - (float)mapPos.y) / forwardNormal.y + 0.0001f : FLT_MAX;
			float distToMaxX = -(startPos.x + startPosDisplacement.x - (float)(mapPos.x + 1)) / forwardNormal.x > 0.f
				? distToMaxX = -(startPos.x + startPosDisplacement.x - (float)(mapPos.x + 1)) / forwardNormal.x + 0.0001f : FLT_MAX;
			float distToMaxY = -(startPos.y + startPosDisplacement.y - (float)(mapPos.y + 1)) / forwardNormal.y > 0.f
				? -(startPos.y + startPosDisplacement.y - (float)(mapPos.y + 1)) / forwardNormal.y + 0.0001f : FLT_MAX;
			if (distToMinX == Minf( distToMinX, Minf( distToMinY, Minf( distToMaxX, distToMaxY ) ) )) {
				distToBound += distToMinX;
				startPosDisplacement += distToMinX * forwardNormal;
			}
			else if (distToMinY == Minf( distToMinX, Minf( distToMinY, Minf( distToMaxX, distToMaxY ) ) )) {
				distToBound += distToMinY;
				startPosDisplacement += distToMinY * forwardNormal;
			}
			else if (distToMaxX == Minf( distToMinX, Minf( distToMinY, Minf( distToMaxX, distToMaxY ) ) )) {
				distToBound += distToMaxX;
				startPosDisplacement += distToMaxX * forwardNormal;
			}
			else if (distToMaxY == Minf( distToMinX, Minf( distToMinY, Minf( distToMaxX, distToMaxY ) ) )) {
				distToBound += distToMaxY;
				startPosDisplacement += distToMaxY * forwardNormal;
			}
			else {
				return res3D;
			}
		}
	}

	m_tileHeatMap->RayCastVsGrid2D( res2D, startPos + startPosDisplacement, forwardNormal2D, (maxDist - distToBound) * forwardLengthXY, FLT_MAX );
	res3D.m_didImpact = res2D.m_didImpact;
	if (!res3D.m_didImpact) {
		return res3D;
	}
	res3D.m_impactDist = res2D.m_impactDist / forwardLengthXY + distToBound;
	if (res3D.m_impactDist == 0.f) {
		res3D.m_impactPos = startPos;
		res3D.m_impactNormal = -forwardNormal;
		res3D.m_rayForwardNormal = forwardNormal;
		res3D.m_rayMaxLength = maxDist;
		res3D.m_rayStartPos = startPos;
		return res3D;
	}
	res3D.m_impactPos = startPos + forwardNormal * res3D.m_impactDist;
	res3D.m_impactNormal = res2D.m_impactNormal;
	res3D.m_rayForwardNormal = forwardNormal;
	res3D.m_rayMaxLength = maxDist;
	res3D.m_rayStartPos = startPos;
	return res3D;
}

void Map::DealRangeDamage( Vec3 const& position, float range, FloatRange const& damage, ActorUID damageSource, bool isDamping /*= true */ )
{
	for (auto actor : m_actors) {
		if (actor && actor->m_def.m_canBePossessed) {
			Vec3 nearestPos = GetNearestPointOnZCylinder3D( position, actor->m_position, actor->m_physicsRadius, 0.f, actor->m_height );
			float distanceSquared = GetDistance3D( nearestPos, position );
			if (distanceSquared >= range * range) {
				continue;
			}
			else {
				if (isDamping) {
					float distance = sqrtf( distanceSquared );
					float reducedDamage = g_theGame->m_randNumGen->RollRandomFloatInRange( damage ) * (range -distance) / range;
					bool isLethal = false;
					actor->BeAttacked( reducedDamage, isLethal );
					if (actor->m_controller) {
						actor->m_controller->Damagedby( damageSource, isLethal );
					}
				}
				else {
					bool isLethal = false;
					actor->BeAttacked( g_theGame->m_randNumGen->RollRandomFloatInRange( damage ), isLethal );
					if (actor->m_controller) {
						actor->m_controller->Damagedby( damageSource, isLethal );
					}
				}
			}
		}
	}
}

RayCastResult3D const Map::RayCastWorldZ( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist ) const
{
	RayCastResult3D res;
	if (forwardNormal.z > 0.f && startPos.z < 1.f) {
		float travelLength = (1.f - startPos.z) / forwardNormal.z;
		if (travelLength > maxDist) {
			return res;
		}
		res.m_didImpact = true;
		res.m_impactDist = travelLength;
		res.m_impactNormal = Vec3( 0, 0, -1.f );
		res.m_impactPos = startPos + forwardNormal * travelLength;
		res.m_rayForwardNormal = forwardNormal;
		res.m_rayMaxLength = maxDist;
		res.m_rayStartPos = startPos;
	}
	else if(forwardNormal.z < 0.f && startPos.z > 0.f) {
		float travelLength = -startPos.z / forwardNormal.z;
		if (travelLength > maxDist) {
			return res;
		}
		res.m_didImpact = true;
		res.m_impactDist = travelLength;
		res.m_impactNormal = Vec3( 0, 0, 1.f );
		res.m_impactPos = startPos + forwardNormal * travelLength;
		res.m_rayForwardNormal = forwardNormal;
		res.m_rayMaxLength = maxDist;
		res.m_rayStartPos = startPos;
	}
	return res;
}

RayCastResultDoomenstein const Map::RayCastWorldAllActors( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Actor* ignoreActor, ActorFaction ignoreFaction ) const
{
	RayCastResultDoomenstein res;
	res.m_impactDist = FLT_MAX;
	RayCastResultDoomenstein curRes;
	for (auto const actor : m_actors) {
		// if hit
		if (actor && actor->IsAlive() && actor != ignoreActor && (ignoreFaction == ActorFaction::NONE || ignoreFaction != actor->m_def.m_faction) &&
			RayCastVsCylinderZ3D( curRes, startPos, forwardNormal, maxDist, actor->m_position, actor->m_position.z, actor->m_position.z + actor->m_height, actor->m_physicsRadius )) {
			if (curRes.m_impactDist < res.m_impactDist) {
				res = curRes;
				res.m_didHitActor = true;
				res.m_uid = actor->m_uid;
			}
		}
	}
	return res;
}
