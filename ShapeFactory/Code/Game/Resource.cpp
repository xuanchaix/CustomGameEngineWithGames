#include "Game/Resource.hpp"
#include "Game/Conveyer.hpp"
#include "Game/Map.hpp"
#include "Game/Logistics.hpp"
#include "Game/PowerBuilding.hpp"
#include "Game/Factory.hpp"
#include "Game/Tower.hpp"

Resource::Resource( ProductDefinition const& def )
	:m_def(def)
{

}

void Resource::Update()
{

}

void Resource::Render() const
{
	if (!m_render) {
		return;
	}
	std::vector<Vertex_PCU> verts;

	DrawResource( m_centerPos, m_def );
	//AddVertsForResource( verts, m_centerPos, m_def );

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

void Resource::PreMove( Conveyer* conveyerOn, float deltaSeconds )
{
	m_hasPriority = false;
	if (!conveyerOn) {
		return;
	}
	Vec2 moveDir;
	// turning
	constexpr float turningExtraSpeed = 1.6f;
	if ((conveyerOn->m_dir == Direction::Down || conveyerOn->m_dir == Direction::Up)) {
		if (m_centerPos.x < (float)conveyerOn->m_leftBottomCoords.x + 0.5f) {
			moveDir = Vec2( 1.f, 0.f );
			m_movementTargetPos = m_centerPos + moveDir * deltaSeconds * conveyerOn->m_conveySpeed * turningExtraSpeed;
			if (m_movementTargetPos.x > ( float )conveyerOn->m_leftBottomCoords.x + 0.5f) {
				float diff = m_movementTargetPos.x - (float)conveyerOn->m_leftBottomCoords.x - 0.5f;
				m_movementTargetPos.x = (float)conveyerOn->m_leftBottomCoords.x + 0.5f;
				if (conveyerOn->m_dir == Direction::Down) {
					m_movementTargetPos.y = m_centerPos.y - diff;
				}
				else {
					m_movementTargetPos.y = m_centerPos.y + diff;
				}
			}
			return;
		}
		else if (m_centerPos.x > (float)conveyerOn->m_leftBottomCoords.x + 0.5f) {
			moveDir = Vec2( -1.f, 0.f );
			m_movementTargetPos = m_centerPos + moveDir * deltaSeconds * conveyerOn->m_conveySpeed * turningExtraSpeed;
			if (m_movementTargetPos.x < (float)conveyerOn->m_leftBottomCoords.x + 0.5f) {
				float diff = -m_movementTargetPos.x + (float)conveyerOn->m_leftBottomCoords.x + 0.5f;
				m_movementTargetPos.x = (float)conveyerOn->m_leftBottomCoords.x + 0.5f;
				if (conveyerOn->m_dir == Direction::Down) {
					m_movementTargetPos.y = m_centerPos.y - diff;
				}
				else {
					m_movementTargetPos.y = m_centerPos.y + diff;
				}
			}
			return;
		}
	}
	else if ((conveyerOn->m_dir == Direction::Left || conveyerOn->m_dir == Direction::Right)) {
		if (m_centerPos.y < (float)conveyerOn->m_leftBottomCoords.y + 0.5f) {
			moveDir = Vec2( 0.f, 1.f );
			m_movementTargetPos = m_centerPos + moveDir * deltaSeconds * conveyerOn->m_conveySpeed * turningExtraSpeed;
			if (m_movementTargetPos.y > ( float )conveyerOn->m_leftBottomCoords.y + 0.5f) {
				float diff = m_movementTargetPos.y - (float)conveyerOn->m_leftBottomCoords.y - 0.5f;
				m_movementTargetPos.y = (float)conveyerOn->m_leftBottomCoords.y + 0.5f;
				if (conveyerOn->m_dir == Direction::Left) {
					m_movementTargetPos.x = m_centerPos.x - diff;
				}
				else {
					m_movementTargetPos.x = m_centerPos.x + diff;
				}
			}
			return;
		}
		else if (m_centerPos.y > (float)conveyerOn->m_leftBottomCoords.y + 0.5f) {
			moveDir = Vec2( 0.f, -1.f );
			m_movementTargetPos = m_centerPos + moveDir * deltaSeconds * conveyerOn->m_conveySpeed * turningExtraSpeed;
			if (m_movementTargetPos.y < (float)conveyerOn->m_leftBottomCoords.y + 0.5f) {
				float diff = -m_movementTargetPos.y + (float)conveyerOn->m_leftBottomCoords.y + 0.5f;
				m_movementTargetPos.y = (float)conveyerOn->m_leftBottomCoords.y + 0.5f;
				if (conveyerOn->m_dir == Direction::Left) {
					m_movementTargetPos.x = m_centerPos.x - diff;
				}
				else {
					m_movementTargetPos.x = m_centerPos.x + diff;
				}
			}
			return;
		}
	}
	if (conveyerOn->m_dir == Direction::Down) {
		moveDir = Vec2( 0.f, -1.f );
	}
	else if (conveyerOn->m_dir == Direction::Up) {
		moveDir = Vec2( 0.f, 1.f );
	}
	else if (conveyerOn->m_dir == Direction::Left) {
		moveDir = Vec2( -1.f, 0.f );
	}
	else if (conveyerOn->m_dir == Direction::Right) {
		moveDir = Vec2( 1.f, 0.f );
	}
	m_movementTargetPos = m_centerPos + moveDir * deltaSeconds * conveyerOn->m_conveySpeed;

	if (m_conveyerOn) {
		if ((m_conveyerOn->m_dir == Direction::Up || m_conveyerOn->m_dir == Direction::Down)
			&& m_centerPos.x == (float)m_conveyerOn->m_leftBottomCoords.x + 0.5f) {
			m_hasPriority = true;
		}
		if ((m_conveyerOn->m_dir == Direction::Left || m_conveyerOn->m_dir == Direction::Right)
			&& m_centerPos.y == (float)m_conveyerOn->m_leftBottomCoords.y + 0.5f) {
			m_hasPriority = true;
		}
	}
	else {
		m_hasPriority = false;
	}
}

void Resource::ReconsiderMovement( Resource* ignoreResource )
{
	if (m_movementTargetPos == m_centerPos) {
		return;
	}
	m_movementTargetPos = m_centerPos;
	Map* map = GetCurMap();
	for (int i = 0; i < (int)map->m_resources.size(); ++i) {
		if (map->m_resources[i] && map->m_resources[i] != this && map->m_resources[i] != ignoreResource && map->m_resources[i]->m_conveyerOn != nullptr) {
			Resource* otherResource = map->m_resources[i];
			if (DoDiscsOverlap( m_centerPos, RESOURCE_RADIUS, otherResource->m_movementTargetPos, RESOURCE_RADIUS ) && (!otherResource->m_hasPriority || (m_hasPriority && otherResource->m_hasPriority))) {
				otherResource->ReconsiderMovement( this );
			}
		}
	}
}

void Resource::Move()
{
	if (!m_conveyerOn) {
		return;
	}
	m_centerPos = m_movementTargetPos;
	Building* buildingOn = GetCurMap()->GetBuildingFromCoords( GetCurMap()->GetTileCoordsFromPos( m_centerPos ) );
	if (!buildingOn) {
		m_isCollected = true;
		m_isGarbage = true;
		return;
	}
	if (buildingOn->m_buildingType == BuildingType::Conveyer) {
		m_conveyerOn = (Conveyer*)buildingOn;
	}
	else if (buildingOn->m_buildingType == BuildingType::Base) {
		buildingOn->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::Selector) {
		if (((Selector*)buildingOn)->CanResourceMoveInto( this )) {
			((Selector*)buildingOn)->AddResource( this );
		}
	}
	else if (buildingOn->m_buildingType == BuildingType::Router) {
		((Router*)buildingOn)->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::Drill) {
		if (((Drill*)buildingOn)->CanResourceMoveInto( this )) {
			((Drill*)buildingOn)->AddResource( this );
		}
	}
	else if (buildingOn->m_buildingType == BuildingType::OverflowGate) {
		if (((OverflowGate*)buildingOn)->CanResourceMoveInto( this )) {
			((OverflowGate*)buildingOn)->AddResource( this );
		}
	}
	else if (buildingOn->m_buildingType == BuildingType::WareHouse) {
		((WareHouse*)buildingOn)->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::Junction) {
		((Junction*)buildingOn)->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::Bridge) {
		if (((Bridge*)buildingOn)->CanResourceMoveInto( this )) {
			((Bridge*)buildingOn)->AddResource( this );
		}
	}
	else if (buildingOn->m_buildingType == BuildingType::PowerPlant) {
		((PowerPlant*)buildingOn)->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::Refinery) {
		((Refinery*)buildingOn)->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::Blender) {
		((Blender*)buildingOn)->AddResource( this );
	}
	else if (buildingOn->m_buildingType == BuildingType::StraightArcher
		|| buildingOn->m_buildingType == BuildingType::GuidedMissile
		|| buildingOn->m_buildingType == BuildingType::ThreeDirectionsPike
		|| buildingOn->m_buildingType == BuildingType::Mortar
		|| buildingOn->m_buildingType == BuildingType::Laser) {
		((TowerBase*)buildingOn)->AddResource( this );
	}
}

bool Resource::IsResourceType( std::string const& typeShort ) const
{
	return m_def.m_shortType.find_first_of( typeShort ) != std::string::npos;
}

bool Resource::GetNumOfBasicResource( unsigned char basicResourceType ) const
{
	return m_def.GetNumOfBasicResource( basicResourceType );
}

ProductDefinition::ProductDefinition()
{

}

ProductDefinition::ProductDefinition( XmlElement* xmlIter )
{
	m_name = ParseXmlAttribute( *xmlIter, "name", "None" );
	m_shortType = ParseXmlAttribute( *xmlIter, "shortName", "None" );
	m_isRaw = ParseXmlAttribute( *xmlIter, "isRaw", true );
	m_isProjectile = ParseXmlAttribute( *xmlIter, "isProjectile", false );
	m_normalProjectileID = ParseXmlAttribute( *xmlIter, "normalProjectileID", -1 );
	m_guidedProjectileID = ParseXmlAttribute( *xmlIter, "guidedProjectileID", -1 );
	m_shellProjectileID = ParseXmlAttribute( *xmlIter, "shellProjectileID", -1 );
	m_id = ParseXmlAttribute( *xmlIter, "id", -1 );
	std::string factoryType = ParseXmlAttribute( *xmlIter, "factory", "None" );
	if (factoryType == "Refinery") {
		m_factory = BuildingType::Refinery;
	}
	else if (factoryType == "Blender") {
		m_factory = BuildingType::Blender;
	}
	m_texture = g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "iconTexture", "None" ).c_str() );
	XmlElement* recipeElement = xmlIter->FirstChildElement( "Recipe" );
	if (recipeElement) {
		m_recipe.m_productCount = ParseXmlAttribute( *recipeElement, "produceCount", m_recipe.m_productCount );
		m_recipe.m_productionTime = ParseXmlAttribute( *recipeElement, "productionTime", m_recipe.m_productionTime );
		XmlElement* productMaterialIter = recipeElement->FirstChildElement();
		while (productMaterialIter) {
			m_recipe.m_idCountPair.push_back( std::pair<int, int>( ParseXmlAttribute( *productMaterialIter, "id", -1 ), ParseXmlAttribute( *productMaterialIter, "count", -1 ) ) );
			productMaterialIter = productMaterialIter->NextSiblingElement();
		}
	}
}

void ProductDefinition::SetUpProductDefinitions()
{
	ProductDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/ProductDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document ProductDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ProductDefinitions" ), "Syntax Error! Name of the root of ProductDefinitions.xml should be \"ProductDefinitions\" " );
	//NumOfProductTypes = ParseXmlAttribute( *root, "numOfProducts", NumOfProductTypes );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ProductDefinition" ), "Syntax Error! Names of the elements of ProductDefinitions.xml should be \"ProductDefinition\" " );
		ProductDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
	s_definitions.emplace_back();
	s_definitions[s_definitions.size() - 1].m_id = -1;
}

int ProductDefinition::GetNumOfBasicResource( unsigned char basicResourceShortType ) const
{
	int count = 0;
	for (int i = 0; i < (int)m_shortType.size(); ++i) {
		if (m_shortType[i] == basicResourceShortType) {
			++count;
		}
	}
	return count;
}

std::vector<ProductDefinition> ProductDefinition::s_definitions;

ProductDefinition const& ProductDefinition::GetDefinition( std::string const& nameOrShortType, bool isShortType /*= true */ )
{
	if (isShortType) {
		for (auto const& definition : s_definitions) {
			if (nameOrShortType == definition.m_shortType) {
				return definition;
			}
		}
	}
	else {
		for (auto const& definition : s_definitions) {
			if (nameOrShortType == definition.m_name) {
				return definition;
			}
		}
	}
	ERROR_AND_DIE( Stringf("Cannot Get Definition %s", nameOrShortType.c_str()) );
}

ProductDefinition const& ProductDefinition::GetDefinition( int id )
{
	return s_definitions[id];
}

ProductDefinition const& ProductDefinition::GetNullDef()
{
	return s_definitions[s_definitions.size() - 1];
}
