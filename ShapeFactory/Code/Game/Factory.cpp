#include "Game/Factory.hpp"
#include "Game/Resource.hpp"
#include "Game/Map.hpp"
#include "Game/Conveyer.hpp"
#include "Game/Game.hpp"


Factory::Factory( IntVec2 const& LBCoords )
	:PowerBuilding(LBCoords)
	,m_factoryClock(*g_theGame->m_gameClock)
	,m_productionTimer(1.f, &m_factoryClock)
{
	m_neverFull = true;
	m_powerInput = true;
	m_physicsBounds = GetPhysicsBounds();
	m_physicsBounds = AABB2( Vec2(), Vec2( 2.f, 2.f ) );
	m_position = GetCenterPos();
}

Factory::~Factory()
{

}

Vec2 Factory::GetCenterPos() const
{
	return Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f );
}

AABB2 Factory::GetPhysicsBounds() const
{
	return AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 2.f ) );
}

bool Factory::AddResource( Resource* resource )
{
	if ( CanAddResource(resource->m_def.m_id) && m_numOfResource[resource->m_def.m_id] < 5 * GetResourceNumNeededForSingleProduct(resource->m_def.m_id) ) {
		resource->m_conveyerOn = nullptr;
		++m_numOfResource[resource->m_def.m_id];
		resource->m_isGarbage = true;
		return true;
	}
	return false;
}

bool Factory::AddResource( int resourceID )
{
	if (CanAddResource(resourceID) && m_numOfResource[resourceID] < 5 * GetResourceNumNeededForSingleProduct(resourceID) ) {
		++m_numOfResource[resourceID];
		return true;
	}
	return false;
}

bool Factory::IsFull()
{
	return GetNumOfResources() >= FactoryMaxResource;
}

bool Factory::IsCoordsInBuilding( IntVec2 const& coords ) const
{
	if (coords == m_leftBottomCoords) {
		return true;
	}
	else if (coords == m_leftBottomCoords + IntVec2( 0, 1 )) {
		return true;
	}
	else if (coords == m_leftBottomCoords + IntVec2( 1, 0 )) {
		return true;
	}
	else if (coords == m_leftBottomCoords + IntVec2( 1, 1 )) {
		return true;
	}
	return false;
}

bool Factory::CanResourceMoveInto( Resource* resource )
{
	return CanAddResource( resource->m_def.m_id ) && m_numOfResource[resource->m_def.m_id] < 5 * GetResourceNumNeededForSingleProduct(resource->m_def.m_id) ;
}

bool Factory::ChooseProduct( EventArgs& args )
{
 	Map* map = GetCurMap();
	int productType = args.GetValue( "productType", -1 );
	if (productType == -1) {
		ERROR_AND_DIE( "Cannot have a product type -1" );
	}
	m_productionType = &ProductDefinition::GetDefinition( productType );
 	for (int i = 0; i < NumOfProductTypes; ++i) {
		for (int j = 0; j < (int)m_productionType->m_recipe.m_idCountPair.size(); ++j) {
			if (i != m_productionType->m_recipe.m_idCountPair[j].first) {
				map->m_numOfResource[i] += m_numOfResource[i];
				m_numOfResource[i] = 0;
			}
		}
 	}
	m_productionTimer.Stop();
	m_productionTimer.SetPeriodSeconds( m_productionType->m_recipe.m_productionTime );
 	m_numOfResourceInInventory = 0;
 	return true;
}

bool Factory::TryGenerateResource( Vec2 const& pos, Resource* resourceToGive )
{
	Map* map = GetCurMap();
	Vec2 const& generatePos = pos;
	Building* buildingOnTile = map->GetBuildingFromCoords( map->GetTileCoordsFromPos( generatePos ) );
	if (buildingOnTile) {
		// go to conveyor
		if (buildingOnTile->m_buildingType == BuildingType::Conveyer && !IsCoordsInBuilding(map->GetTileCoordsFromPos( generatePos ) + DirectionUnitIntVec[(int)((Conveyer*)buildingOnTile)->m_dir])) {
			bool canSpawn = true;
			for (auto resource : map->m_resources) {
				if (resource && DoDiscsOverlap( resource->m_centerPos, RESOURCE_RADIUS, generatePos, RESOURCE_RADIUS )) {
					canSpawn = false;
				}
			}
			if (canSpawn) {
				resourceToGive->m_conveyerOn = (Conveyer*)buildingOnTile;
				resourceToGive->m_centerPos = generatePos;
				resourceToGive->m_render = true;
				return true;
			}
		}
		// go to building
		else if (buildingOnTile->AddResource( resourceToGive )) {
			return true;
		}
	}
	return false;
}

int Factory::GetNumOfResources() const
{
	int sum = 0;
	for (int i = 0; i < NumOfProductTypes; ++i) {
		sum += m_numOfResource[i];
	}
	return sum;
}

bool Factory::CanAddResource( int resourceID ) const
{
	for (auto const& pair : m_productionType->m_recipe.m_idCountPair) {
		if (pair.first == resourceID) {
			return true;
		}
	}
	return false;
}

int Factory::GetResourceNumNeededForSingleProduct( int resourceID ) const
{
	for (int i = 0; i < (int)m_productionType->m_recipe.m_idCountPair.size(); ++i) {
		if (m_productionType->m_recipe.m_idCountPair[i].first == resourceID) {
			return m_productionType->m_recipe.m_idCountPair[i].second;
		}
	}
	return 0;
}

Blender::Blender( IntVec2 const& LBCoords )
	:Factory( LBCoords )
	,m_sprite(SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Buildings/blender.png"), IntVec2(8, 4)))
	,m_animationSprite( m_sprite, 0, 31, 3.f, SpriteAnimPlaybackType::LOOP)
{
	m_buildingType = BuildingType::Blender;
	m_canProduce = true;
	m_powerRange = 0.f;
	m_productionType = &ProductDefinition::GetNullDef();
}

Blender::~Blender()
{

}

void Blender::Render() const
{
	std::vector<Vertex_PCU> verts;
	DrawBlender( this, m_leftBottomCoords, Rgba8::WHITE );
	AddVertsForPowerLink( verts, this );
	//AddVertsForResource( verts, GetCenterPos(), *m_productionType );
	AddVertsForAABB2D( verts, AABB2( GetCenterPos() - Vec2( RESOURCE_RADIUS, RESOURCE_RADIUS ), GetCenterPos() + Vec2( RESOURCE_RADIUS, RESOURCE_RADIUS ) ), Rgba8(255, 255, 255, 200) );
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 0.05f ) ), Rgba8( 0, 0, 0, 255 ) );
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( m_productionTimer.GetElapsedFraction() * 2.f, 0.05f ) ), Rgba8( 0, 255, 0, 255 ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	if (m_productionType->m_id != -1) {
		DrawResource( GetCenterPos(), *m_productionType );
	}
}

Refinery::Refinery( IntVec2 const& LBCoords )
	:Factory( LBCoords )
{
	m_buildingType = BuildingType::Refinery;
	m_canProduce = true;
	m_powerRange = 0.f;
	m_productionType = &ProductDefinition::GetNullDef();
}

Refinery::~Refinery()
{

}

void Refinery::Render() const
{
	std::vector<Vertex_PCU> verts;
	DrawRefinery( m_leftBottomCoords, Rgba8::WHITE );
	AddVertsForPowerLink( verts, this );
	//AddVertsForResource( verts, GetCenterPos(), *m_productionType );
	AddVertsForAABB2D( verts, AABB2( GetCenterPos() - Vec2( RESOURCE_RADIUS, RESOURCE_RADIUS ), GetCenterPos() + Vec2( RESOURCE_RADIUS, RESOURCE_RADIUS ) ), Rgba8( 255, 255, 255, 200 ) );
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 0.05f ) ), Rgba8( 0, 0, 0, 255 ) );
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( m_productionTimer.GetElapsedFraction() * 2.f, 0.05f ) ), Rgba8( 0, 255, 0, 255 ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	if (m_productionType->m_id != -1) {
		DrawResource( GetCenterPos(), *m_productionType );
	}
}

void Factory::Produce()
{
	if (m_productionType->m_id == -1) {
		return;
	}
	//Map* map = GetCurMap();
	m_factoryClock.SetTimeScale( m_netWork->GetPowerPercentage() );

	if (!m_productionTimer.IsStopped() && !m_productionTimer.IsPaused()) {
		m_powerConsumption = FactoryEnergyConsumption;
		m_animationTime += m_factoryClock.GetDeltaSeconds();
	}
	else {
		m_powerConsumption = 0.f;
	}

	if (m_productionTimer.IsStopped()) {
		if (m_numOfResourceInInventory < 3 && DoProductionConsumeResource()) {
			m_productionTimer.Start();
		}
	}
	else {
		if (m_productionTimer.DecrementPeriodIfElapsed()) {
			m_numOfResourceInInventory += m_productionType->m_recipe.m_productCount;
			if (m_numOfResourceInInventory < m_productionType->m_recipe.m_productCount * 5) {
				if (!DoProductionConsumeResource()) {
					m_productionTimer.Stop();
				}
			}
			else {
				m_productionTimer.Stop();
			}
		}
	}

	if (m_numOfResourceInInventory > 0) {
		int beforeTryPos = m_curTryPos;
		do {
			if ((this->*m_generationFuncList[m_curTryPos])(m_productionType->m_id)) {
				m_curTryPos = (m_curTryPos + 1) % 8;
				--m_numOfResourceInInventory;
				break;
			}
			m_curTryPos = (m_curTryPos + 1) % 8;
		} while (beforeTryPos != m_curTryPos);
	}
}

bool Factory::DoProductionConsumeResource()
{
	for (int i = 0; i < (int)m_productionType->m_recipe.m_idCountPair.size(); ++i) {
		int resourceID = m_productionType->m_recipe.m_idCountPair[i].first;
		int numNeeded = m_productionType->m_recipe.m_idCountPair[i].second;
		if (m_numOfResource[resourceID] < numNeeded) {
			return false;
		}
	}
	for (int i = 0; i < (int)m_productionType->m_recipe.m_idCountPair.size(); ++i) {
		int resourceID = m_productionType->m_recipe.m_idCountPair[i].first;
		int numNeeded = m_productionType->m_recipe.m_idCountPair[i].second;
		m_numOfResource[resourceID] -= numNeeded;
	}

	return true;
}
