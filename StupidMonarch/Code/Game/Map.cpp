#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Province.hpp"
#include "Game/Force.hpp"
#include "Game/Army.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <queue>

Map::Map( Game* game )
	:m_game(game)
{

}

Map::~Map()
{
	if (m_mapImage) {
		delete m_mapImage;
	}
	if (m_riverImage) {
		delete m_riverImage;
	}
	for (int i = 0; i < (int)m_provinces.size(); i++) {
		if (m_provinces[i]) {
			delete m_provinces[i];
			m_provinces[i] = 0;
		}
	}
	for (int i = 0; i < (int)m_forcesAsOrder.size(); i++) {
		if (m_forcesAsOrder[i]) {
			delete m_forcesAsOrder[i];
			m_forcesAsOrder[i] = 0;
		}
	}
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
}

void Map::StartUp( std::string const& mapPath )
{
	// Load pixel map to class Image
	m_mapImage = new Image( mapPath.c_str() );
	m_riverImage = new Image( "Data/Map/river.png" );
	m_dimensions = m_mapImage->GetDimensions();
	GUARANTEE_OR_DIE( m_dimensions == m_riverImage->GetDimensions(), Stringf( "Error! Dimensions of province map and river map are not the same." ) );

	m_provinces.resize( 1000 );
	for (int i = 0; i < (int)m_provinces.size(); i++) {
		Province* pProvince = new Province( this, i );
		m_provinces[i] = pProvince;
	}

	LoadForces();
	LoadProvinces( g_theGame->m_scenarios[0].m_year );
	LoadLinkInfo();

	// calculate adjacent
	for (int i = 0; i < m_dimensions.x; i++) {
		for (int j = 0; j < m_dimensions.y; j++) {
			Rgba8 const& color = m_mapImage->GetTexelColor( IntVec2( i, j ) );
			int provId = GetProvIdFromColor( color );
			if (provId == -1) {
				continue;
			}
			Province* prov = m_provinces[provId];
			prov->m_numOfPixels++;
			prov->m_sumOfPixelPos += IntVec2( i, j );
			if (i > 0) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i - 1, j ) );
				if (!(adjColor == color)) {
					int adjId = GetProvIdFromColor( adjColor );
					if (adjId != -1) {
						Province* adjProv = m_provinces[adjId];
						adjProv->AddAdjacentProvince( prov );
					}
				}
			}
			if (j > 0) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i, j - 1 ) );
				if (!(adjColor == color)) {
					int adjId = GetProvIdFromColor( adjColor );
					if (adjId != -1) {
						Province* adjProv = m_provinces[adjId];
						adjProv->AddAdjacentProvince( prov );
					}
				}
			}
			if (i < m_dimensions.x - 1) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i + 1, j ) );
				if (!(adjColor == color)) {
					int adjId = GetProvIdFromColor( adjColor );
					if (adjId != -1) {
						Province* adjProv = m_provinces[adjId];
						adjProv->AddAdjacentProvince( prov );
					}
				}
			}
			if (j < m_dimensions.y - 1) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i, j + 1 ) );
				if (!(adjColor == color)) {
					int adjId = GetProvIdFromColor( adjColor );
					if (adjId != -1) {
						Province* adjProv = m_provinces[adjId];
						adjProv->AddAdjacentProvince( prov );
					}
				}
			}
		}
	}

	CreateVertexBuffer();
}

void Map::Update( float deltaTime )
{
	UNUSED( deltaTime );
	for (auto force : m_forcesAsOrder) {
		if (force) {
			force->Update();
		}
	}
}

void Map::Render() const
{

	if (g_gameMode == GameMode::CLICK_ARMY) {
		std::vector<Vertex_PCU> tempVerts;
		tempVerts.reserve( 6 );
		AddVertsForAABB2D( tempVerts, AABB2( Vec2( -1000.f, -1000.f ), Vec2( (float)m_dimensions.x + 1000.f, (float)m_dimensions.y + 1000.f ) ), Rgba8( 192, 192, 192 ), AABB2::IDENTITY );
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( tempVerts );
	}
	//float timeStamp = Clock::GetSystemClock()->GetTotalSeconds();

	for (auto prov : m_provinces) {
		if (prov == nullptr || prov->m_owner == nullptr) {
			continue;
		}
		if (g_gameMode == GameMode::CLICK_ARMY) {
			Army const* choosingArmy = m_game->GetChoosingArmy();
			if (choosingArmy->IsProvValidToGo( prov )) {
				g_theRenderer->SetModelConstants( Mat44(), Rgba8( 102, 255, 102 ) );
			}
			else {
				g_theRenderer->SetModelConstants( Mat44(), Rgba8( 192, 192, 192 ) );
			}
		}
		else if (g_gameMode == GameMode::VIEW_MAP || g_gameMode == GameMode::ARMY_FIGHT_INSPECT) {
			if (m_viewMode == MapViewMode::VIEW_FORCE_MAP) {
				Rgba8 color = prov->GetOwner()->GetForceColor();
				if (prov == g_theGame->m_onInspectProv) {
					color.r = color.r + 50 > 255 ? 255 : color.r + 50;
					color.g = color.g + 50 > 255 ? 255 : color.g + 50;
					color.b = color.b + 50 > 255 ? 255 : color.b + 50;
				}
				g_theRenderer->SetModelConstants( Mat44(), color );
			}
			else if (m_viewMode == MapViewMode::VIEW_ECONOMY_MAP) {
				float economy = prov->GetEconomyCorrectedPoint();
				Rgba8 color = Rgba8::Interpolate( Rgba8( 100, 0, 50 ), Rgba8( 50, 255, 50 ), economy / 500000.f > 1.f ? 1.f : economy / 500000.f );// Rgba8( 50, (unsigned char)RangeMapClamped( economy, 0.f, 300000.f, 0.f, 255.f ), 50 );
				g_theRenderer->SetModelConstants( Mat44(), color );
			}
		}
		else if (g_gameMode == GameMode::CHOOSE_FORCES) {
			Rgba8 color = prov->GetOwner()->GetForceColor();
			if (m_game->m_onInspectForce
				&& m_game->m_onInspectForce->isProvOwned( prov )) {
						color.r = color.r + 80 > 255 ? 255 : color.r + 80;
						color.g = color.g + 80 > 255 ? 255 : color.g + 80;
						color.b = color.b + 80 > 255 ? 255 : color.b + 80;
			}
			if (g_theGame->IsForceChosenByPlayer( prov->m_owner )
				&& m_game->m_onInspectForce != prov->m_owner) {
				color.r = 96;
				color.g = 96;
				color.b = 96;
			}
			g_theRenderer->SetModelConstants( Mat44(), color );
		}
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexBuffer( m_vertexBuffer, prov->m_numOfVerts, prov->m_startOfVertexBuffer );
	}
	//DebugAddMessage( Stringf( "%f", Clock::GetSystemClock()->GetTotalSeconds() - timeStamp ), 1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexBuffer( m_vertexBuffer, m_numOfEdgeVerts + m_numOfRiverVerts, m_startOfEdge );
	//g_theRenderer->DrawVertexBuffer( m_vertexBuffer, , m_startOfRiver );

	if (g_gameMode == GameMode::CLICK_ARMY) {
		Army const* choosingArmy = m_game->GetChoosingArmy();
		std::vector<Province*> provincesCanGo;
		GetAllProvsArmyCanGo( provincesCanGo, choosingArmy );
		std::vector<Vertex_PCU> tempVerts;
		tempVerts.reserve( 1000 );
		for (auto prov : provincesCanGo) {
			AddVertsForArrow2D( tempVerts, choosingArmy->GetCenter(), prov->GetCenter(), 0.8f, 0.5f, Rgba8( 200, 0, 0 ) );
		}
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( tempVerts );
	}

	//g_theRenderer->DrawVertexBuffer(m_vertexBuffer)
	/*
	std::vector<Vertex_PCU> mapAABB2Verts;
	mapAABB2Verts.reserve( 150000 );

	AABB2 const& cameraRangeOnMap = m_game->GetCameraRangeOnMap();
	int maxI = RoundDownToInt( cameraRangeOnMap.m_maxs.x ) + 1;
	int minI = RoundDownToInt( cameraRangeOnMap.m_mins.x );
	int maxJ = RoundDownToInt( cameraRangeOnMap.m_maxs.y ) + 1;
	int minJ = RoundDownToInt( cameraRangeOnMap.m_mins.y );

	maxI = GetClamped( maxI, 0, m_dimensions.x );
	minI = GetClamped( minI, 0, m_dimensions.x );
	maxJ = GetClamped( maxJ, 0, m_dimensions.y );
	minJ = GetClamped( minJ, 0, m_dimensions.y );
	
	if (g_gameMode == GameMode::CLICK_ARMY) {
		std::vector<Province*> provincesCanGo;
		Army const* choosingArmy = m_game->GetChoosingArmy();
		GetAllProvsArmyCanGo( provincesCanGo, choosingArmy );

		AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( -50.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) ), Rgba8( 192, 192, 192 ), AABB2::IDENTITY );

		for (int i = minI; i < maxI; i++) {
			for (int j = minJ; j < maxJ; j++) {
				Rgba8 const& pixelColor = m_mapImage->GetTexelColor( IntVec2( i, j ) );
				Rgba8 color;
				if (pixelColor == Rgba8( 255, 255, 255 )) {
					color = Rgba8( 192, 192, 192 );
				}
				else {
					Province* provToColor = GetProvFromColor( pixelColor );
					bool find = false;
					for (auto prov : provincesCanGo) {
						if (provToColor == prov) {
							color = Rgba8( 102, 255, 102 );
							find = true;
							break;
						}
					}
					if (!find) {
						color = Rgba8( 192, 192, 192 );
					}
				}
				AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)(j + 1) ) ), color, AABB2::IDENTITY );
			}
		}

		for (auto prov : provincesCanGo) {
			AddVertsForArrow2D( mapAABB2Verts, choosingArmy->GetCenter(), prov->GetCenter(), 0.8f, 0.5f, Rgba8( 200, 0, 0 ) );
		}
	}
	else if (g_gameMode == GameMode::VIEW_MAP || g_gameMode == GameMode::ARMY_FIGHT_INSPECT) {
		if (m_viewMode == MapViewMode::VIEW_FORCE_MAP) {
			for (int i = minI; i < maxI; i++) {
				for (int j = minJ; j < maxJ; j++) {
					Rgba8 const& pixelColor = m_mapImage->GetTexelColor( IntVec2( i, j ) );
					Rgba8 color;
					if (!(pixelColor == Rgba8( 255, 255, 255 ))) {
						color = GetProvFromColor( pixelColor )->GetOwner()->GetForceColor();
					}
					else {
						color = pixelColor;
					}
					if (m_game->m_onInspectProv && !(pixelColor == Rgba8( 255, 255, 255 )) && m_provinces[m_mapFromColorToProvId.find( pixelColor )->second] == m_game->m_onInspectProv) {
						color.r = color.r + 50 > 255 ? 255 : color.r + 50;
						color.g = color.g + 50 > 255 ? 255 : color.g + 50;
						color.b = color.b + 50 > 255 ? 255 : color.b + 50;
					}
					AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)(j + 1) ) ), color, AABB2::IDENTITY );
				}
			}
		}
		else if(m_viewMode == MapViewMode::VIEW_ECONOMY_MAP){
			for (int i = minI; i < maxI; i++) {
				for (int j = minJ; j < maxJ; j++) {
					Rgba8 const& pixelColor = m_mapImage->GetTexelColor( IntVec2( i, j ) );
					Rgba8 color;
					if (!(pixelColor == Rgba8( 255, 255, 255 ))) {
						float economy = GetProvFromColor( pixelColor )->GetEconomyCorrectedPoint();
						color = Rgba8( 50, (unsigned char)RangeMapClamped( economy, 0.f, 500000.f, 0.f, 255.f ), 50 );
					}
					else {
						color = pixelColor;
					}
					//if (m_game->m_onInspectProv && !(pixelColor == Rgba8( 255, 255, 255 )) && m_provinces[m_mapFromColorToProvId.find( pixelColor )->second] == m_game->m_onInspectProv) {
					//	color.r = color.r + 50 > 255 ? 255 : color.r + 50;
					//	color.g = color.g + 50 > 255 ? 255 : color.g + 50;
					//	color.b = color.b + 50 > 255 ? 255 : color.b + 50;
					//}
					AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)(j + 1) ) ), color, AABB2::IDENTITY );
				}
			}
		}
	}
	else if (g_gameMode == GameMode::CHOOSE_FORCES) {
		for (int i = minI; i < maxI; i++) {
			for (int j = minJ; j < maxJ; j++) {
				Rgba8 const& pixelColor = m_mapImage->GetTexelColor( IntVec2( i, j ) );
				Rgba8 color;
				if (!(pixelColor == Rgba8( 255, 255, 255 ))) {
					color = GetProvFromColor( pixelColor )->GetOwner()->GetForceColor();
				}
				else {
					color = pixelColor;
				}
				if (m_game->m_onInspectForce 
					&& !(pixelColor == Rgba8( 255, 255, 255 )) 
					&& m_game->m_onInspectForce->isProvOwned(m_provinces[m_mapFromColorToProvId.find( pixelColor )->second])) {
					color.r = color.r + 80 > 255 ? 255 : color.r + 80;
					color.g = color.g + 80 > 255 ? 255 : color.g + 80;
					color.b = color.b + 80 > 255 ? 255 : color.b + 80;
				}
				if (!(pixelColor == Rgba8( 255, 255, 255 )) 
					&& g_theGame->IsForceChosenByPlayer( m_provinces[m_mapFromColorToProvId.find( pixelColor )->second]->m_owner )
					&& m_game->m_onInspectForce != m_provinces[m_mapFromColorToProvId.find( pixelColor )->second]->m_owner) {
					color.r = 96;
					color.g = 96;
					color.b = 96;
				}
				AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)(j + 1) ) ), color, AABB2::IDENTITY );
			}
		}
	}

	constexpr float lineThickness = 0.3f;
	for (int i = minI; i < maxI; i++) {
		for (int j = minJ; j < maxJ; j++) {
			Rgba8 const& color = m_mapImage->GetTexelColor( IntVec2( i, j ) );
			if (i == 0 && !(color == Rgba8::WHITE)) {
				AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)j ), Vec2( (float)i, (float)(j + 1) ), lineThickness, Rgba8( 128, 128, 128 ) );
			}
			if (i > 0) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i - 1, j ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)j ), Vec2( (float)i, (float)(j + 1) ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
			if (j > 0) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i, j - 1 ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)j ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
			if (i < m_dimensions.x - 1) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i + 1, j ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)(i + 1), (float)(j + 1) ), Vec2( (float)(i + 1), (float)j ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
			if (j < m_dimensions.y - 1) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i, j + 1 ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)(j + 1) ), Vec2( (float)(i + 1), (float)(j + 1) ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
		}
	}

	for (int i = minI; i < maxI; i++) {
		for (int j = minJ; j < maxJ; j++) {
			Rgba8 const& color = m_riverImage->GetTexelColor( IntVec2( i, j ) );
			if (color == Rgba8( 0, 0, 255 )) {
				AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)i + 1.f, (float)j + 1.f ) ), Rgba8( 0, 0, 255, 255 ), AABB2::IDENTITY );
			}
		}
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( mapAABB2Verts );
	*/
	for (auto force : m_forcesAsOrder) {
		if (force) {
			force->Render();
		}
	}
}

void Map::NextTurn()
{
//	if (g_gameMode == GameMode::ARMY_FIGHT_INSPECT) {
//		return;
//	}
	for(;;){
		m_curForceIndex++;
		if (m_curForceIndex == (int)m_forcesAsOrder.size()) {
			//ResolveCombat();
			m_game->NextRound();
			m_curForceIndex = -1;
		}
		else if (!m_forcesAsOrder[m_curForceIndex] || !(m_forcesAsOrder[m_curForceIndex]->isAlive())) {
			continue;
		}
		else if (m_forcesAsOrder[m_curForceIndex]->isAI()) {
			m_forcesAsOrder[m_curForceIndex]->NextTurn();
		}
		else {
			m_game->SetCurrentForce( m_forcesAsOrder[m_curForceIndex] );
			break;
		}
	}
}

void Map::ResolveCombat()
{

}

bool Map::IsAdjacent( Province* p1, Province* p2 ) const
{
	return p1->IsAdjacent( p2 );
}

Province* Map::GetProvinceByWorldPos( Vec2 const& worldPos ) const
{
	if (worldPos.x > (float)m_dimensions.x || worldPos.x < 0.f || worldPos.y >( float )m_dimensions.y || worldPos.y < 0.f) {
		return nullptr;
	}
	int id = GetProvinceIdFromMapPos( GetMapPosFromWorldPos( worldPos ) );
	if (id == -1) {
		return nullptr;
	}
	return m_provinces[id];
}

LinkType Map::GetProvLinkType( Province* provFrom, Province* provTo ) const
{
	std::queue<Province*> provQue;
	int* flag = new int[m_provinces.size()];
	for (int i = 0; i < (int)m_provinces.size(); i++) {
		flag[i] = 0;
	}
	provQue.push( provFrom );
	while (!provQue.empty()) {
		Province* thisProv = provQue.front();
		provQue.pop();
		for (auto adjProv : thisProv->m_adjacentProvinces) {
			if (adjProv == provTo) {
				LinkType retType = GetAdjProvLinkType( thisProv, adjProv );
				Province* sameDepthProv = nullptr;
				if (provQue.empty()) {
					return retType;
				}
				// check all nearest way
				do {
					sameDepthProv = provQue.front();
					provQue.pop();
					if (IsAdjacent( sameDepthProv, adjProv )) {
						LinkType lnkType = GetAdjProvLinkType( sameDepthProv, adjProv );
						if ((int)lnkType < (int)retType) {
							retType = lnkType;
						}
					}
				} while (flag[sameDepthProv->m_id] == flag[thisProv->m_id] && !provQue.empty());
				delete[] flag;
				return retType;
			}
			else if (flag[adjProv->m_id] == 0) {
				flag[adjProv->m_id] = flag[thisProv->m_id] + 1;
				provQue.push( adjProv );
			}
		}
	}
	delete[] flag;
	ERROR_AND_DIE( "Two provinces are not connected together!" );
}

LinkType Map::GetAdjProvLinkType( Province* provFrom, Province* provTo ) const
{
	if (!IsAdjacent( provFrom, provTo )) {
		ERROR_AND_DIE( "Two provinces are not Adjacent to each other!" );
	}
	for (auto& link : m_provLinkInfo[provFrom->m_id]) {
		if (link.m_provIdToLink == provTo->m_id) {
			return link.m_type;
		}
	}
	return LinkType::Default;
}

IntVec2 const& Map::GetDimensions() const
{
	return m_dimensions;
}

void Map::LoadForces()
{
	// Load forces
	m_forcesAsOrder.resize( 100 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Map/ForceDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document ForceDefinitions.xml failed!" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ForceDefinitions" ), "Syntax Error! Name of the root of ForceDefinitions.xml should be \"ForceDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ForceDefinition" ), "Syntax Error! Names of the elements of ForceDefinitions.xml should be \"ForceDefinition\" " );
		int id = ParseXmlAttribute( *xmlIter, "id", -1 );
		Rgba8 color = ParseXmlAttribute( *xmlIter, "color", Rgba8( 255, 255, 255 ) );
		std::string nickName = ParseXmlAttribute( *xmlIter, "forceNickName", "DEFAULT" );
		int capitalProvId = ParseXmlAttribute( *xmlIter, "capitalProvinceId", -1 );
		Force* pForce = new Force( this, id, color, nickName, m_provinces[capitalProvId] );
		//pForce->SetAsPlayer( true );
		m_forcesAsOrder[id] = pForce;
		xmlIter = xmlIter->NextSiblingElement();
	}
}

void Map::LoadProvinces( int startYear )
{
	// Load province info and construct mapping
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Map/ProvinceDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document ProvinceDefinitions.xml failed!" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ProvinceDefinitions" ), "Syntax Error! Name of the root of ProvinceDefinitions.xml should be \"ProvinceDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ProvinceDefinition" ), "Syntax Error! Names of the elements of ProvinceDefinitions.xml should be \"ProvinceDefinition\" " );
		int id = ParseXmlAttribute( *xmlIter, "id", -1 );
		Province* pProvince = m_provinces[id];
		Rgba8 color = ParseXmlAttribute( *xmlIter, "color", Rgba8( 255, 255, 255 ) );
		bool isMajor = ParseXmlAttribute( *xmlIter, "isMajor", false );
		int maxDef = ParseXmlAttribute( *xmlIter, "maxDef", 1000 );
		float huhuaness = ParseXmlAttribute( *xmlIter, "huhuaness", 0.f );
		std::string name = ParseXmlAttribute( *xmlIter, "provinceName", "Default" );
		std::string capitalName = ParseXmlAttribute( *xmlIter, "capitalName", "Default" );
		bool isPlain = ParseXmlAttribute( *xmlIter, "isPlain", false );
		bool isMountain = ParseXmlAttribute( *xmlIter, "isMountain", false );
		auto findValue = m_mapFromColorToProvId.find( color );
		GUARANTEE_OR_DIE( findValue == m_mapFromColorToProvId.end(), Stringf( "Load provinces error! Already has prov id%d that has such color R%dG%dB%d", findValue->second, color.r, color.g, color.b ) );
		m_mapFromColorToProvId[color] = id;
		pProvince->StartUp( isMajor, isPlain, isMountain, maxDef, huhuaness, name, capitalName );
		LoadHistory( startYear );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

void Map::LoadHistory( int loadToYear )
{
	// Load province info and construct mapping
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Map/ProvinceDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document ProvinceDefinitions.xml failed!" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ProvinceDefinitions" ), "Syntax Error! Name of the root of ProvinceDefinitions.xml should be \"ProvinceDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ProvinceDefinition" ), "Syntax Error! Names of the elements of ProvinceDefinitions.xml should be \"ProvinceDefinition\" " );
		int id = ParseXmlAttribute( *xmlIter, "id", -1 );
		Province* pProvince = m_provinces[id];
		XmlElement* historyIter = xmlIter->FirstChildElement();
		GUARANTEE_OR_DIE( historyIter != nullptr, Stringf( "Province id %d must have history!", id ) );
		while (historyIter != nullptr) {
			int year = ParseXmlAttribute( *historyIter, "year", -1 );
			if (year > loadToYear) {
				GUARANTEE_OR_DIE( pProvince->m_owner != nullptr, Stringf( "Province id %d, history error! Do not owned by a force!", id ) );
				break;
			}
			int economy = ParseXmlAttribute( *historyIter, "economy", 1 );
			int population = ParseXmlAttribute( *historyIter, "pop", 1000 );
			std::string ownerNickname = ParseXmlAttribute( *historyIter, "owner", "DEFAULT" );
			std::string isLegal = ParseXmlAttribute( *historyIter, "isLegal", "DEFAULT" );
			Force* legalForce = GetForceByNickName( isLegal );
			Force* ownerForce = GetForceByNickName( ownerNickname );
			pProvince->AddLegalForce( legalForce );
			if (pProvince->m_owner) {
				pProvince->m_owner->LoseProvince( pProvince );
			}
			GUARANTEE_OR_DIE( ownerForce != nullptr, Stringf( "Province id %d owned by %s, has no such force!", id, ownerNickname.c_str() ) );
			pProvince->SetHistory( economy, population, ownerForce, legalForce );
			ownerForce->GainProvince( pProvince );
			historyIter = historyIter->NextSiblingElement();
		}
		xmlIter = xmlIter->NextSiblingElement();
	}
}

void Map::LoadLinkInfo()
{
	m_provLinkInfo.resize( m_provinces.size() );
	// Load province info and construct mapping
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Map/ProvinceLinkInfo.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document ProvinceLinkInfo.xml failed!" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ProvLinkInfo" ), "Syntax Error! Name of the root of ProvinceLinkInfo.xml should be \"ProvLinkInfo\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ProvLink" ), "Syntax Error! Names of the elements of ProvinceLinkInfo.xml should be \"ProvLink\" " );
		int id1 = ParseXmlAttribute( *xmlIter, "id1", -1 );
		if (id1 == -1) {
			xmlIter = xmlIter->NextSiblingElement();
			continue;
		}
		int id2 = ParseXmlAttribute( *xmlIter, "id2", -1 );
		if (id2 == -1) {
			xmlIter = xmlIter->NextSiblingElement();
			continue;
		}
		std::string type = ParseXmlAttribute( *xmlIter, "type", "River" );
		if (type == "River") {
			m_provLinkInfo[id1].emplace_back( LinkType::River, id2 );
			m_provLinkInfo[id2].emplace_back( LinkType::River, id1 );
		}
		else if (type == "BigRiver") {
			m_provLinkInfo[id1].emplace_back( LinkType::BigRiver, id2 );
			m_provLinkInfo[id2].emplace_back( LinkType::BigRiver, id1 );
		}
		xmlIter = xmlIter->NextSiblingElement();
	}
}

void Map::CreateVertexBuffer()
{
	std::vector<Vertex_PCU> mapAABB2Verts;
	mapAABB2Verts.reserve( m_dimensions.x * m_dimensions.y * 10 );

	for (auto prov : m_provinces) {
		int startIndex = (int)mapAABB2Verts.size();
		int mostLeft = INT_MAX;
		int mostRight = 0;
		int mostTop = 0;
		int mostBottom = INT_MAX;
		for (int i = 0; i < m_dimensions.x; i++) {
			for (int j = 0; j < m_dimensions.y; j++) {
				Rgba8 const& pixelColor = m_mapImage->GetTexelColor( IntVec2( i, j ) );
				Rgba8 color;
				if (!(pixelColor == Rgba8( 255, 255, 255 )) && prov == GetProvFromColor( pixelColor )) {
					AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)(j + 1) ) ), Rgba8::WHITE, AABB2::IDENTITY );
					if (i > mostRight) {
						mostRight = i + 1;
					}
					if (i < mostLeft) {
						mostLeft = i;
					}
					if (j > mostTop) {
						mostTop = j;
					}
					if (j < mostBottom) {
						mostBottom = j + 1;
					}
				}
			}
		}
		int endIndex = (int)mapAABB2Verts.size();
		prov->m_startOfVertexBuffer = startIndex;
		prov->m_numOfVerts = endIndex - startIndex;
		prov->m_leftBottomPos = Vec2( (float)mostLeft, (float)mostBottom );
		prov->m_rightTopPos = Vec2( (float)mostRight, (float)mostTop );
	}

	constexpr float lineThickness = 0.3f;
	m_startOfEdge = (int)mapAABB2Verts.size();
	for (int i = 0; i < m_dimensions.x; i++) {
		for (int j = 0; j < m_dimensions.y; j++) {
			Rgba8 const& color = m_mapImage->GetTexelColor( IntVec2( i, j ) );
			if (i == 0 && !(color == Rgba8::WHITE)) {
				AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)j ), Vec2( (float)i, (float)(j + 1) ), lineThickness, Rgba8( 128, 128, 128 ) );
			}
			if (i > 0) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i - 1, j ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)j ), Vec2( (float)i, (float)(j + 1) ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
			if (j > 0) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i, j - 1 ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)j ), Vec2( (float)(i + 1), (float)j ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
			if (i < m_dimensions.x - 1) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i + 1, j ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)(i + 1), (float)(j + 1) ), Vec2( (float)(i + 1), (float)j ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
			if (j < m_dimensions.y - 1) {
				Rgba8 const& adjColor = m_mapImage->GetTexelColor( IntVec2( i, j + 1 ) );
				if (!(adjColor == color)) {
					AddVertsForLineSegment2D( mapAABB2Verts, Vec2( (float)i, (float)(j + 1) ), Vec2( (float)(i + 1), (float)(j + 1) ), lineThickness, Rgba8( 128, 128, 128 ) );
				}
			}
		}
	}
	m_numOfEdgeVerts = (int)mapAABB2Verts.size() - m_startOfEdge;

	m_startOfRiver = (int)mapAABB2Verts.size();
	for (int i = 0; i < m_dimensions.x; i++) {
		for (int j = 0; j < m_dimensions.y; j++) {
			Rgba8 const& color = m_riverImage->GetTexelColor( IntVec2( i, j ) );
			if (color == Rgba8( 0, 0, 255 )) {
				AddVertsForAABB2D( mapAABB2Verts, AABB2( Vec2( (float)i, (float)j ), Vec2( (float)i + 1.f, (float)j + 1.f ) ), Rgba8( 0, 0, 255, 255 ), AABB2::IDENTITY );
			}
		}
	}
	m_numOfRiverVerts = (int)mapAABB2Verts.size() - m_startOfRiver;

	
	//m_vertexBuffer = new VertexBuffer( mapAABB2Verts.size() );
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( mapAABB2Verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( mapAABB2Verts.data(), mapAABB2Verts.size() * sizeof( Vertex_PCU ), m_vertexBuffer );
	//g_theRenderer->BindVertexBuffer( m_vertexBuffer );
}

IntVec2 Map::GetMapPosFromWorldPos( Vec2 const& worldPos ) const
{
	return IntVec2( RoundDownToInt( worldPos.x ), RoundDownToInt( worldPos.y ) );
}

int Map::GetProvinceIdFromMapPos( IntVec2 const& mapPos ) const
{
	IntVec2 clampedMapPos = IntVec2( GetClamped( mapPos.x, 0, m_dimensions.x - 1 ), GetClamped( mapPos.y, 0, m_dimensions.y - 1 ) );
	Rgba8 const& pixelColor = m_mapImage->GetTexelColor( mapPos );
	return GetProvIdFromColor( pixelColor );
}

int Map::GetProvIdFromColor( Rgba8 const& color ) const
{
	if (color == Rgba8( 255, 255, 255 )) {
		return -1;
	}
	auto iter = m_mapFromColorToProvId.find( color );
	if (iter != m_mapFromColorToProvId.end()) {
		return iter->second;
	}
	ERROR_AND_DIE( Stringf( "Cannot find a province has color %d %d %d", color.r, color.g, color.b ) );
}

Province* Map::GetProvFromColor( Rgba8 const& color ) const
{
	auto iter = m_mapFromColorToProvId.find( color );
	if (iter != m_mapFromColorToProvId.end()) {
		return m_provinces[iter->second];
	}
	ERROR_AND_DIE( Stringf( "Cannot find a province has color %d %d %d", color.r, color.g, color.b ) );
}

void Map::GetForceRank( std::vector<Force*>& forceRank ) const
{
	forceRank.clear();
	std::vector<Force*> tempForceVector;
	for (auto force : m_forcesAsOrder) {
		if (force && force->isAlive() && force->m_nickName != g_gameConfigBlackboard.GetValue( "observerForce", "OB" )) {
			tempForceVector.push_back( force );
		}
	}
	std::sort( tempForceVector.begin(), tempForceVector.end(), []( Force* a, Force* b )->bool {
		return a->GetCorrectedEconomyPoint() + a->GetTotalArmySize() * 1000 > b->GetCorrectedEconomyPoint() + b->GetTotalArmySize() * 1000;
		} );

	int numOfForces = (int)tempForceVector.size() >= 8 ? 8 : (int)tempForceVector.size();
	for (int i = 0; i < numOfForces; i++) {
		forceRank.push_back( tempForceVector[i] );
	}
}

Force* Map::GetForceByNickName( std::string const& nickname ) const
{
	for (int i = 0; i < (int)m_forcesAsOrder.size(); i++) {
		if (m_forcesAsOrder[i] && m_forcesAsOrder[i]->isCalledNickName( nickname )) {
			return m_forcesAsOrder[i];
		}
	}
	return nullptr;
}

void Map::GetAllProvsArmyCanGo( std::vector<Province*>& provCanGo, Army const* army ) const
{
	provCanGo.clear();
	for (auto prov : m_provinces) {
		if (army->IsProvValidToGo( prov )) {
			provCanGo.push_back( prov );
		}
	}
	/*
	// can go to own position
	provCanGo.push_back( provIn );
	// if start from own province, can go any provinces belong to the force
	if (army->m_owner->isProvOwned( provIn )) {
		std::vector<Province*> curForceOwnedProvs = army->GetOwner()->GetOwnedProvs();
		provCanGo.reserve( curForceOwnedProvs.size() );
		for (auto prov : curForceOwnedProvs) {
			if (IsTwoProvsConnected( provIn, prov, army->m_owner ) && prov != provIn) {
				provCanGo.push_back( prov );
			}
		}
	}
	// can go to any adjacent provinces
	std::vector<Province*> adjacentProvs = provIn->GetAdjacentProvinces();
	for (auto prov : adjacentProvs) {
		if ((army->m_owner->isProvOwned( provIn ) && prov->m_owner != army->m_owner) || !army->m_owner->isProvOwned( provIn )) {
			provCanGo.push_back( prov );
		}
	}*/
}

bool Map::IsTwoProvsConnected( Province* prov1, Province* prov2, Force* ownedForce ) const
{
	if (prov1->m_owner != ownedForce || prov2->m_owner != ownedForce) {
		return false;
	}
	if (prov1 == prov2) {
		return true;
	}
	std::queue<Province*> provQue;
	int* flag = new int[m_provinces.size()];
	for (int i = 0; i < (int)m_provinces.size(); i++) {
		flag[i] = 0;
	}
	provQue.push( prov1 );
	while (!provQue.empty()) {
		Province* thisProv = provQue.front();
		provQue.pop();
		for (auto adjProv : thisProv->m_adjacentProvinces) {
			if (adjProv == prov2) {
				delete[] flag;
				return true;
			}
			else if (adjProv->m_owner == ownedForce && flag[adjProv->m_id] == 0) {
				flag[adjProv->m_id] = 1;
				provQue.push( adjProv );
			}
		}
	}

	delete[] flag;
	return false;
}

Link::Link( LinkType type, int id )
	:m_type(type), m_provIdToLink(id)
{

}
