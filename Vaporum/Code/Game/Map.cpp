#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Unit.hpp"
#include "Game/Effect.hpp"

std::vector<MapDefinition> MapDefinition::s_definitions;

MapDefinition::MapDefinition( XmlElement* elem )
{
	m_name = ParseXmlAttribute( *elem, "name", m_name );
	m_overlayShader = ParseXmlAttribute( *elem, "overlayShader", m_overlayShader );
	m_gridSize = ParseXmlAttribute( *elem, "gridSize", m_gridSize );
	m_worldBoundsMin = ParseXmlAttribute( *elem, "worldBoundsMin", m_worldBoundsMin );
	m_worldBoundsMax = ParseXmlAttribute( *elem, "worldBoundsMax", m_worldBoundsMax );

	XmlElement* tileElem = elem->FirstChildElement( "Tiles" );
	if (tileElem) {
		m_tiles = tileElem->GetText();
	}
	XmlElement* unitElem = elem->FirstChildElement( "Units" );
	if (unitElem) {
		m_unitLayout1 = unitElem->GetText();
	}
	unitElem = unitElem->NextSiblingElement( "Units" );
	if (unitElem) {
		m_unitLayout2 = unitElem->GetText();
	}
}

MapDefinition const& MapDefinition::GetDefinition( std::string const& defName )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == defName) {
			return def;
		}
	}
	g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "Do not have map name %s", defName.c_str() ) );
	return *((MapDefinition*)(nullptr));
	//ERROR_AND_DIE( Stringf( "Error! No such map definition named %s", defName.c_str() ) );
}

Map::Map( MapDefinition const& def )
	:m_def(def)
{
	
}

Map::~Map()
{
	if (m_gameState != GameState::Win && m_isOnlineNetworkingGame) {
		g_theNetSystem->Send( "PlayerQuit" );
	}
	delete m_tileHexVertexBuffer;
	delete m_currentHoveringHexBuffer;
	for (auto& pair : m_tiles) {
		delete pair.second;
	}
	for (auto unit : m_units) {
		delete unit;
	}
	delete m_moonSurfaceMat;
	delete m_lightConstantBuffer;
	delete m_tileHexIndexBuffer;
	delete m_currentHoveringHexIndexBuffer;
}

void Map::StartUp()
{
	m_cameraBounds = AABB3( m_def.m_worldBoundsMin, m_def.m_worldBoundsMax );
	m_mapBounds = AABB2( m_def.m_worldBoundsMin, m_def.m_worldBoundsMax );
	Strings rows;
	SplitStringOnDelimiter( rows, m_def.m_tiles.substr( 1, m_def.m_tiles.size() - 2 ), '\n' );
	int y = 0;
	for (int i = (int)rows.size() - 1; i >= 0; i--) {
		Strings lines;
		SplitStringOnDelimiter( lines, rows[i], ' ' );
		for (int x = 0; x < m_def.m_gridSize.x; ++x) {
			char symbol = lines[x][0];
			if (IsPointInsideAABB2D( Vec2( (float)x * 0.86602540378f, (float)y + (float)x * 0.5f ), m_mapBounds )) {
				m_tiles[IntVec2( x, y )] = new Tile( IntVec2( x, y ), TileDefinition::GetDefinition( symbol ) );
			}
		}
		++y;
	}

	//rows.clear();
	SplitStringOnDelimiter( rows, m_def.m_unitLayout1.substr( 1, m_def.m_unitLayout1.size() - 2 ), '\n' );
	y = 0;
	for (int i = (int)rows.size() - 1; i >= 0; i--) {
		Strings lines;
		SplitStringOnDelimiter( lines, rows[i], ' ' );
		for (int x = 0; x < m_def.m_gridSize.x; ++x) {
			char symbol = lines[x][0];
			if (symbol != '.') {
				Unit* newUnit = new Unit( g_theGame, UnitDefinition::GetDefinition( symbol ) );
				Tile* tileOn = m_tiles[IntVec2( x, y )];
				m_units.push_back( newUnit );
				m_player1AvailableUnits.push_back( newUnit );
				newUnit->m_position = tileOn->GetUnitPosition();
				newUnit->m_faction = 1;
				newUnit->m_tileOn = tileOn;
				newUnit->m_targetDir = newUnit->m_dir;
				tileOn->m_unitOnTile = newUnit;
			}
		}
		++y;
	}

	SplitStringOnDelimiter( rows, m_def.m_unitLayout2.substr( 1, m_def.m_unitLayout2.size() - 2 ), '\n' );
	y = 0;
	for (int i = (int)rows.size() - 1; i >= 0; i--) {
		Strings lines;
		SplitStringOnDelimiter( lines, rows[i], ' ' );
		for (int x = 0; x < m_def.m_gridSize.x; ++x) {
			char symbol = lines[x][0];
			if (symbol != '.') {
				Unit* newUnit = new Unit( g_theGame, UnitDefinition::GetDefinition( symbol ) );
				Tile* tileOn = m_tiles[IntVec2( x, y )];
				m_units.push_back( newUnit );
				m_player2AvailableUnits.push_back( newUnit );
				newUnit->m_position = tileOn->GetUnitPosition();
				newUnit->m_faction = 2;
				newUnit->m_dir = UnitDirection::Left;
				newUnit->m_targetDir = newUnit->m_dir;
				newUnit->m_orientation.m_yawDegrees = 180.f;
				newUnit->m_tileOn = tileOn;
				tileOn->m_unitOnTile = newUnit;
			}
		}
		++y;
	}

	for (auto& pair : m_tiles) {
		pair.second->InitializeNeighbors();
	}

	std::vector<Vertex_PCU> verts;
	std::vector<unsigned int> indexes;
	for(auto& pair: m_tiles){
		if (pair.second->m_def.m_isBlocked) {
			AddVertsForSolidHexagon( verts, indexes, pair.second->m_center, 0.5f );
		}
		else {
			AddVertsForHexagon( verts, indexes, pair.second->m_center, 0.5f );
		}
	}
	m_tileHexVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_tileHexVertexBuffer );
	m_tileHexIndexBuffer = g_theRenderer->CreateIndexBuffer( sizeof( unsigned int ) * indexes.size() );
	g_theRenderer->CopyCPUToGPU( indexes.data(), sizeof( unsigned int ) * indexes.size(), m_tileHexIndexBuffer );
	verts.clear();
	indexes.clear();

	AddVertsForHexagon( verts, indexes, Vec2(), 0.4f, 0.08f, Rgba8::WHITE );
	m_currentHoveringHexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_currentHoveringHexBuffer );
	m_currentHoveringHexIndexBuffer = g_theRenderer->CreateIndexBuffer( sizeof( unsigned int ) * indexes.size() );
	g_theRenderer->CopyCPUToGPU( indexes.data(), sizeof( unsigned int ) * indexes.size(), m_currentHoveringHexIndexBuffer );

	m_dlc.m_ambientIntensity = 0.2f;
	m_dlc.m_sunDirection = Vec3( 0.6666667f, -0.6666667f, -0.3333333f );
	m_dlc.m_sunIntensity = 0.8f;
	m_dlc.m_worldEyePosition = g_theGame->m_playerPOV.m_position;
	m_lightConstantBuffer = g_theRenderer->CreateConstantBuffer( sizeof( DirectionalLightConstants ) );
	g_theRenderer->CopyCPUToGPU( &m_dlc, sizeof( DirectionalLightConstants ), m_lightConstantBuffer );

	m_moonSurfaceMat = new Material( "Data/Materials/Moon.xml", g_theRenderer );

	AddVertsForQuad3D( m_tbnVerts, m_tbnInts, Vec2( -19.55f, -8.4f ), Vec2( 29.05f, -8.4f ), Vec2( 29.05f, 24.9f ), Vec2( -19.55f, 24.9f ), Rgba8::WHITE );
	CalculateTangentSpaceBasisVectors( m_tbnVerts, m_tbnInts, false, true );

	if (m_isOnlineNetworkingGame) {
		m_gameState = GameState::WaitingForTheOtherPlayer;
		if (g_theNetSystem->IsServer()) {
			m_networkPlayer = 1;
		}
		else {
			m_networkPlayer = 2;
		}
		g_theNetSystem->Send( "PlayerReady" );
	}
}

void Map::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if (m_gameState != GameState::WaitingForTheOtherPlayer) {
		if (m_gameState != GameState::AttackAnimation && m_gameState != GameState::MovementAnimation 
			&& (!m_isOnlineNetworkingGame || (m_isOnlineNetworkingGame && (m_curPlayer == m_networkPlayer || m_gameState == GameState::Win)))) {
			HandleMouseInput();
		}
		UpdateGameState();
		g_theRenderer->SetCustomConstantBuffer( m_lightConstantBuffer, &m_dlc, sizeof( m_dlc ), 1 );
		for (auto unit : m_units) {
			if (unit) {
				unit->Update();
			}
		}
		for (int i = 0; i < (int)m_effects.size(); ++i) {
			if (m_effects[i]) {
				m_effects[i]->Update();
			}
		}
		DeleteGarbage();
	}

	std::sort( m_effects.begin(), m_effects.end(), 
		[]( Effect* e1, Effect* e2 ) { 
			if (e1 == nullptr) {
				return false;
			}
			else if (e2 == nullptr) {
				return true;
			}
			return GetDistanceSquared3D( e1->m_position, g_theGame->m_playerPOV.m_position ) > GetDistanceSquared3D( e2->m_position, g_theGame->m_playerPOV.m_position ); 
		} 
	);

	if (m_gameState == GameState::ViewingMap) {
		bool hasPlayer1Unit = false;
		for (auto unit : m_units) {
			if (unit && unit->m_faction == 1) {
				hasPlayer1Unit = true;
				break;
			}
		}
		bool hasPlayer2Unit = false;
		for (auto unit : m_units) {
			if (unit && unit->m_faction == 2) {
				hasPlayer2Unit = true;
				break;
			}
		}
		if (!hasPlayer1Unit) {
			m_gameState = GameState::Win;
			m_player1Wins = false;
		}
		else if (!hasPlayer2Unit) {
			m_gameState = GameState::Win;
			m_player1Wins = true;
		}
	}
}

void Map::UpdateGameState()
{
	if (m_gameState == GameState::ClickOnUnitMovement) {
		if (m_selectingUnit->m_tileOn != m_curHoveringTile
			&& std::find( m_selectingUnit->m_legitimateMovingDest.begin(), m_selectingUnit->m_legitimateMovingDest.end(), m_curHoveringTile ) != m_selectingUnit->m_legitimateMovingDest.end()) {
			std::vector<Tile*> routeToFocusedHex;
			GetRouteBetweenTwoTiles( routeToFocusedHex, m_selectingUnit->m_tileOn, m_curHoveringTile );
			m_selectingUnit->m_targetDir = m_selectingUnit->m_tileOn->GetDirectionToTile( routeToFocusedHex[routeToFocusedHex.size() - 2] );
		}
	}
	else if (m_gameState == GameState::ClickOnUnitAttack) {
		if (m_selectingUnit->m_tileOn != m_curHoveringTile && m_curHoveringTile && m_curHoveringTile->m_unitOnTile
			&& m_selectingUnit->IsUnitInAttackRange( m_curHoveringTile->m_unitOnTile )) {
			m_selectingUnit->m_targetDir = UnitDirection::Custom;
			m_selectingUnit->m_customYawDegrees = Vec2( m_curHoveringTile->m_unitOnTile->m_position - m_selectingUnit->m_position ).GetOrientationDegrees();
			if (m_selectingUnit->m_customYawDegrees < 0.f) {
				m_selectingUnit->m_customYawDegrees += 360.f;
			}
		}
	}
	else if (m_gameState == GameState::MovementAnimation) {
		if (m_selectingUnit->m_position == m_selectingUnit->m_tileOn->GetUnitPosition()) {
			if (m_selectingUnit->m_def.m_type == UnitType::Artillery && m_selectingUnit->m_performedAction) {
				m_gameState = GameState::ViewingMap;
			}
			else {
				m_gameState = GameState::ClickOnUnitAttack;
			}
		}
	}
	else if (m_gameState == GameState::AttackAnimation) {
		if (m_selectingUnit && m_selectingUnit->m_endedAttackAnim) {
			m_gameState = GameState::ViewingMap;
			EndCurUnitTurn();
		}
	}
}

void Map::Render() const
{
	if (m_gameState == GameState::WaitingForTheOtherPlayer) {
		return;
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( m_moonSurfaceMat->m_shader );
	g_theRenderer->BindTexture( m_moonSurfaceMat->m_normalTexture, 1 );
	g_theRenderer->BindTexture( m_moonSurfaceMat->m_diffuseTxeture, 0 );
	g_theRenderer->BindTexture( m_moonSurfaceMat->m_specGlossEmitTexture, 2 );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( m_tbnVerts, m_tbnInts );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_overlayShader.c_str(), VertexType::PCU ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexIndexed( m_tileHexVertexBuffer, m_tileHexIndexBuffer, m_tileHexIndexBuffer->GetIndexCount() );

	if (m_selectingUnit) {
		if (m_gameState == GameState::ClickOnUnitMovement) {
			std::vector<Vertex_PCU> movementTileVerts;
			std::vector<unsigned int> movementTileInts;
			for (auto tile : m_selectingUnit->m_legitimateMovingDest) {
				AddVertsForSolidHexagon( movementTileVerts, movementTileInts, tile->m_center, 0.5f, 0.06f, Rgba8( 255, 255, 255, 100 ) );
				AddVertsForHexagon( movementTileVerts, movementTileInts, tile->m_center, 0.5f, 0.06f, Rgba8( 255, 255, 255 ) );
			}
			if (m_curHoveringTile && !m_curHoveringTile->m_unitOnTile && 
				std::find( m_selectingUnit->m_legitimateMovingDest.begin(), m_selectingUnit->m_legitimateMovingDest.end(), m_curHoveringTile ) != m_selectingUnit->m_legitimateMovingDest.end()) {
				std::vector<Tile*> route;
				GetRouteBetweenTwoTiles( route, m_selectingUnit->m_tileOn, m_curHoveringTile );
				for (auto tile : route) {
					AddVertsForSolidHexagon( movementTileVerts, movementTileInts, tile->m_center, 0.5f, 0.12f, Rgba8( 255, 255, 255, 100 ) );
					AddVertsForHexagon( movementTileVerts, movementTileInts, tile->m_center, 0.5f, 0.12f, Rgba8( 255, 255, 255 ) );
				}
			}
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_overlayShader.c_str(), VertexType::PCU ) );
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( movementTileVerts, movementTileInts );
		}
		else if (m_gameState == GameState::ClickOnUnitAttack) {
			std::vector<Vertex_PCU> attackTileVerts;
			std::vector<unsigned int> attackTileInts;
			std::vector<Unit*> inRangeUnits;
			m_selectingUnit->GetAllEnemyUnitsInRange( inRangeUnits );
			for (auto unit : inRangeUnits) {
				AddVertsForSolidHexagon( attackTileVerts, attackTileInts, unit->m_tileOn->m_center, 0.4f, 0.06f, Rgba8( 255, 0, 0, 100 ) );
			}
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_overlayShader.c_str(), VertexType::PCU ) );
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( attackTileVerts, attackTileInts );
		}

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_overlayShader.c_str(), VertexType::PCU ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants( Mat44( Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), m_selectingUnit->m_tileOn->m_center ), Rgba8( 0, 0, 255 ) );
		g_theRenderer->DrawVertexIndexed( m_currentHoveringHexBuffer, m_currentHoveringHexIndexBuffer, m_currentHoveringHexIndexBuffer->GetIndexCount() );
	}

	if (m_curHoveringTile && !m_curHoveringTile->m_def.m_isBlocked) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_overlayShader.c_str(), VertexType::PCU ) );
		g_theRenderer->BindTexture( nullptr );
		if (m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile->m_faction != m_curPlayer) {
			g_theRenderer->SetModelConstants( Mat44( Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), m_curHoveringTile->m_center ), Rgba8( 255, 0, 0 ) );
		}
		else {
			g_theRenderer->SetModelConstants( Mat44( Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), m_curHoveringTile->m_center ), Rgba8( 0, 255, 0 ) );
		}
		g_theRenderer->DrawVertexIndexed( m_currentHoveringHexBuffer, m_currentHoveringHexIndexBuffer, m_currentHoveringHexIndexBuffer->GetIndexCount() );
	}

	std::vector<Vertex_PCU> verts;
	AddVertsForLineSegment2D( verts, m_mapBounds.m_mins, Vec2( m_mapBounds.m_maxs.x, m_mapBounds.m_mins.y ), 0.05f, Rgba8::WHITE );
	AddVertsForLineSegment2D( verts, m_mapBounds.m_mins, Vec2( m_mapBounds.m_mins.x, m_mapBounds.m_maxs.y ), 0.05f, Rgba8::WHITE );
	AddVertsForLineSegment2D( verts, m_mapBounds.m_maxs, Vec2( m_mapBounds.m_mins.x, m_mapBounds.m_maxs.y ), 0.05f, Rgba8::WHITE );
	AddVertsForLineSegment2D( verts, m_mapBounds.m_maxs, Vec2( m_mapBounds.m_maxs.x, m_mapBounds.m_mins.y ), 0.05f, Rgba8::WHITE );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	for (auto unit : m_units) {
		if (unit) {
			unit->Render();
		}
	}

	for (auto effect : m_effects) {
		if (effect) {
			effect->Render();
		}
	}
}

void Map::RenderUI() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	if (m_gameState == GameState::Win) {
		if (m_player1Wins) {
			AddVertsForUIPanelBlack( verts, AABB2( Vec2( 550.f, 280.f ), Vec2( 1050.f, 520.f ) ), 2.f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 450.f ), Vec2( 1050.f, 505.f ) ), 55.f, Stringf( "Player 1 Wins", m_curPlayer ), Rgba8::WHITE, 0.5f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 308.f ), Vec2( 1050.f, 335.f ) ), 27.f, Stringf( "Press ENTER or Click to Continue" ), Rgba8::WHITE, 0.5f );
		}
		else {
			AddVertsForUIPanelBlack( verts, AABB2( Vec2( 550.f, 280.f ), Vec2( 1050.f, 520.f ) ), 2.f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 450.f ), Vec2( 1050.f, 505.f ) ), 55.f, Stringf( "Player 2 Wins", m_curPlayer ), Rgba8::WHITE, 0.5f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 308.f ), Vec2( 1050.f, 335.f ) ), 27.f, Stringf( "Press ENTER or Click to Continue" ), Rgba8::WHITE, 0.5f );
		}

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( textVerts );
		return;
	}

	// unit info
	std::vector<Vertex_PCU> illustrationVerts1;
	std::vector<Vertex_PCU> illustrationVerts2;
	Texture* illustrationTexture1 = nullptr;
	Texture* illustrationTexture2 = nullptr;

	if ((m_gameState == GameState::ClickOnUnitMovement || m_gameState == GameState::ClickOnUnitAttack) || (m_gameState == GameState::ViewingMap && m_curHoveringTile && m_curHoveringTile->m_unitOnTile)) {
		Unit* inspectingUnit = nullptr;
		if ((m_gameState == GameState::ClickOnUnitMovement || m_gameState == GameState::ClickOnUnitAttack) && m_selectingUnit && m_selectingUnit->m_faction == 1) {
			inspectingUnit = m_selectingUnit;
			illustrationTexture1 = inspectingUnit->m_def.m_imageFile;
		}
		else if (m_curHoveringTile && m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile->m_faction == 1) {
			inspectingUnit = m_curHoveringTile->m_unitOnTile;
			illustrationTexture1 = inspectingUnit->m_def.m_imageFile;
		}
		if (inspectingUnit) {
			AddVertsForUIPanelBlack( verts, AABB2( Vec2( 40.f, 30.f ), Vec2( 280.f, 460.f ) ), 2.f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 60.f, 420.f ), Vec2( 260.f, 450.f ) ), 30.f, inspectingUnit->m_def.m_name, Rgba8::WHITE, 0.5f);
			if (inspectingUnit->m_def.m_type == UnitType::Artillery) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 60.f, 400.f ), Vec2( 260.f, 420.f ) ), 20.f, "Artillery", Rgba8::RED, 0.5f);
			}
			else if (inspectingUnit->m_def.m_type == UnitType::Tank) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 60.f, 400.f ), Vec2( 260.f, 420.f ) ), 20.f, "Tank", Rgba8::RED, 0.5f );
			}
			AddVertsForAABB2D( illustrationVerts1, AABB2( Vec2( 60.f, 220.f ), Vec2( 260.f, 420.f ) ), Rgba8::WHITE );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 200.f ), Vec2( 270.f, 230.f ) ), 26.f, "Attack", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 200.f ), Vec2( 270.f, 230.f ) ), 26.f, Stringf( "%d", inspectingUnit->m_def.m_groundAttackDamage ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 160.f ), Vec2( 270.f, 190.f ) ), 26.f, "Defense", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 160.f ), Vec2( 270.f, 190.f ) ), 26.f, Stringf( "%d", inspectingUnit->m_def.m_defense ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 120.f ), Vec2( 270.f, 150.f ) ), 26.f, "Range", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 120.f ), Vec2( 270.f, 150.f ) ), 26.f, Stringf( "%d-%d", inspectingUnit->m_def.m_groundAttackRangeMin, inspectingUnit->m_def.m_groundAttackRangeMax ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 80.f ), Vec2( 270.f, 110.f ) ), 26.f, "Move", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 80.f ), Vec2( 270.f, 110.f ) ), 26.f, Stringf( "%d", inspectingUnit->m_def.m_movementRange ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 40.f ), Vec2( 270.f, 70.f ) ), 26.f, "Health", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 40.f ), Vec2( 270.f, 70.f ) ), 26.f, Stringf( "%d/%d", inspectingUnit->m_health, inspectingUnit->m_def.m_health ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
		}		
		inspectingUnit = nullptr;
		if ((m_gameState == GameState::ClickOnUnitMovement || m_gameState == GameState::ClickOnUnitAttack) && m_selectingUnit && m_selectingUnit->m_faction == 2) {
			inspectingUnit = m_selectingUnit;
			illustrationTexture2 = inspectingUnit->m_def.m_imageFile;
		}
		else if (m_curHoveringTile && m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile->m_faction == 2) {
			inspectingUnit = m_curHoveringTile->m_unitOnTile;
			illustrationTexture2 = inspectingUnit->m_def.m_imageFile;
		}
		if (inspectingUnit) {
			AddVertsForUIPanelBlack( verts, AABB2( Vec2( 1320.f, 40.f ), Vec2( 1560.f, 460.f ) ), 2.f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1340.f, 420.f ), Vec2( 1540.f, 450.f ) ), 30.f, inspectingUnit->m_def.m_name, Rgba8::WHITE, 0.5f );
			if (inspectingUnit->m_def.m_type == UnitType::Artillery) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1340.f, 400.f ), Vec2( 1540.f, 420.f ) ), 20.f, "Artillery", Rgba8::RED, 0.5f );
			}
			else if (inspectingUnit->m_def.m_type == UnitType::Tank) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1340.f, 400.f ), Vec2( 1540.f, 420.f ) ), 20.f, "Tank", Rgba8::RED, 0.5f );
			}
			AddVertsForAABB2D( illustrationVerts2, AABB2( Vec2( 1340.f, 220.f ), Vec2( 1540.f, 420.f ) ), Rgba8::WHITE );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 200.f ), Vec2( 1550.f, 230.f ) ), 26.f, "Attack", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 200.f ), Vec2( 1550.f, 230.f ) ), 26.f, Stringf( "%d", inspectingUnit->m_def.m_groundAttackDamage ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 160.f ), Vec2( 1550.f, 190.f ) ), 26.f, "Defense", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 160.f ), Vec2( 1550.f, 190.f ) ), 26.f, Stringf( "%d", inspectingUnit->m_def.m_defense ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 120.f ), Vec2( 1550.f, 150.f ) ), 26.f, "Range", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 120.f ), Vec2( 1550.f, 150.f ) ), 26.f, Stringf( "%d-%d", inspectingUnit->m_def.m_groundAttackRangeMin, inspectingUnit->m_def.m_groundAttackRangeMax ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 80.f ), Vec2(  1550.f, 110.f ) ), 26.f, "Move", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 80.f ), Vec2(  1550.f, 110.f ) ), 26.f, Stringf( "%d", inspectingUnit->m_def.m_movementRange ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 40.f ), Vec2(  1550.f, 70.f ) ), 26.f, "Health", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ) );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1330.f, 40.f ), Vec2(  1550.f, 70.f ) ), 26.f, Stringf( "%d/%d", inspectingUnit->m_health, inspectingUnit->m_def.m_health ), Rgba8::WHITE, 0.5f, Vec2( 1.f, 0.5f ) );

		}
	}
	if (m_gameState != GameState::StartTurn && m_gameState != GameState::EndTurn && m_gameState != GameState::WaitingForTheOtherPlayer) {
		AddVertsForUIPanelBlack( verts, AABB2( Vec2( 280.f, 10.f ), Vec2( 1320.f, 95.f ) ), 2.f );
	}

	// top left panel player1/2's turn
	AABB2 turnBounds;
	if (m_curPlayer == 1) {
		turnBounds = AABB2( Vec2( 10.f, 720.f ), Vec2( 500.f, 790.f ) );
	}
	else if (m_curPlayer == 2) {
		turnBounds = AABB2( Vec2( 1100.f, 720.f ), Vec2( 1590.f, 790.f ) );
	}
	if (m_gameState != GameState::WaitingForTheOtherPlayer) {
		AddVertsForUIPanelBlack( verts, turnBounds, 2.f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, turnBounds, 45.f, Stringf( "Player %d's Turn", m_curPlayer ), Rgba8( 0, 255, 0 ), 0.5f );
	}
	// net working wait for the other player
	if (m_gameState == GameState::WaitingForTheOtherPlayer) {
		AddVertsForUIPanelBlack( verts, AABB2( Vec2( 550.f, 280.f ), Vec2( 1050.f, 520.f ) ), 2.f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 387.5f ), Vec2( 1050.f, 412.5f ) ), 25.f, Stringf( "Waiting for Players" ), Rgba8::WHITE, 0.5f );
	}

	// start turn and end turn panel
	if (m_gameState == GameState::StartTurn) {
		AddVertsForUIPanelBlack( verts, AABB2( Vec2( 550.f, 280.f ), Vec2( 1050.f, 520.f ) ), 2.f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 450.f ), Vec2( 1050.f, 505.f ) ), 55.f, Stringf( "Player %d's Turn", m_curPlayer ), Rgba8::WHITE, 0.5f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 308.f ), Vec2( 1050.f, 335.f ) ), 27.f, Stringf( "Press ENTER or Click to Continue" ), Rgba8::WHITE, 0.5f );
	}
	else if (m_gameState == GameState::EndTurn) {
		AddVertsForUIPanelBlack( verts, AABB2( Vec2( 550.f, 280.f ), Vec2( 1050.f, 520.f ) ), 2.f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 450.f ), Vec2( 1050.f, 505.f ) ), 55.f, Stringf( "End Turn?" ), Rgba8::WHITE, 0.5f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 281.f ), Vec2( 1050.f, 335.f ) ), 27.f, Stringf( "Press ENTER or Click to Continue\nPress ESCAPE to Cancel" ), Rgba8::WHITE, 0.5f );
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	if (m_gameState == GameState::ViewingMap) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 20.f ), Vec2( 1600.f, 50.f ) ), 26.f, "Previous", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 700.f, 20.f ), Vec2( 1600.f, 50.f ) ), 26.f, "Next", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1000.f, 20.f ), Vec2( 1600.f, 50.f ) ), 26.f, "End Turn", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		std::vector<Vertex_PCU> iconVerts;
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 310.f, 22.5f ), Vec2( 335.f, 47.5f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/Left.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
		iconVerts.clear();
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 665.f, 22.5f ), Vec2( 690.f, 47.5f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/Right.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
		iconVerts.clear();
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 965.f, 22.5f ), Vec2( 990.f, 47.5f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/Y.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
	}
	else if (m_gameState == GameState::ClickOnUnitMovement) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 20.f ), Vec2( 1600.f, 50.f ) ), 26.f, "Previous", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 700.f, 20.f ), Vec2( 1600.f, 50.f ) ), 26.f, "Next", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		bool renderLMB = false;
		if (m_curHoveringTile && m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile != m_selectingUnit && m_curHoveringTile->m_unitOnTile->m_faction == m_curPlayer) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Select", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			renderLMB = true;
		}
		else if (m_curHoveringTile == m_selectingUnit->m_tileOn) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Stay", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			renderLMB = true;
		}
		else if(m_curHoveringTile && !m_curHoveringTile->m_unitOnTile && 
			std::find( m_selectingUnit->m_legitimateMovingDest.begin(), m_selectingUnit->m_legitimateMovingDest.end(), m_curHoveringTile ) != m_selectingUnit->m_legitimateMovingDest.end()) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Move", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			renderLMB = true;
		}
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 700.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Deselect", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );

		std::vector<Vertex_PCU> iconVerts;
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 310.f, 22.5f ), Vec2( 335.f, 47.5f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/Left.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
		iconVerts.clear();
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 665.f, 22.5f ), Vec2( 690.f, 47.5f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/Right.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
		iconVerts.clear();
		if (renderLMB) {
			AddVertsForAABB2D( iconVerts, AABB2( Vec2( 310.f, 60.f ), Vec2( 335.f, 85.f ) ), Rgba8::WHITE );
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/LMB.png" ) );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( iconVerts );
			iconVerts.clear();
		}
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 665.f, 60.f ), Vec2( 690.f, 85.f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/RMB.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
		iconVerts.clear();
	}
	else if (m_gameState == GameState::ClickOnUnitAttack) {
		bool renderLMB = false;
		if (m_curHoveringTile && m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile == m_selectingUnit) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Hold Fire", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			renderLMB = true;
		}
		else if ( m_curHoveringTile && m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile->m_faction != m_curPlayer && m_selectingUnit && m_selectingUnit->IsUnitInAttackRange(m_curHoveringTile->m_unitOnTile)) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 345.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Attack", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			renderLMB = true;
		}
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 700.f, 57.5f ), Vec2( 1600.f, 87.5f ) ), 26.f, "Cancel", Rgba8::WHITE, 0.5f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );

		std::vector<Vertex_PCU> iconVerts;
		if (renderLMB) {
			AddVertsForAABB2D( iconVerts, AABB2( Vec2( 310.f, 60.f ), Vec2( 335.f, 85.f ) ), Rgba8::WHITE );
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/LMB.png" ) );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( iconVerts );
			iconVerts.clear();
		}
		AddVertsForAABB2D( iconVerts, AABB2( Vec2( 665.f, 60.f ), Vec2( 690.f, 85.f ) ), Rgba8::WHITE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Icons/RMB.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( iconVerts );
		iconVerts.clear();
	}

	if (illustrationTexture1) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( illustrationTexture1 ); 
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( illustrationVerts1 );
	}
	if (illustrationTexture2) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( illustrationTexture2 );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( illustrationVerts2 );
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void Map::DeleteGarbage()
{
	for (int i = 0; i < (int)m_units.size(); ++i) {
		if (m_units[i] && m_units[i]->m_isGarbage) {
			delete m_units[i];
			m_units[i] = nullptr;
		}
	}
	for (int i = 0; i < (int)m_effects.size(); ++i) {
		if (m_effects[i] && m_effects[i]->m_isGarbage) {
			delete m_effects[i];
			m_effects[i] = nullptr;
		}
	}
}

Effect* Map::SpawnEffect( EffectType type, Vec3 const& position, EulerAngles const& orientation )
{
	Effect* newEffect = nullptr;
	if (type == EffectType::Damage_Number) {
		newEffect = (Effect*)new DamageNumber( position, orientation );
	}
	else if (type == EffectType::Cone_Smoke_Particle) {
		newEffect = (Effect*)new ConeSmokeParticle( position, orientation );
	}
	else if (type == EffectType::Sphere_Fire_Particle) {
		newEffect = (Effect*)new SphereFireParticle( position, orientation );
	}
	else if (type == EffectType::Cone_Muzzle_Particle) {
		newEffect = (Effect*)new ConeMuzzleParticle( position, orientation );
	}
	else if (type == EffectType::Art_Rocket) {
		newEffect = (Effect*)new Rocket( position, orientation );
	}

	for (int i = 0; i < (int)m_effects.size(); ++i) {
		if (m_effects[i] == nullptr) {
			m_effects[i] = newEffect;
			return newEffect;
		}
	}
	m_effects.push_back( newEffect );
	return newEffect;
}

int Map::GetDistBetweenTwoTiles( Tile* t1, Tile* t2 ) const
{
	std::deque<std::pair<Tile*, int>> queue;
	std::map<IntVec2, Tile*> dirtyMap;
	queue.push_back( std::pair<Tile*, int>( t1, 0 ) );
	dirtyMap[t1->m_coords] = t1;
	while (!queue.empty()) {
		std::pair<Tile*, int> const& thisTile = queue.front();
		if (thisTile.first == t2) {
			return thisTile.second;
		}
		std::vector<Tile*> neighbors;
		thisTile.first->GetAllNeighbors( neighbors );
		for (auto adjTile : neighbors) {
			if (!adjTile->m_def.m_isBlocked && dirtyMap.find( adjTile->m_coords ) == dirtyMap.end()) {
				dirtyMap[adjTile->m_coords] = adjTile;
				queue.push_back( std::pair<Tile*, int>( adjTile, thisTile.second + 1 ) );
			}
		
		}
		queue.pop_front();
	}
	ERROR_AND_DIE( "Cannot reach here!" );
}

struct TileNode
{
	TileNode( Tile* tile, TileNode const* parent ) : m_tile( tile ), m_parent( parent ) {};
	Tile* m_tile;
	TileNode const* m_parent;
};

void Map::GetRouteBetweenTwoTiles( std::vector<Tile*>& out_route, Tile* t1, Tile* t2 ) const
{
	out_route.clear();
	std::deque<TileNode*> queue; // current tile, parent
	std::map<IntVec2, Tile*> dirtyMap;
	std::vector<TileNode*> nodeList;
	TileNode* node = new TileNode( t1, nullptr );
	queue.push_back( node );
	nodeList.push_back( node );
	dirtyMap[t1->m_coords] = t1;
	while (!queue.empty()) {
		TileNode const* thisTile = queue.front();
		if (thisTile->m_tile == t2) {
			break;
		}
		else {
			std::vector<Tile*> neighbors;
			thisTile->m_tile->GetAllNeighbors( neighbors );
			for (auto adjTile : neighbors) {
				if (!adjTile->m_def.m_isBlocked && dirtyMap.find( adjTile->m_coords ) == dirtyMap.end()) {
					dirtyMap[adjTile->m_coords] = adjTile;
					node = new TileNode( adjTile, thisTile );
					queue.push_back( node );
					nodeList.push_back( node );
				}
			}
		}
		queue.pop_front();
	}

	if (!queue.empty()) {
		TileNode const* iter = queue.front();
		while (iter != nullptr) {
			out_route.push_back( iter->m_tile );
			iter = iter->m_parent;
		}
	}

	for (auto nodeIter : nodeList) {
		delete nodeIter;
	}
}

Tile* Map::GetTile( IntVec2 const& coords ) const
{
	auto iter = m_tiles.find( coords );
	if (iter != m_tiles.end()) {
		return iter->second;
	}
	return nullptr;
}

Tile* Map::GetTile( int x, int y ) const
{
	return GetTile( IntVec2( x, y ) );
}

void Map::HandleMouseInput()
{
	Vec2 mousePos = g_theInput->GetCursorNormalizedPosition();
	Vec3 mouseWorldPos = g_theGame->m_playerPOV.PerspectiveScreenPosToWorld( mousePos );
	float zeroPlaneFraction = GetFractionWithinRange( 0.f, g_theGame->m_playerPOV.m_position.z, mouseWorldPos.z );
	Vec2 zeroPlaneXY = Interpolate( g_theGame->m_playerPOV.m_position, mouseWorldPos, zeroPlaneFraction );
	//DebugAddWorldLine( g_theGame->m_playerPOV.m_position, mouseWorldPos, 0.02f, 0.f, Rgba8::WHITE, Rgba8::WHITE );
	m_curHoveringTile = GetTileByPos( zeroPlaneXY );
	if (m_gameState == GameState::StartTurn) {
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			m_gameState = GameState::ViewingMap;
			if (m_isOnlineNetworkingGame) {
				g_theNetSystem->Send( "StartTurn" );
			}
		}
	}
	else if (m_gameState == GameState::EndTurn) {
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			if (m_isOnlineNetworkingGame) {
				g_theNetSystem->Send( "EndTurn" );
			}
			m_gameState = GameState::StartTurn;
			EndTurn();
		}
		else if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
			m_gameState = GameState::ViewingMap;
		}
	}
	else if (m_gameState == GameState::ViewingMap) {
		if (m_isOnlineNetworkingGame && m_curHoveringTile) {
			g_theNetSystem->Send( Stringf( "SetFocusedHex Coords=%d,%d", m_curHoveringTile->m_coords.x, m_curHoveringTile->m_coords.y ) );
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			if (m_curHoveringTile && m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile->m_faction == m_curPlayer && !m_curHoveringTile->m_unitOnTile->m_performedAction) {
				if (m_isOnlineNetworkingGame) {
					g_theNetSystem->Send( Stringf( "SelectFocusedUnit" ) );
				}
				m_gameState = GameState::ClickOnUnitMovement;
				m_selectingUnit = m_curHoveringTile->m_unitOnTile;
				m_selectingUnit->m_prevDir = m_selectingUnit->m_dir;
				m_selectingUnit->CalculateLegitimateMovingDest();
			}
		}
		if (g_theInput->WasKeyJustPressed( 'Y' )) {
			// end turn
			m_gameState = GameState::EndTurn;
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW )) {
			if ((m_curPlayer == 1 && m_player1AvailableUnits.size() != 0) || (m_curPlayer == 2 && m_player2AvailableUnits.size() != 0)) {
				m_gameState = GameState::ClickOnUnitMovement;
				if (m_isOnlineNetworkingGame) {
					g_theNetSystem->Send( Stringf( "SelectPreviousUnit" ) );
				}
				SelectPrevUnitInViewingMode();
			}
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW )) {
			if ((m_curPlayer == 1 && m_player1AvailableUnits.size() != 0) || (m_curPlayer == 2 && m_player2AvailableUnits.size() != 0)) {
				m_gameState = GameState::ClickOnUnitMovement;
				if (m_isOnlineNetworkingGame) {
					g_theNetSystem->Send( Stringf( "SelectNextUnit" ) );
				}
				SelectNextUnitInViewingMode();
			}
		}

	}
	else if (m_gameState == GameState::ClickOnUnitMovement) {
		if (m_isOnlineNetworkingGame && m_curHoveringTile) {
			g_theNetSystem->Send( Stringf( "SetFocusedHex Coords=%d,%d", m_curHoveringTile->m_coords.x, m_curHoveringTile->m_coords.y ) );
		}

		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			if (m_curHoveringTile) {
				if (m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile != m_selectingUnit && m_curHoveringTile->m_unitOnTile->m_faction == m_curPlayer) {
					m_selectingUnit = m_curHoveringTile->m_unitOnTile;
					m_selectingUnit->m_prevDir = m_selectingUnit->m_dir;
					m_selectingUnit->CalculateLegitimateMovingDest();
					if (m_isOnlineNetworkingGame) {
						g_theNetSystem->Send( Stringf( "SelectFocusedUnit" ) );
					}
				}
				else if (m_curHoveringTile->m_unitOnTile && m_curHoveringTile->m_unitOnTile == m_selectingUnit) {
					// stay
					m_selectingUnit->MoveTo( m_curHoveringTile );
					m_gameState = GameState::MovementAnimation;
					if (m_isOnlineNetworkingGame) {
						g_theNetSystem->Send( Stringf( "Stay" ) );
					}
				}
				else if (!m_curHoveringTile->m_unitOnTile && std::find( m_selectingUnit->m_legitimateMovingDest.begin(), m_selectingUnit->m_legitimateMovingDest.end(), m_curHoveringTile ) != m_selectingUnit->m_legitimateMovingDest.end()) {
					m_selectingUnit->MoveTo( m_curHoveringTile );
					if (m_selectingUnit->m_def.m_type == UnitType::Artillery) {
						m_selectingUnit->m_performedAction = true;
						m_gameState = GameState::MovementAnimation;
					}
					else {
						m_gameState = GameState::MovementAnimation;
					}
					if (m_isOnlineNetworkingGame) {
						g_theNetSystem->Send( Stringf( "Move" ) );
					}
				}
			}
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTMOUSE )) {
			m_gameState = GameState::ViewingMap;
			m_selectingUnit->m_targetDir = m_selectingUnit->m_prevDir;
			m_selectingUnit = nullptr;
			if (m_isOnlineNetworkingGame) {
				g_theNetSystem->Send( Stringf( "Cancel" ) );
			}
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW )) {
			if ((m_curPlayer == 1 && m_player1AvailableUnits.size() != 0) || (m_curPlayer == 2 && m_player2AvailableUnits.size() != 0)) {
				if (m_isOnlineNetworkingGame) {
					g_theNetSystem->Send( Stringf( "SelectPreviousUnit" ) );
				}
				SelectPrevUnitInMovementMode();
			}
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW )) {
			if ((m_curPlayer == 1 && m_player1AvailableUnits.size() != 0) || (m_curPlayer == 2 && m_player2AvailableUnits.size() != 0)) {
				if (m_isOnlineNetworkingGame) {
					g_theNetSystem->Send( Stringf( "SelectNextUnit" ) );
				}
				SelectNextUnitInMovementMode();
			}
		}
	}
	else if (m_gameState == GameState::ClickOnUnitAttack) {
		if (m_isOnlineNetworkingGame && m_curHoveringTile) {
			g_theNetSystem->Send( Stringf( "SetFocusedHex Coords=%d,%d", m_curHoveringTile->m_coords.x, m_curHoveringTile->m_coords.y ) );
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTMOUSE )) {
			m_gameState = GameState::ClickOnUnitMovement;
			m_selectingUnit->m_targetDir = m_selectingUnit->m_prevDir;
			m_selectingUnit->MoveTo( m_selectingUnit->m_prevTile, true );

			if (m_isOnlineNetworkingGame) {
				g_theNetSystem->Send( Stringf( "Cancel" ) );
			}
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			if (m_curHoveringTile && m_curHoveringTile->m_unitOnTile) {
				if (m_selectingUnit == m_curHoveringTile->m_unitOnTile) {
					if (m_isOnlineNetworkingGame) {
						g_theNetSystem->Send( Stringf( "HoldFire" ) );
					}
					m_gameState = GameState::ViewingMap;
					EndCurUnitTurn();
				}
				else if (m_selectingUnit->IsUnitInAttackRange( m_curHoveringTile->m_unitOnTile )) {
					if (m_isOnlineNetworkingGame) {
						g_theNetSystem->Send( Stringf( "Attack" ) );
					}
					PerformAttackAction();
				}
			}
		}
	}
	else if (m_gameState == GameState::Win) {
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			g_theGame->m_returnToMenu = true;
		}
	}
}

Tile* Map::GetTileByPos( Vec2 const& pos ) const
{
	for (auto const& pair : m_tiles) {
		if (pair.second->IsPointInsideHexagon( pos )) {
			return pair.second;
		}
	}
	return nullptr;
}

void Map::EndTurn()
{
	if (m_curPlayer == 1) {
		m_curPlayer = 2;
		m_player2AvailableUnits.clear();
		for (auto unit : m_units) {
			if (unit && unit->m_faction == m_curPlayer) {
				unit->m_performedAction = false;
				m_player2AvailableUnits.push_back( unit );
			}
		}
	}
	else if (m_curPlayer == 2) {
		m_curPlayer = 1;
		m_player1AvailableUnits.clear();
		for (auto unit : m_units) {
			if (unit && unit->m_faction == m_curPlayer) {
				unit->m_performedAction = false;
				m_player1AvailableUnits.push_back( unit );
			}
		}
	}
}

void Map::PerformAttackAction()
{
	m_selectingUnit->Fire( m_curHoveringTile->m_unitOnTile );
	if (m_curHoveringTile->m_unitOnTile->IsUnitInAttackRange( m_selectingUnit )) {
		m_curHoveringTile->m_unitOnTile->m_targetDir = UnitDirection::Custom;
		m_curHoveringTile->m_unitOnTile->m_customYawDegrees = Vec2( m_selectingUnit->m_position - m_curHoveringTile->m_unitOnTile->m_position ).GetOrientationDegrees();
		if (m_curHoveringTile->m_unitOnTile->m_customYawDegrees < 0.f) {
			m_curHoveringTile->m_unitOnTile->m_customYawDegrees += 360.f;
		}
		m_curHoveringTile->m_unitOnTile->Fire( m_selectingUnit );
	}
	//m_curHoveringTile->m_unitOnTile->ResolveDamage( m_selectingUnit->m_def.m_groundAttackDamage, m_selectingUnit );
	m_gameState = GameState::AttackAnimation;
}

void Map::EndCurUnitTurn()
{
	if (m_curPlayer == 1) {
		for (int i = 0; i < (int)m_player1AvailableUnits.size(); i++) {
			if (m_player1AvailableUnits[i] == m_selectingUnit) {
				m_player1AvailableUnits.erase( m_player1AvailableUnits.begin() + i );
				break;
			}
		}
	}
	else if (m_curPlayer == 2) {
		for (int i = 0; i < (int)m_player2AvailableUnits.size(); i++) {
			if (m_player2AvailableUnits[i] == m_selectingUnit) {
				m_player2AvailableUnits.erase( m_player2AvailableUnits.begin() + i );
				break;
			}
		}
	}
	m_selectingUnit->m_performedAction = true;
	m_selectingUnit = nullptr;
}

void Map::SelectNextUnitInMovementMode()
{
	if (m_curPlayer == 1) {
		for (int i = 0; i < (int)m_player1AvailableUnits.size(); i++) {
			if (m_player1AvailableUnits[i] == m_selectingUnit) {
				m_selectingUnit = m_player1AvailableUnits[(i + 1) % (int)m_player1AvailableUnits.size()];
				m_selectingUnit->CalculateLegitimateMovingDest();
				break;
			}
		}
	}
	else if (m_curPlayer == 2) {
		for (int i = 0; i < (int)m_player2AvailableUnits.size(); i++) {
			if (m_player2AvailableUnits[i] == m_selectingUnit) {
				m_selectingUnit = m_player2AvailableUnits[(i + 1) % (int)m_player2AvailableUnits.size()];
				m_selectingUnit->CalculateLegitimateMovingDest();
				break;
			}
		}
	}
	m_selectingUnit->m_prevDir = m_selectingUnit->m_dir;
}

void Map::SelectPrevUnitInMovementMode()
{
	if (m_curPlayer == 1) {
		for (int i = 0; i < (int)m_player1AvailableUnits.size(); i++) {
			if (m_player1AvailableUnits[i] == m_selectingUnit) {
				m_selectingUnit = m_player1AvailableUnits[(i - 1 + (int)m_player1AvailableUnits.size()) % (int)m_player1AvailableUnits.size()];
				m_selectingUnit->CalculateLegitimateMovingDest();
				break;
			}
		}
	}
	else if (m_curPlayer == 2) {
		for (int i = 0; i < (int)m_player2AvailableUnits.size(); i++) {
			if (m_player2AvailableUnits[i] == m_selectingUnit) {
				m_selectingUnit = m_player2AvailableUnits[(i - 1 + (int)m_player2AvailableUnits.size()) % (int)m_player2AvailableUnits.size()];
				m_selectingUnit->CalculateLegitimateMovingDest();
				break;
			}
		}
	}
	m_selectingUnit->m_prevDir = m_selectingUnit->m_dir;
}

void Map::SelectNextUnitInViewingMode()
{
	if (m_curPlayer == 1) {
		m_selectingUnit = m_player1AvailableUnits[0];
	}
	else if (m_curPlayer == 2) {
		m_selectingUnit = m_player2AvailableUnits[0];
	}
	m_selectingUnit->CalculateLegitimateMovingDest();
	m_selectingUnit->m_prevDir = m_selectingUnit->m_dir;
}

void Map::SelectPrevUnitInViewingMode()
{
	if (m_curPlayer == 1) {
		m_selectingUnit = m_player1AvailableUnits[m_player1AvailableUnits.size() - 1];
	}
	else if (m_curPlayer == 2) {
		m_selectingUnit = m_player2AvailableUnits[m_player2AvailableUnits.size() - 1];
	}
	m_selectingUnit->CalculateLegitimateMovingDest();
	m_selectingUnit->m_prevDir = m_selectingUnit->m_dir;
}

bool operator<( IntVec2 const& a, IntVec2 const& b )
{
	if (a.y < b.y) {
		return true;
	}
	if (a.y > b.y) {
		return false;
	}
	if (a.x < b.x) {
		return true;
	}
	return false;
}

Map* GetCurMap()
{
	return g_theGame->m_map;
}

bool Command_PlayerReady( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	if (map && map->m_gameState == GameState::WaitingForTheOtherPlayer) {
		map->m_gameState = GameState::StartTurn;
		g_theNetSystem->Send( "PlayerReady" );
	}
	return false;
}

bool Command_StartTurn( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	map->m_gameState = GameState::ViewingMap;
	return false;
}

bool Command_SetFocusedHex( EventArgs& args )
{
	Map* map = GetCurMap();
	IntVec2 coords = args.GetValue( "Coords", IntVec2( 0, 0 ) );
	auto iter = map->m_tiles.find( coords );
	if (iter != map->m_tiles.end()) {
		map->m_curHoveringTile = iter->second;
	}
	return false;
}

bool Command_SelectFocusedUnit( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	map->m_gameState = GameState::ClickOnUnitMovement;
	map->m_selectingUnit = map->m_curHoveringTile->m_unitOnTile;
	map->m_selectingUnit->CalculateLegitimateMovingDest();
	map->m_selectingUnit->m_prevDir = map->m_selectingUnit->m_dir;
	return false;
}

bool Command_SelectPreviousUnit( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	if (map->m_gameState == GameState::ClickOnUnitMovement) {
		map->SelectPrevUnitInMovementMode();
	}
	else if (map->m_gameState == GameState::ViewingMap) {
		map->m_gameState = GameState::ClickOnUnitMovement;
		map->SelectPrevUnitInViewingMode();
	}
	return false;
}

bool Command_SelectNextUnit( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();

	if (map->m_gameState == GameState::ViewingMap) {
		map->m_gameState = GameState::ClickOnUnitMovement;
		map->SelectNextUnitInViewingMode();
	}
	else if (map->m_gameState == GameState::ClickOnUnitMovement) {
		map->SelectNextUnitInMovementMode();
	}
	return false;
}

bool Command_Move( EventArgs& args )
{
	Map* map = GetCurMap();
	UNUSED( args );
	map->m_selectingUnit->MoveTo( map->m_curHoveringTile );
	if (map->m_selectingUnit->m_def.m_type == UnitType::Artillery) {
		map->m_selectingUnit->m_performedAction = true;
		map->m_gameState = GameState::MovementAnimation;
	}
	else {
		map->m_gameState = GameState::MovementAnimation;
	}
	return false;
}

bool Command_Stay( EventArgs& args )
{
	Map* map = GetCurMap();
	UNUSED( args );
	map->m_selectingUnit->MoveTo( map->m_curHoveringTile );
	map->m_gameState = GameState::MovementAnimation;;
	return false;
}

bool Command_HoldFire( EventArgs& args )
{
	Map* map = GetCurMap();
	UNUSED( args );
	map->m_gameState = GameState::ViewingMap;
	map->EndCurUnitTurn();
	return false;
}

bool Command_Attack( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	map->PerformAttackAction();
	return false;
}

bool Command_Cancel( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	if (map->m_gameState == GameState::ClickOnUnitAttack) {
		map->m_gameState = GameState::ClickOnUnitMovement;
		map->m_selectingUnit->MoveTo( map->m_selectingUnit->m_prevTile, true );
		map->m_selectingUnit->m_targetDir = map->m_selectingUnit->m_prevDir;
	}
	else if (map->m_gameState == GameState::ClickOnUnitMovement) {
		map->m_gameState = GameState::ViewingMap;
		map->m_selectingUnit->m_targetDir = map->m_selectingUnit->m_prevDir;
		map->m_selectingUnit = nullptr;
	}
	return false;
}

bool Command_EndTurn( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	map->m_gameState = GameState::StartTurn;
	map->EndTurn();
	return false;
}

bool Command_PlayerQuit( EventArgs& args )
{
	UNUSED( args );
	Map* map = GetCurMap();
	// Todo: combine with UI
	//FireEvent( "quit" );
	if (map->m_networkPlayer == 1) {
		map->m_player1Wins = true;
		map->m_gameState = GameState::Win;
	}
	else {
		map->m_player1Wins = false;
		map->m_gameState = GameState::Win;
	}
	return false;
}
