#include "Game/PowerBuilding.hpp"
#include "Game/Game.hpp"
#include "Game/Resource.hpp"

PowerPlant::PowerPlant( IntVec2 const& LBCoords )
	:PowerBuilding(LBCoords)
	,m_burnTimer(PowerPlantEfficiency, g_theGame->m_gameClock)
{
	m_powerRange = 1.f;
	m_buildingType = BuildingType::PowerPlant;
	m_canProduce = true;
	m_powerGeneration = 900.f;
}

PowerPlant::~PowerPlant()
{

}

void PowerPlant::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForPowerPlant( verts, m_leftBottomCoords );
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( 1.f, 0.05f ) ), Rgba8( 0, 0, 0, 255 ) );
	AddVertsForAABB2D( verts, AABB2( m_leftBottomCoords, Vec2( m_leftBottomCoords ) + Vec2( m_burnTimer.GetElapsedFraction(), 0.05f ) ), Rgba8( 0, 255, 0, 255 ) );

 	AddVertsForPowerLink( verts, this );

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
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.5f ), Vec2( m_leftBottomCoords ) + Vec2( 1.f, 1.f ) ), 10.f, Stringf( "%d", m_resourceCount ), Rgba8::RED );
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

bool PowerPlant::IsFull()
{
	return m_resourceCount > PowerPlantMaxCapacity;
}

void PowerPlant::Produce()
{
	if (m_burnTimer.IsStopped()) {
		if (m_resourceCount > 0) {
			m_burnTimer.Start();
			--m_resourceCount;
		}
	}
	else {
		if (m_burnTimer.DecrementPeriodIfElapsed()) {
			if (m_resourceCount == 0) {
				m_burnTimer.Stop();
			}
			else {
				--m_resourceCount;
			}
		}
	}
	if (!m_burnTimer.IsStopped()) {
		m_powerOutput = true;
	}
	else {
		m_powerOutput = false;
	}
}

bool PowerPlant::AddResource( Resource* resource )
{
	// ToDo: change this hard code to xml data
	int numOfD = resource->GetNumOfBasicResource( 'D' );
	if (!IsFull() && numOfD > 0) {
		resource->m_isGarbage = true;
		m_resourceCount += numOfD;
		return true;
	}
	return false;
}

bool PowerPlant::AddResource( int resourceID )
{
	int numOfD = ProductDefinition::GetDefinition(resourceID).GetNumOfBasicResource( 'D' );
	if (!IsFull() && numOfD > 0) {
		m_resourceCount += numOfD;
		return true;
	}
	return false;
}

bool PowerPlant::CanResourceMoveInto( Resource* resource )
{
	if (IsFull()) {
		return false;
	}
	if (resource->GetNumOfBasicResource('D') == 0) {
		return false;
	}
	return true;
}

PowerNode::PowerNode( IntVec2 const& LBCoords )
	:PowerBuilding(LBCoords)
{
	m_buildingType = BuildingType::PowerNode;
}

PowerNode::~PowerNode()
{

}

void PowerNode::Render() const
{
	std::vector<Vertex_PCU> verts;
	DrawPowerNode( m_leftBottomCoords );
	AddVertsForPowerLink( verts, this );

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

Vec2 PowerNode::GetCenterPos() const
{
	return Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 1.5f );
}

void PowerNetwork::Update()
{
	m_netWorkPower = 0.f;
	m_powerConsumed = 0.f;
	m_powerGenerated = 0.f;
	for (auto building : m_buildingInNetWork) {
		if (building->m_powerInput) {
			m_netWorkPower -= building->m_powerConsumption;
			m_powerConsumed += building->m_powerConsumption;
		}
		if (building->m_powerOutput) {
			m_netWorkPower += building->m_powerGeneration;
			m_powerGenerated += building->m_powerGeneration;
		}
	}
}

bool PowerNetwork::SeparateNetwork( std::vector<PowerNetwork*>& out_networks )
{
	std::vector<PowerBuilding*> allProviderBuilding = m_buildingInNetWork;
	for (auto providerToInit : allProviderBuilding) {
		providerToInit->m_flagForCheck = false;
	}
	int numOfNetworks = 0;
	while ((int)allProviderBuilding.size() > 0) {
		PowerBuilding* testProvider = allProviderBuilding[0];
		std::vector<PowerBuilding*> curNetwork;
		BFSBuildNetwork( testProvider, curNetwork );
		if (numOfNetworks == 0) {
			// the first network, do nothing to the member variable network
			// remove them from all provider
			for (auto providerToRemove : curNetwork) {
				allProviderBuilding.erase( std::remove( allProviderBuilding.begin(), allProviderBuilding.end(), providerToRemove ) );
			}
		}
		else {
			PowerNetwork* newNetwork = new PowerNetwork();
			for (auto providerToRemove : curNetwork) {
				allProviderBuilding.erase( std::remove( allProviderBuilding.begin(), allProviderBuilding.end(), providerToRemove ) );
				m_buildingInNetWork.erase( std::remove( m_buildingInNetWork.begin(), m_buildingInNetWork.end(), providerToRemove ) );
				providerToRemove->m_netWork = newNetwork;
			}
			newNetwork->m_buildingInNetWork = curNetwork;
			out_networks.push_back( newNetwork );
		}
		++numOfNetworks;
	}
	if (numOfNetworks == 1 || numOfNetworks == 0) {
		return false;
	}
	return true;
}

bool PowerNetwork::IsEmpty() const
{
	return (int)m_buildingInNetWork.size() == 0;
}

void PowerNetwork::RemoveProvider( PowerBuilding* provider )
{
	m_buildingInNetWork.erase( std::remove( m_buildingInNetWork.begin(), m_buildingInNetWork.end(), provider ) );
}

// bool PowerNetwork::IsProviderInNetwork( IPowerProvider* provider ) const
// {
// 	return std::find( m_providerInNetWork.begin(), m_providerInNetWork.end(), provider ) != m_providerInNetWork.end();
// }

void PowerNetwork::JoinNetwork( PowerNetwork* networkToJoin )
{
	for (auto provider : networkToJoin->m_buildingInNetWork) {
		provider->m_netWork = this;
		m_buildingInNetWork.push_back( provider );
	}
}

float PowerNetwork::GetPowerPercentage() const
{
	if (m_powerGenerated == 0.f) {
		return 0.f;
	}
	if (m_powerConsumed > m_powerGenerated) {
		return m_powerGenerated / m_powerConsumed;
	}
	else {
		return 1.f;
	}
}

void PowerNetwork::BFSBuildNetwork( PowerBuilding* startProvider, std::vector<PowerBuilding*>& out_network )
{
	startProvider->m_flagForCheck = true;
	std::deque<PowerBuilding*> queue;
	queue.push_back( startProvider );
	while (!queue.empty()) {
		PowerBuilding* thisProvider = queue.front();
		queue.pop_front();
		out_network.push_back( thisProvider );
		for (auto testProvider : thisProvider->m_otherPowerBuildingInRange) {
			if (testProvider->m_flagForCheck == false) {
				testProvider->m_flagForCheck = true;
				queue.push_back( testProvider );
			}
		}
	}
}

PowerBuilding::PowerBuilding( IntVec2 const& LBCoords )
	:Building(LBCoords)
{

}

PowerBuilding::~PowerBuilding()
{

}

void IPowerBuilding::RemoveInRangeProvider( PowerBuilding* provider )
{
	m_otherPowerBuildingInRange.erase( std::remove_if( m_otherPowerBuildingInRange.begin(), m_otherPowerBuildingInRange.end(), [&]( PowerBuilding* building ) {return provider == building; } ), m_otherPowerBuildingInRange.end() );
}

