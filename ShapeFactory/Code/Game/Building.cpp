#include "Game/Building.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Resource.hpp"
#include "Game/Conveyer.hpp"
#include "Game/Logistics.hpp"

Base::Base( IntVec2 const& LBCoords )
	:Building(LBCoords)
{ 
	m_buildingType = BuildingType::Base;
	m_canProduce = true;
	//m_physicsBounds = GetPhysicsBounds();
	m_physicsBounds = AABB2( Vec2(), Vec2( 3.f, 3.f ) );
	m_position = GetCenterPos();
}

Base::~Base()
{

}

void Base::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 3.f, 3.f ) ), Rgba8( 255, 255, 255 ), AABB2( Vec2( 0.f, 0.1f ), Vec2( 1.f, 1.f ) ) );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/base.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

bool Base::AddResource( Resource* resource )
{
	resource->m_isGarbage = true;
	resource->m_isCollected = true;
	return true;
}

bool Base::AddResource( int resourceID )
{
	GetCurMap()->m_numOfResource[resourceID]++;
	return true;
}

void Base::Produce()
{
	Map* map = GetCurMap();
	Building* buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 3, 0 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 3, 1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 3, 2 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( -1, 0 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( -1, 1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( -1, 2 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 0, -1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 1, -1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 2, -1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 0, 3 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 1, 3 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 2, 3 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
}

Vec2 Base::GetCenterPos() const
{
	return Vec2( m_leftBottomCoords ) + Vec2( 1.5f, 1.5f );
}

AABB2 Base::GetPhysicsBounds() const
{
	return AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 3.f, 3.f ) );
}

void Base::CheckBuildingToExport( Building* building )
{
	if (building->m_buildingType == BuildingType::Exporter) {
		Map* map = GetCurMap();
		if (!((Exporter*)building)->IsFull() && map->m_numOfResource[(int)((Exporter*)building)->m_typeToExportID] > 0) {
			((Exporter*)building)->AddResource( ((Exporter*)building)->m_typeToExportID );
			--map->m_numOfResource[(int)((Exporter*)building)->m_typeToExportID];
		}
	}
}

Building::Building( IntVec2 const& LBCoords )
	:Entity(GetCenterPos() , g_theGame)
	,m_leftBottomCoords(LBCoords)
{
	m_entityType = EntityType::Building;
	m_health = 15.f;
	m_maxHealth = 15.f;
	m_tileLBOn = g_theGame->m_map->GetTileFromCoords( LBCoords );
	//m_physicsBounds = GetPhysicsBounds();
	m_physicsBounds = AABB2( Vec2(), Vec2( 1.f, 1.f ) );
	m_position = GetCenterPos();
}

Building::~Building()
{

}

void Building::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_health <= 0.f) {
		m_isDead = true;
		return;
	}
}

void Building::Die()
{
	m_isDead = true;
}

void Building::Produce()
{

}

bool Building::IsFull()
{
	return false;
}

Vec2 Building::GetCenterPos() const
{
	return Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.5f );
}

bool Building::AddResource( Resource* resource )
{
	UNUSED( resource );
	return false;
}

bool Building::AddResource( int resourceID )
{
	UNUSED( resourceID );
	return false;
}

AABB2 Building::GetPhysicsBounds() const
{
	return AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) );
}

void Building::BuildingAddVertsForHealthBar(std::vector<Vertex_PCU>& verts) const
{
	if (!m_hasHealthBar || m_health == m_maxHealth) {
		return;
	}

	AddVertsForHealthBar( verts, this );
}

Drill::Drill( IntVec2 const& LBCoords )
	:Building( LBCoords )
	,m_drillingSpeed(2.f)
	,m_productionTimer(m_drillingSpeed ,g_theGame->m_gameClock)
{
	m_canProduce = true;
	m_buildingType = BuildingType::Drill;
	m_productionTimer.Start();
}

Drill::~Drill()
{

}

void Drill::Produce()
{
	if (m_numInInventory > 0) {
		Map* map = GetCurMap();
		bool res = map->CreateNewResource( m_drillingType, Vec2( m_leftBottomCoords + DirectionUnitIntVec[(int)m_dir] ) + InputVec[(int)m_dir], this );
		if (res) {
			--m_numInInventory;
		}
	}
	if (m_productionTimer.DecrementPeriodIfElapsed()) {
		// produce one product
		Map* map = GetCurMap();
		bool res = map->CreateNewResource( m_drillingType, Vec2(m_leftBottomCoords + DirectionUnitIntVec[(int)m_dir]) + InputVec[(int)m_dir], this );
		if (!res && m_numInInventory < 5) {
			++m_numInInventory;
		}
	}
}

void Drill::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForDrill( verts, m_leftBottomCoords, m_dir, Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.5f ), Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) ), 10.f, Stringf( "%d", m_numInInventory ), Rgba8::RED );
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

bool Drill::IsFull()
{
	if (m_numInInventory >= 5) {
		return true;
	}
	return false;
}

bool Drill::AddResource( Resource* resource )
{
	if (resource->m_def.m_id == m_drillingType && !IsFull()) {
		resource->m_isGarbage = true;
		m_numInInventory++;
		return true;
	}
	return false;
}

bool Drill::AddResource( int resourceID )
{
	if (resourceID == m_drillingType && !IsFull()) {
		m_numInInventory++;
		return true;
	}
	return false;
}

bool Drill::CanResourceMoveInto( Resource* resource )
{
	return resource->m_conveyerOn->m_dir != GetInversedDir( m_dir );
}

WareHouse::WareHouse( IntVec2 const& LBCoords )
	:Building(LBCoords)
{
	m_buildingType = BuildingType::WareHouse;
	m_canProduce = true;
}

WareHouse::~WareHouse()
{

}

void WareHouse::Render() const
{
	std::vector<Vertex_PCU> verts;
	DrawWareHouse( m_leftBottomCoords, Rgba8::WHITE );

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

void WareHouse::Produce()
{
	Map* map = GetCurMap();
	Building* buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 1, 0 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( -1, 0 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 0, 1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
	buildingNextTo = map->GetBuildingFromCoords( m_leftBottomCoords + IntVec2( 0, -1 ) );
	if (buildingNextTo) {
		CheckBuildingToExport( buildingNextTo );
	}
}

bool WareHouse::AddResource( Resource* resource )
{
	resource->m_isGarbage = true;
	m_numOfResource[(int)resource->m_def.m_id]++;
	return true;
}

bool WareHouse::AddResource( int resourceID )
{
	m_numOfResource[resourceID]++;
	return true;
}

void WareHouse::CheckBuildingToExport( Building* building )
{
	if (building->m_buildingType == BuildingType::Exporter) {
		if (!((Exporter*)building)->IsFull() && m_numOfResource[(int)((Exporter*)building)->m_typeToExportID] > 0) {
			((Exporter*)building)->AddResource( ((Exporter*)building)->m_typeToExportID );
			--m_numOfResource[(int)((Exporter*)building)->m_typeToExportID];
		}
	}
}
