#include "Game/Logistics.hpp"
#include "Game/Resource.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Widget.hpp"
#include "Game/Conveyer.hpp"

std::vector<SFWidget*> g_filterUIs;
Timer* g_conveyorAnimTimer = nullptr;

std::vector<SFWidget*> g_constructionPanelUI;

Selector::Selector( IntVec2 const& LBCoords )
	:LogisticBuilding(LBCoords)
{
	m_buildingType = BuildingType::Selector;
	m_canProduce = true;
}

Selector::~Selector()
{

}

void Selector::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForSelector( verts, this, Rgba8::WHITE );
	//AddVertsForResource( verts, GetCenterPos(), ProductDefinition::GetDefinition( m_selectingTypeID ) );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	if (GetCurMap()->m_debugRenderBuildingStats) {
		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.5f ), Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) ), 10.f, Stringf( "%d", (int)m_resources.size() ), Rgba8::RED );
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
	DrawResource( GetCenterPos(), ProductDefinition::GetDefinition( m_selectingTypeID ) );
}

void Selector::Produce()
{
	if (m_resources.empty()) {
		return;
	}
	float m_curTime = g_theGame->m_gameClock->GetTotalSeconds();
	if (m_resources.front().m_enterTime + m_conveyTime <= m_curTime) {
		Resource* resourceToGive = m_resources.front().m_resource;
		if (resourceToGive->m_def.m_id == m_selectingTypeID ) {
			TryGenerateResource( m_dir, resourceToGive );
		}
		else {
			if (m_goLeft) {
				if (!TryGenerateResource( GetTurnLeftDir( m_dir ), resourceToGive )) {
					if (TryGenerateResource( GetTurnRightDir( m_dir ), resourceToGive )) {
						m_goLeft = true;
					}
				}
				else {
					m_goLeft = false;
				}
			}
			else {
				if (!TryGenerateResource( GetTurnRightDir( m_dir ), resourceToGive )) {
					if (TryGenerateResource( GetTurnLeftDir( m_dir ), resourceToGive )) {
						m_goLeft = false;
					}
				}
				else {
					m_goLeft = true;
				}
			}
		}
	}
}

bool Selector::CanResourceMoveInto( Resource* resource )
{
	return resource->m_conveyerOn->m_dir == m_dir;
}

bool Selector::ChooseFilter( EventArgs& args )
{
	m_selectingTypeID = args.GetValue( "productType", -1 );
	if (m_selectingTypeID == -1) {
		ERROR_AND_DIE( "Error in UI! Cannot set a resource ID of -1" );
	}
	return true;
}

// bool Selector::ChooseCircleFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Selector) {
// 		((Selector*)g_curChosenBuilding)->m_selectingTypeID = ResourceType::Circle;
// 	}
// 	return true;
// }
// 
// bool Selector::ChooseSquareFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Selector) {
// 		((Selector*)g_curChosenBuilding)->m_selectingTypeID = ResourceType::Square;
// 	}
// 	return true;
// }
// 
// bool Selector::ChooseTriangleFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Selector) {
// 		((Selector*)g_curChosenBuilding)->m_selectingTypeID = ResourceType::Triangle;
// 	}
// 	return true;
// }
// 
// bool Selector::ChooseHexagonFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Selector) {
// 		((Selector*)g_curChosenBuilding)->m_selectingTypeID = ResourceType::Hexagon;
// 	}
// 	return true;
// }
// 
// bool Selector::ChoosePentagonFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Selector) {
// 		((Selector*)g_curChosenBuilding)->m_selectingTypeID = ResourceType::Pentagon;
// 	}
// 	return true;
// }

Router::Router( IntVec2 const& LBCoords )
	:LogisticBuilding(LBCoords)
{
	m_buildingType = BuildingType::Router;
	m_canProduce = true;
}

Router::~Router()
{

}

void Router::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForRouter( verts, m_leftBottomCoords, Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	if (GetCurMap()->m_debugRenderBuildingStats) {
		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.5f ), Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) ), 10.f, Stringf( "%d", (int)m_resources.size() ), Rgba8::RED );
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
}

void Router::Produce()
{
	if (m_resources.empty()) {
		return;
	}
	float m_curTime = g_theGame->m_gameClock->GetTotalSeconds();
	if (m_resources.front().m_enterTime + m_conveyTime <= m_curTime) {
		Resource* resourceToGive = m_resources.front().m_resource;
		if (m_nextDir == ProcessDirection::Forward) {
			m_nextDir = ProcessDirection::Right;
			if (TryGenerateResource( m_dir, resourceToGive )) {
				return;
			}
		}
		if (m_nextDir == ProcessDirection::Right) {
			m_nextDir = ProcessDirection::Backward;
			if (TryGenerateResource( GetTurnRightDir( m_dir ), resourceToGive )) {
				return;
			}
		}
		if (m_nextDir == ProcessDirection::Backward) {
			m_nextDir = ProcessDirection::Left;
			if (TryGenerateResource( GetInversedDir( m_dir ), resourceToGive )) {
				return;
			}
		}
		if (m_nextDir == ProcessDirection::Left) {
			m_nextDir = ProcessDirection::Forward;
			if (TryGenerateResource( GetTurnLeftDir( m_dir ), resourceToGive )) {
				return;
			}
		}
	}
}

OverflowGate::OverflowGate( IntVec2 const& LBCoords )
	:LogisticBuilding(LBCoords)
{
	m_buildingType = BuildingType::OverflowGate;
	m_canProduce = true;
}

OverflowGate::~OverflowGate()
{

}

void OverflowGate::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForOverflowGate( verts, m_leftBottomCoords, m_dir, Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	if (GetCurMap()->m_debugRenderBuildingStats) {
		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.5f ), Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) ), 10.f, Stringf( "%d", (int)m_resources.size() ), Rgba8::RED );
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
}

void OverflowGate::Produce()
{
	if (m_resources.empty()) {
		return;
	}
	float m_curTime = g_theGame->m_gameClock->GetTotalSeconds();
	if (m_resources.front().m_enterTime + m_conveyTime <= m_curTime) {
		Resource* resourceToGive = m_resources.front().m_resource;
		// first check if forward is full
		if (TryGenerateResource( m_dir, resourceToGive )) {
			return;
		}
		
		// if the forward is full: overflow to left or right
		if (m_nextDir == ProcessDirection::Right) {
			m_nextDir = ProcessDirection::Left;
			if (TryGenerateResource( GetTurnRightDir( m_dir ), resourceToGive )) {
				return;
			}
		}
		if (m_nextDir == ProcessDirection::Left) {
			m_nextDir = ProcessDirection::Right;
			if (TryGenerateResource( GetTurnLeftDir( m_dir ), resourceToGive )) {
				return;
			}
		}
	}
}

bool OverflowGate::CanResourceMoveInto( Resource* resource )
{
	return resource->m_conveyerOn->m_dir == m_dir;
}

LogisticBuilding::LogisticBuilding( IntVec2 const& LBCoords )
	:Building(LBCoords)
{

}

LogisticBuilding::~LogisticBuilding()
{

}

bool LogisticBuilding::AddResource( Resource* resource )
{
	if (!IsFull()) {
		resource->m_conveyerOn = nullptr;
		m_resources.push_back( ResourceInBuilding() );
		m_resources[m_resources.size() - 1].m_resource = resource;
		m_resources[m_resources.size() - 1].m_enterTime = g_theGame->m_gameClock->GetTotalSeconds();
		resource->m_render = false;
		resource->m_centerPos = GetCenterPos();
		resource->m_movementTargetPos = resource->m_centerPos;
		return true;
	}
	return false;
}

bool LogisticBuilding::AddResource( int resourceID )
{
	if (!IsFull()) {
		Resource* newResource = new Resource(ProductDefinition::GetDefinition(resourceID));
		GetCurMap()->AddResourceToList( newResource );
		newResource->m_conveyerOn = nullptr;
		m_resources.push_back( ResourceInBuilding() );
		m_resources[m_resources.size() - 1].m_resource = newResource;
		m_resources[m_resources.size() - 1].m_enterTime = g_theGame->m_gameClock->GetTotalSeconds();
		newResource->m_render = false;
		newResource->m_centerPos = GetCenterPos();
		newResource->m_movementTargetPos = newResource->m_centerPos;
		return true;
	}
	return false;
}

bool LogisticBuilding::IsFull()
{
	return (int)m_resources.size() >= LogisticBuildingMaxCapacity;
}

bool LogisticBuilding::TryGenerateResource( Direction dir, Resource* resourceToGive )
{
	Map* map = GetCurMap();
	Vec2 generatePos = Vec2( m_leftBottomCoords + DirectionUnitIntVec[(int)dir] ) + InputVec[(int)dir];
	Building* buildingOnTile = map->GetBuildingFromCoords( map->GetTileCoordsFromPos( generatePos ) );
	if (buildingOnTile) {
		// go to conveyor
		if (buildingOnTile->m_buildingType == BuildingType::Conveyer && ((Conveyer*)buildingOnTile)->m_dir != GetInversedDir( dir )) {
			bool canSpawn = true;
			for (auto resource : map->m_resources) {
				if (resource && DoDiscsOverlap( resource->m_centerPos, RESOURCE_RADIUS, generatePos, RESOURCE_RADIUS )) {
					canSpawn = false;
				}
			}
			if (canSpawn) {
				resourceToGive->m_conveyerOn = (Conveyer*)buildingOnTile;
				resourceToGive->m_centerPos = generatePos;
				m_resources.pop_front();
				resourceToGive->m_render = true;
				return true;
			}
		}
		// go to building
		else if (buildingOnTile->AddResource( resourceToGive )) {
			m_resources.pop_front();
			return true;
		}
	}
	return false;
}

Exporter::Exporter( IntVec2 const& LBCoords )
	:LogisticBuilding(LBCoords)
{
	m_buildingType = BuildingType::Exporter;
	m_canProduce = true;
}

Exporter::~Exporter()
{

}

void Exporter::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForExporter( verts, m_leftBottomCoords, m_dir, Rgba8::WHITE );
	//AddVertsForResource( verts, GetCenterPos(),  ProductDefinition::GetDefinition( m_typeToExportID )  );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
	DrawResource( GetCenterPos(), ProductDefinition::GetDefinition( m_typeToExportID ) );
}

void Exporter::Produce()
{
	if (m_resources.empty()) {
		return;
	}
	float m_curTime = g_theGame->m_gameClock->GetTotalSeconds();
	if (m_resources.front().m_enterTime + m_conveyTime <= m_curTime) {
		Resource* resourceToGive = m_resources.front().m_resource;
		if (m_nextDir == ProcessDirection::Forward) {
			m_nextDir = ProcessDirection::Right;
			if (TryGenerateResource( m_dir, resourceToGive )) {
				return;
			}
		}
		if (m_nextDir == ProcessDirection::Right) {
			m_nextDir = ProcessDirection::Left;
			if (TryGenerateResource( GetTurnRightDir( m_dir ), resourceToGive )) {
				return;
			}
		}
		if (m_nextDir == ProcessDirection::Left) {
			m_nextDir = ProcessDirection::Forward;
			if (TryGenerateResource( GetTurnLeftDir( m_dir ), resourceToGive )) {
				return;
			}
		}
	}
}

bool Exporter::ChooseFilter( EventArgs& args )
{
	m_typeToExportID = args.GetValue( "productType", -1 );
	if (m_typeToExportID == -1) {
		ERROR_AND_DIE( "Error in UI! Cannot set a resource ID of -1" );
	}
	return true;
}

// bool Exporter::ChooseCircleFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Exporter) {
// 		((Exporter*)g_curChosenBuilding)->m_typeToExportID = ResourceType::Circle;
// 	}
// 	return true;
// }
// 
// bool Exporter::ChooseSquareFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Exporter) {
// 		((Exporter*)g_curChosenBuilding)->m_typeToExportID = ResourceType::Square;
// 	}
// 	return true;
// }
// 
// bool Exporter::ChooseTriangleFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Exporter) {
// 		((Exporter*)g_curChosenBuilding)->m_typeToExportID = ResourceType::Triangle;
// 	}
// 	return true;
// }
// 
// bool Exporter::ChooseHexagonFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Exporter) {
// 		((Exporter*)g_curChosenBuilding)->m_typeToExportID = ResourceType::Hexagon;
// 	}
// 	return true;
// }
// 
// bool Exporter::ChoosePentagonFilter( EventArgs& args )
// {
// 	UNUSED( args );
// 	if (g_curChosenBuilding->m_buildingType == BuildingType::Exporter) {
// 		((Exporter*)g_curChosenBuilding)->m_typeToExportID = ResourceType::Pentagon;
// 	}
// 	return true;
// }

Junction::Junction( IntVec2 const& LBCoords )
	:LogisticBuilding(LBCoords)
{
	m_buildingType = BuildingType::Junction;
	m_canProduce = true;
}

Junction::~Junction()
{

}

void Junction::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForJunction( verts, m_leftBottomCoords, Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void Junction::Produce()
{
	if (!m_resourcesForwardBack.empty()) {
		ProduceDirection( m_resourcesForwardBack, true );
	}
	if (!m_resourcesLeftRight.empty()) {
		ProduceDirection( m_resourcesLeftRight, false );
	}
}

bool Junction::AddResource( Resource* resource )
{
	// get resource direction
	Vec2 centerPos = GetCenterPos();
	Vec2 resourcePos = resource->m_centerPos;
	Vec2 displacement = resourcePos - centerPos;
	if (abs( DotProduct2D( displacement, DirectionUnitVec[(int)m_dir] ) ) > abs( DotProduct2D( displacement, DirectionUnitVec[(int)GetTurnLeftDir( m_dir )] ) )) {
		if ((int)m_resourcesForwardBack.size() >= LogisticBuildingMaxCapacity) {
			return false;
		}
		resource->m_conveyerOn = nullptr;
		m_resourcesForwardBack.push_back( ResourceInBuilding() );
		m_resourcesForwardBack[m_resourcesForwardBack.size() - 1].m_resource = resource;
		m_resourcesForwardBack[m_resourcesForwardBack.size() - 1].m_enterTime = g_theGame->m_gameClock->GetTotalSeconds();
		resource->m_render = false;
		resource->m_centerPos = GetCenterPos();
		resource->m_movementTargetPos = resource->m_centerPos;
		return true;
	}
	else {
		if ((int)m_resourcesLeftRight.size() >= LogisticBuildingMaxCapacity) {
			return false;
		}
		resource->m_conveyerOn = nullptr;
		m_resourcesLeftRight.push_back( ResourceInBuilding() );
		m_resourcesLeftRight[m_resourcesLeftRight.size() - 1].m_resource = resource;
		m_resourcesLeftRight[m_resourcesLeftRight.size() - 1].m_enterTime = g_theGame->m_gameClock->GetTotalSeconds();
		resource->m_render = false;
		resource->m_centerPos = GetCenterPos();
		resource->m_movementTargetPos = resource->m_centerPos;
		return true;
	}
	return true;
}

bool Junction::AddResource( int resourceID )
{
	UNUSED( resourceID );
	ERROR_AND_DIE( "Error! This function is problematic and should not be called!" );
	//return false;
}

bool Junction::IsFull()
{
	return (int)m_resourcesForwardBack.size() >= LogisticBuildingMaxCapacity && (int)m_resourcesLeftRight.size() >= LogisticBuildingMaxCapacity;
}

bool Junction::CanResourceMoveInto( Resource* resource )
{
	Vec2 centerPos = GetCenterPos();
	Vec2 resourcePos = resource->m_centerPos;
	Vec2 displacement = resourcePos - centerPos;
	if (abs( DotProduct2D( displacement, DirectionUnitVec[(int)m_dir] ) ) > abs( DotProduct2D( displacement, DirectionUnitVec[(int)GetTurnLeftDir( m_dir )] ) )) {
		if ((int)m_resourcesForwardBack.size() >= LogisticBuildingMaxCapacity) {
			return false;
		}
		return true;
	}
	else {
		if ((int)m_resourcesLeftRight.size() >= LogisticBuildingMaxCapacity) {
			return false;
		}
		return true;
	}
}

void Junction::ProduceDirection( std::deque<ResourceInBuilding>& array, bool isForwardBack )
{
	float m_curTime = g_theGame->m_gameClock->GetTotalSeconds();
	if (array.front().m_enterTime + m_conveyTime <= m_curTime) {
		Resource* resourceToGive = array.front().m_resource;
		if (isForwardBack) {
			if (TryGenerateResource( m_dir, resourceToGive, array )) {
				return;
			}
			if (TryGenerateResource( GetInversedDir( m_dir ), resourceToGive, array )) {
				return;
			}
		}
		else {
			if (TryGenerateResource( GetTurnRightDir( m_dir ), resourceToGive, array )) {
				return;
			}
			if (TryGenerateResource( GetTurnLeftDir( m_dir ), resourceToGive, array )) {
				return;
			}
		}
	}
}

bool Junction::TryGenerateResource( Direction dir, Resource* resourceToGive, std::deque<ResourceInBuilding>& array )
{
	Map* map = GetCurMap();
	Vec2 generatePos = Vec2( m_leftBottomCoords + DirectionUnitIntVec[(int)dir] ) + InputVec[(int)dir];
	Building* buildingOnTile = map->GetBuildingFromCoords( map->GetTileCoordsFromPos( generatePos ) );
	if (buildingOnTile) {
		// go to conveyor
		if (buildingOnTile->m_buildingType == BuildingType::Conveyer && ((Conveyer*)buildingOnTile)->m_dir != GetInversedDir( dir )) {
			bool canSpawn = true;
			for (auto resource : map->m_resources) {
				if (resource && DoDiscsOverlap( resource->m_centerPos, RESOURCE_RADIUS, generatePos, RESOURCE_RADIUS )) {
					canSpawn = false;
				}
			}
			if (canSpawn) {
				resourceToGive->m_conveyerOn = (Conveyer*)buildingOnTile;
				resourceToGive->m_centerPos = generatePos;
				array.pop_front();
				resourceToGive->m_render = true;
				return true;
			}
		}
		// go to building
		else if (buildingOnTile->AddResource( resourceToGive )) {
			array.pop_front();
			return true;
		}
	}
	return false;
}

Bridge::Bridge( IntVec2 const& LBCoords )
	:LogisticBuilding( LBCoords )
{
	m_buildingType = BuildingType::Bridge;
	m_canProduce = true;
}

Bridge::~Bridge()
{

}

void Bridge::Render() const
{
	std::vector<Vertex_PCU> verts;
	DrawBridge( m_leftBottomCoords, m_dir, m_isInput, Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void Bridge::Produce()
{
	if (!m_isInput && m_theOtherHead) {
		if (m_theOtherHead->m_resources.empty()) {
			return;
		}
		float m_curTime = g_theGame->m_gameClock->GetTotalSeconds();
		if (m_theOtherHead->m_resources.front().m_enterTime + m_conveyTime * m_bridgeLength <= m_curTime) {
			Resource* resourceToGive = m_theOtherHead->m_resources.front().m_resource;
			// first check if forward is full
			if (TryGenerateResource( GetInversedDir( m_dir ), resourceToGive )) {
				return;
			}
		}
	}
}

bool Bridge::AddResource( Resource* resource )
{
	if (!IsFull() && m_isInput) {
		resource->m_conveyerOn = nullptr;
		m_resources.push_back( ResourceInBuilding() );
		m_resources[m_resources.size() - 1].m_resource = resource;
		m_resources[m_resources.size() - 1].m_enterTime = g_theGame->m_gameClock->GetTotalSeconds();
		resource->m_render = false;
		resource->m_centerPos = GetCenterPos();
		resource->m_movementTargetPos = resource->m_centerPos;
		return true;
	}
	return false;
}

bool Bridge::AddResource( int resourceID )
{
	if (!IsFull() && m_isInput) {
		Resource* newResource = new Resource( ProductDefinition::GetDefinition( resourceID ) );
		GetCurMap()->AddResourceToList( newResource );
		newResource->m_conveyerOn = nullptr;
		m_resources.push_back( ResourceInBuilding() );
		m_resources[m_resources.size() - 1].m_resource = newResource;
		m_resources[m_resources.size() - 1].m_enterTime = g_theGame->m_gameClock->GetTotalSeconds();
		newResource->m_render = false;
		newResource->m_centerPos = GetCenterPos();
		newResource->m_movementTargetPos = newResource->m_centerPos;
		return true;
	}
	return false;
}

bool Bridge::CanResourceMoveInto( Resource* resource )
{
	if (!m_isInput) {
		return false;
	}
	return resource->m_conveyerOn->m_dir == m_dir && !IsFull();
}

bool Bridge::IsFull()
{
	return (int)m_resources.size() >= LogisticBuildingMaxCapacity * m_bridgeLength;
}

bool Bridge::TryGenerateResource( Direction dir, Resource* resourceToGive )
{
	Map* map = GetCurMap();
	Vec2 generatePos = Vec2( m_leftBottomCoords + DirectionUnitIntVec[(int)dir] ) + InputVec[(int)dir];
	Building* buildingOnTile = map->GetBuildingFromCoords( map->GetTileCoordsFromPos( generatePos ) );
	if (buildingOnTile) {
		// go to conveyor
		if (buildingOnTile->m_buildingType == BuildingType::Conveyer && ((Conveyer*)buildingOnTile)->m_dir != GetInversedDir( dir )) {
			bool canSpawn = true;
			for (auto resource : map->m_resources) {
				if (resource && DoDiscsOverlap( resource->m_centerPos, RESOURCE_RADIUS, generatePos, RESOURCE_RADIUS )) {
					canSpawn = false;
				}
			}
			if (canSpawn) {
				resourceToGive->m_conveyerOn = (Conveyer*)buildingOnTile;
				resourceToGive->m_centerPos = generatePos;
				m_theOtherHead->m_resources.pop_front();
				resourceToGive->m_render = true;
				return true;
			}
		}
		// go to building
		else if (buildingOnTile->AddResource( resourceToGive )) {
			m_theOtherHead->m_resources.pop_front();
			return true;
		}
	}
	return false;
}
