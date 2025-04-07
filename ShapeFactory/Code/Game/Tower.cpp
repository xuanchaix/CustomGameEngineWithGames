#include "Game/Tower.hpp"
#include "Game/Resource.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Projectile.hpp"

TowerBase::TowerBase( IntVec2 const& LBCoords )
	:PowerBuilding(LBCoords)
	,m_towerClock(*g_theGame->m_gameClock)
	,m_shootTimer(1.f, &m_towerClock)
{
	m_powerInput = true;
	m_neverFull = true;
	m_canProduce = true;
	m_powerRange = 0.f;
}

TowerBase::~TowerBase()
{

}

bool TowerBase::IsFull()
{
	return false;
}

bool TowerBase::AddResource( Resource* resource )
{
	if (CanAddResource( resource->m_def ) && m_numOfResource[resource->m_def.m_id] < 50) {
		resource->m_conveyerOn = nullptr;
		++m_numOfResource[resource->m_def.m_id];
		resource->m_isGarbage = true;
		return true;
	}
	return false;
}

bool TowerBase::AddResource( int resourceID )
{
	if (CanAddResource( ProductDefinition::GetDefinition(resourceID) ) && m_numOfResource[resourceID] < 50) {
		++m_numOfResource[resourceID];
		return true;
	}
	return false;
}

bool TowerBase::CanAddResource( ProductDefinition const& resourceDef )
{
	return resourceDef.m_isProjectile;
}

bool TowerBase::CanResourceMoveInto( Resource* resource )
{
	return CanAddResource( resource->m_def ) && m_numOfResource[resource->m_def.m_id] < 50;
}

bool TowerBase::DoHaveProjectileProducts() const
{
	for (int i = 0; i < NumOfProductTypes; ++i) {
		if (m_numOfResource[i] > 0 && ProductDefinition::GetDefinition( i ).m_isProjectile) {
			return true;
		}
	}
	return false;
}

int TowerBase::ConsumeProjectile()
{
	for (int i = 0; i < NumOfProductTypes; ++i) {
		if (m_numOfResource[i] > 0 && ProductDefinition::GetDefinition( i ).m_isProjectile) {
			--m_numOfResource[i];
			return i;
		}
	}
	return -1;
}

Mortar::Mortar( IntVec2 const& LBCoords )
	:TowerBase(LBCoords)
{
	m_shootTimer.SetPeriodSeconds( 2.f );
	m_buildingType = BuildingType::Mortar;
	//m_physicsBounds = GetPhysicsBounds();
	m_physicsBounds = AABB2( Vec2(), Vec2( 2.f, 2.f ) );
	m_position = GetCenterPos();
}

Mortar::~Mortar()
{

}

void Mortar::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 2.f ) ), Rgba8( 255, 255, 255, 200 ) );
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

void Mortar::Produce()
{
	Shoot();
}

void Mortar::Shoot()
{

}

Vec2 Mortar::GetCenterPos() const
{
	return Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f );
}

AABB2 Mortar::GetPhysicsBounds() const
{
	return AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 2.f ) );
}

GuidedMissile::GuidedMissile( IntVec2 const& LBCoords )
	:TowerBase( LBCoords )
{
	m_buildingType = BuildingType::GuidedMissile;
	//m_physicsBounds = GetPhysicsBounds();
	m_physicsBounds = AABB2( Vec2(), Vec2( 2.f, 2.f ) );
	m_position = GetCenterPos();

}

GuidedMissile::~GuidedMissile()
{

}

void GuidedMissile::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 2.f ) ), Rgba8( 255, 255, 255, 200 ) );
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

void GuidedMissile::Produce()
{
	Shoot();
}

void GuidedMissile::Shoot()
{

}

Vec2 GuidedMissile::GetCenterPos() const
{
	return Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f );
}

AABB2 GuidedMissile::GetPhysicsBounds() const
{
	return AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 2.f, 2.f ) );
}

StraightArcher::StraightArcher( IntVec2 const& LBCoords )
	:TowerBase(LBCoords)
{
	m_buildingType = BuildingType::StraightArcher;
	m_shootTimer.SetPeriodSeconds( 0.5f );
	m_shootTimer.Start();
}

StraightArcher::~StraightArcher()
{

}

void StraightArcher::Render() const
{
	DrawStraightArcher( m_leftBottomCoords, m_dir );
}

void StraightArcher::Produce()
{
	Shoot();
}

void StraightArcher::Shoot()
{
	//Map* map = GetCurMap();
	m_towerClock.SetTimeScale( m_netWork->GetPowerPercentage() );

	if (!m_shootTimer.IsStopped() && !m_shootTimer.IsPaused()) {
		m_powerConsumption = 100.f;
	}
	else {
		m_powerConsumption = 0.f;
	}

	if (m_shootTimer.IsStopped()) {
		if (DoHaveProjectileProducts() || m_numOfProjectileLeft > 0) {
			m_shootTimer.Start();
		}
	}
	else {
		if (m_shootTimer.DecrementPeriodIfElapsed()) {
			if (m_numOfProjectileLeft > 0) {
				--m_numOfProjectileLeft;
				ProjectileDefinition const& def = ProjectileDefinition::GetDefinition( m_curProjectileType );
				GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ), def );
			}
			else {
				int productID = ConsumeProjectile();
				if (productID == -1) {
					m_shootTimer.Stop();
				}
				else {
					ProductDefinition const& productDef = ProductDefinition::GetDefinition( productID );
					ProjectileDefinition const& def = ProjectileDefinition::GetDefinition( productDef.m_normalProjectileID );
					m_curProjectileType = productDef.m_normalProjectileID;
					m_numOfProjectileLeft = def.m_projectileCount - 1;
					GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ), def );
					if (!DoHaveProjectileProducts()) {
						m_shootTimer.Stop();
					}
				}
			}
		}
	}
}

ThreeDirectionsPike::ThreeDirectionsPike( IntVec2 const& LBCoords )
	:TowerBase( LBCoords )
{
	m_shootTimer.SetPeriodSeconds( 1.1f );
	m_shootTimer.Start();
	m_buildingType = BuildingType::ThreeDirectionsPike;
}

ThreeDirectionsPike::~ThreeDirectionsPike()
{

}

void ThreeDirectionsPike::Render() const
{
	DrawThreeDirectionPike( m_leftBottomCoords, m_dir );
}

void ThreeDirectionsPike::Produce()
{
	Shoot();
}

void ThreeDirectionsPike::Shoot()
{
	//Map* map = GetCurMap();
	m_towerClock.SetTimeScale( m_netWork->GetPowerPercentage() );

	if (!m_shootTimer.IsStopped() && !m_shootTimer.IsPaused()) {
		m_powerConsumption = 450.f;
	}
	else {
		m_powerConsumption = 0.f;
	}

	if (m_shootTimer.IsStopped()) {
		if (DoHaveProjectileProducts() || m_numOfProjectileLeft > 0) {
			m_shootTimer.Start();
		}
	}
	else {
		if (m_shootTimer.DecrementPeriodIfElapsed()) {
			if (m_numOfProjectileLeft > 0) {
				--m_numOfProjectileLeft;
				ProjectileDefinition const& def = ProjectileDefinition::GetDefinition( m_curProjectileType );
				GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ), def );
				GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ) + 15.f, def );
				GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ) - 15.f, def );
			}
			else {
				int productID = ConsumeProjectile();
				if (productID == -1) {
					m_shootTimer.Stop();
				}
				else {
					ProductDefinition const& productDef = ProductDefinition::GetDefinition( productID );
					ProjectileDefinition const& def = ProjectileDefinition::GetDefinition( productDef.m_normalProjectileID );
					m_curProjectileType = productDef.m_normalProjectileID;
					m_numOfProjectileLeft = def.m_projectileCount - 1;
					GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ), def );
					GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ) + 15.f, def );
					GetCurMap()->CreateNewProjectile( GetCenterPos() + Vec2( 0.f, 0.3f ), GetOrientationDegreesFromDir( m_dir ) - 15.f, def );
					if (!DoHaveProjectileProducts()) {
						m_shootTimer.Stop();
					}
				}
			}
		}
	}
}

Laser::Laser( IntVec2 const& LBCoords )
	:TowerBase( LBCoords )
{
	m_buildingType = BuildingType::Laser;

}

Laser::~Laser()
{

}

void Laser::Render() const
{

}

void Laser::Produce()
{
	Shoot();
}

void Laser::Shoot()
{

}

Wall::Wall( IntVec2 const& LBCoords )
	:Building(LBCoords)
{
	m_buildingType = BuildingType::Wall;

}

Wall::~Wall()
{

}

void Wall::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) ), Rgba8( 255, 255, 255, 200 ) );
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
