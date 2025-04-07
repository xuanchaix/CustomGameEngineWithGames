#include "Game/Item.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

std::vector<ItemDefinition> ItemDefinition::s_definitions;
std::vector<std::vector<ItemDefinition*>> ItemDefinition::s_itemPools;
std::map<std::string, std::vector<ItemDefinition*>> ItemDefinition::s_itemSpecialPools;
SpriteSheet* ItemDefinition::m_spriteSheet = nullptr;

ItemDefinition::ItemDefinition()
{

}

ItemDefinition::ItemDefinition( XmlElement* xmlIter )
{
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_id = ParseXmlAttribute( *xmlIter, "id", m_id );
	m_type = ParseXmlAttribute( *xmlIter, "type", m_type );
	m_pool = ParseXmlAttribute( *xmlIter, "pool", m_pool );
	m_description = ParseXmlAttribute( *xmlIter, "description", m_description );
	m_category = ParseXmlAttribute( *xmlIter, "category", m_category );
	m_specialPool = ParseXmlAttribute( *xmlIter, "specialPool", m_specialPool );
	m_damageModifier = ParseXmlAttribute( *xmlIter, "damageModifier", m_damageModifier );
	m_attackSpeedModifier = ParseXmlAttribute( *xmlIter, "attackSpeedModifier", m_attackSpeedModifier );
	m_movingSpeedModifier = ParseXmlAttribute( *xmlIter, "movingSpeedModifier", m_movingSpeedModifier );
	m_bulletSpeedModifier = ParseXmlAttribute( *xmlIter, "bulletSpeedModifier", m_bulletSpeedModifier );
	m_bulletLifeTimeModifier = ParseXmlAttribute( *xmlIter, "bulletLifeTimeModifier", m_bulletLifeTimeModifier );
	m_maxHealthModifier = ParseXmlAttribute( *xmlIter, "maxHealthModifier", m_maxHealthModifier );
	m_maxArmorModifier = ParseXmlAttribute( *xmlIter, "maxArmorModifier", m_maxArmorModifier );
	m_dashingCoolDownModifier = ParseXmlAttribute( *xmlIter, "dashingCoolDownModifier", m_dashingCoolDownModifier );
	m_dashingDistanceModifier = ParseXmlAttribute( *xmlIter, "dashingDistanceModifier", m_dashingDistanceModifier );
	m_startCharge = ParseXmlAttribute( *xmlIter, "startCharge", m_startCharge );
	m_chargePerLevel = ParseXmlAttribute( *xmlIter, "chargePerLevel", m_chargePerLevel );
	m_hasCharge = ParseXmlAttribute( *xmlIter, "hasCharge", m_hasCharge );
	m_recoverHealth = ParseXmlAttribute( *xmlIter, "recoverHealth", m_recoverHealth );
	m_detail = ParseXmlAttribute( *xmlIter, "detail", m_detail );
	m_charge = m_startCharge;
}

void ItemDefinition::AddVertsForItem( std::vector<Vertex_PCU>& verts ) const
{
	float itemHalfSize = 3.f;
	AddVertsForAABB2D( verts, AABB2( m_position - Vec2( itemHalfSize, itemHalfSize ), m_position + Vec2( itemHalfSize, itemHalfSize ) ), Rgba8::WHITE );
}

void ItemDefinition::RenderItem( AABB2 const& pos ) const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, pos, Rgba8::WHITE, m_spriteSheet->GetSpriteDef( m_id ).GetUVs() );

	g_theRenderer->BindTexture( &m_spriteSheet->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

int ItemDefinition::GetPrice() const
{
	int itemPrice = 0;
	if (m_pool == 1) {
		if (m_isDiscount) {
			itemPrice = 30;
		}
		else {
			itemPrice = 50;
		}
	}
	else if (m_pool == 2) {
		if (m_isDiscount) {
			itemPrice = 50;
		}
		else {
			itemPrice = 75;
		}
	}
	else if (m_pool == 3) {
		if (m_isDiscount) {
			itemPrice = 70;
		}
		else {
			itemPrice = 100;
		}
	}
	else if (m_pool == 4) {
		if (m_isDiscount) {
			itemPrice = 100;
		}
		else {
			itemPrice = 150;
		}
	}
	if (g_theGame->GetPlayerObject()->m_discountCard) {
		itemPrice = int(itemPrice * 0.5f);
	}
	return itemPrice;
}

int ItemDefinition::GetBasicPrice() const
{
	int itemPrice = 0;
	if (m_pool == 1) {
		itemPrice = 50;
	}
	else if (m_pool == 2) {
		itemPrice = 75;
	}
	else if (m_pool == 3) {
		itemPrice = 100;
	}
	else if (m_pool == 4) {
		itemPrice = 150;
	}
	return itemPrice;
}

void ItemDefinition::AddCharge( int num )
{
	m_charge += num;
	m_charge = GetClamped( m_charge, 0, m_maxCharge );
}

bool ItemDefinition::CanBeUsed() const
{
	return m_charge != 0;
}

void ItemDefinition::SetUpItemDefinitions()
{
	ItemDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/ItemDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document ItemDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ItemDefinitions" ), "Syntax Error! Name of the root of ItemDefinitions.xml should be \"ItemDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ItemDefinition" ), "Syntax Error! Names of the elements of ItemDefinitions.xml should be \"ItemDefinition\" " );
		ItemDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}

	s_itemPools.resize( 5 );
	for (auto& def : s_definitions) {
		s_itemPools[def.m_pool].push_back( &def );
	}
	for (auto& def : s_definitions) {
		if (def.m_specialPool != "Default") {
			auto iter = s_itemSpecialPools.find( def.m_specialPool );
			if (iter == s_itemSpecialPools.end()) {
				s_itemSpecialPools[def.m_specialPool] = std::vector<ItemDefinition*>();
			}
			s_itemSpecialPools[def.m_specialPool].push_back( &def );
		}
	}
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Items.png" );
	m_spriteSheet = new SpriteSheet( *texture, IntVec2( 30, 30 ) );
}

ItemDefinition const& ItemDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "No Such Item Definition %s", name.c_str() ) );
}

ItemDefinition& ItemDefinition::GetDefinition( int id )
{
	return s_definitions[id];
}

ItemDefinition* ItemDefinition::GetRandomDefinition( int pool /*= -1 */ )
{
	if (pool != -1) {
		PlayerShip* player = g_theGame->GetPlayerObject();
		if (player && player->m_upgradeItem) {
			float rndFlt = GetRandGen()->RollRandomFloatZeroToOne();
			if (rndFlt < 0.1f) {
				pool += 1;
			}
		}
		// change to get better item
		while (1) {
			float rndFlt = GetRandGen()->RollRandomFloatZeroToOne();
			if (rndFlt < 0.1f) {
				pool++;
			}
			else {
				break;
			}
			rndFlt = GetRandGen()->RollRandomFloatZeroToOne();
			if (rndFlt < 0.1f) {
				pool++;
			}
			else {
				break;
			}
			rndFlt = GetRandGen()->RollRandomFloatZeroToOne();
			if (rndFlt < 0.1f) {
				pool++;
			}
			else {
				break;
			}
			break;
		}
		pool = GetClamped( pool, 1, 4 );
		std::vector<ItemDefinition*> const& itemPool = s_itemPools[pool];
		GUARANTEE_OR_DIE( (int)itemPool.size() > 0, Stringf( "Error! No level %d items", pool ) );
		int rndNum = 0;
		int count = 0;
		do {
			++count;
			rndNum = GetRandGen()->RollRandomIntLessThan( (int)itemPool.size() );
		} while (!itemPool[rndNum]->m_isAvailable && count < 100);
		if (count >= 100) {
			return nullptr;
		}
		else {
			return itemPool[rndNum];
		}
	}
	else {
		int rndNum = 0;
		int count = 0;
		do {
			++count;
			rndNum = GetRandGen()->RollRandomIntLessThan( (int)s_definitions.size() );
		} while (!s_definitions[rndNum].m_isAvailable && count < 1000);
		if (count >= 100) {
			return nullptr;
		}
		else {
			return &GetDefinition( rndNum );
		}
	}
}

ItemDefinition* ItemDefinition::GetRandomDefinitionInSpecialPool( std::string const& specialPoolName )
{
	auto iter = s_itemSpecialPools.find( specialPoolName );
	if (iter == s_itemSpecialPools.end()) {
		ERROR_AND_DIE( Stringf( "Error! No item special pool named %s!", specialPoolName.c_str() ) );
	}
	std::vector<ItemDefinition*> const& itemPool = iter->second;
	int rndNum = 0;
	int count = 0;
	do {
		++count;
		rndNum = GetRandGen()->RollRandomIntLessThan( (int)itemPool.size() );
	} while (!itemPool[rndNum]->m_isAvailable && count < 100);
	if (count >= 100) {
		return nullptr;
	}
	else {
		return itemPool[rndNum];
	}
}

void ItemDefinition::SetItemAvailability( int id, bool available )
{
	s_definitions[id].m_isAvailable = available;
}

void ItemDefinition::SetStatusAndPosition( int id, ItemStatus status, Vec2 const& position, bool isSold )
{
	s_definitions[id].m_status = status;
	s_definitions[id].m_position = position;
	s_definitions[id].m_isSold = isSold;
}

void ItemDefinition::GetAllItemsInPool( int pool, std::vector<int>& out_ids )
{
	out_ids.clear();
	for (int i = 0; i < (int)s_definitions.size(); i++) {
		if (s_definitions[i].m_pool == pool && s_definitions[i].m_status == ItemStatus::In_Pool && s_definitions[i].m_isAvailable) {
			out_ids.push_back( i );
		}
	}
}

void ItemDefinition::GetAvailableSkillItem( std::vector<int>& out_ids )
{
	out_ids.clear();
	for (auto& def : s_definitions) {
		if (def.m_category == "Skill" && def.m_status == ItemStatus::In_Pool && def.m_isAvailable) {
			out_ids.push_back( def.m_id );
		}
	}
}

void ItemDefinition::GetAvailableSubWeaponItem( std::vector<int>& out_ids )
{
	out_ids.clear();
	for (auto& def : s_definitions) {
		if (def.m_category == "SubWeapon" && def.m_status == ItemStatus::In_Pool && def.m_isAvailable) {
			out_ids.push_back( def.m_id );
		}
	}
}

void ItemDefinition::ResetAllItems()
{
	for (int i = 0; i < (int)s_definitions.size(); i++) {
		s_definitions[i].m_isAvailable = true;
		s_definitions[i].m_isSold = false;
		s_definitions[i].m_isDiscount = false;
		s_definitions[i].m_isThrowAwayItem = false;
		s_definitions[i].m_status = ItemStatus::In_Pool;
		s_definitions[i].m_position = Vec2( 0.f, 0.f );
		s_definitions[i].m_charge = s_definitions[i].m_startCharge;
	}
}

void ItemDefinition::ResetAllInRoomItems()
{
	for (int i = 0; i < (int)s_definitions.size(); i++) {
		if (s_definitions[i].m_status == ItemStatus::In_Room) {
			s_definitions[i].m_isAvailable = true;
			s_definitions[i].m_isSold = false;
			s_definitions[i].m_isDiscount = false;
			s_definitions[i].m_isThrowAwayItem = false;
			s_definitions[i].m_status = ItemStatus::In_Pool;
			s_definitions[i].m_position = Vec2( 0.f, 0.f );
			s_definitions[i].m_charge = s_definitions[i].m_startCharge;
		}
	}
}
