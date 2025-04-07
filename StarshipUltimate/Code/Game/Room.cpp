#include "Game/Room.hpp"
#include "Game/Entity.hpp"
#include "Game/Effects.hpp"
#include "Game/Item.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/PlayerController.hpp"
#include "Game/NeutralFaction.hpp"
#include "Game/App.hpp"

std::vector<RoomDefinition> RoomDefinition::s_definitions;
std::map<std::string, std::vector<std::vector<RoomDefinition*>>> RoomDefinition::s_factionLevelMap;

RoomDefinition::RoomDefinition()
{

}

RoomDefinition::RoomDefinition( XmlElement* xmlIter )
{
	m_roomTypeName = ParseXmlAttribute( *xmlIter, "type", m_roomTypeName );
	if (m_roomTypeName == "Enemy") {
		m_type = RoomType::ENEMY;
	}
	else if (m_roomTypeName == "Shop") {
		m_type = RoomType::SHOP;
	}
	else if (m_roomTypeName == "Chest") {
		m_type = RoomType::CHEST;
	}
	else if (m_roomTypeName == "DangerousChest") {
		m_type = RoomType::DANGEROUS_CHEST;
	}
	m_faction = ParseXmlAttribute( *xmlIter, "faction", m_faction );
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_numOfItemsToChoose = ParseXmlAttribute( *xmlIter, "numOfItemRewards", m_numOfItemsToChoose );
	m_roomLevel = ParseXmlAttribute( *xmlIter, "roomLevel", m_roomLevel );
	m_appearOnlyOnce = ParseXmlAttribute( *xmlIter, "appearOnlyOnce", m_appearOnlyOnce );
	XmlElement* spawnPointsElement = xmlIter->FirstChildElement( "SpawnPoints" );
	if (spawnPointsElement) {
		XmlElement* iter = spawnPointsElement->FirstChildElement();
		while (iter != nullptr) {
			m_spawnInfos.emplace_back( iter );
			iter = iter->NextSiblingElement();
		}
	}

	XmlElement* itemElement = xmlIter->FirstChildElement( "ItemInfo" );
	if (itemElement) {
		m_itemInfo = ItemInfo( itemElement );
	}
}

void RoomDefinition::SetUpRoomDefinitions()
{
	RoomDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/RoomDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document RoomDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "RoomDefinitions" ), "Syntax Error! Name of the root of RoomDefinitions.xml should be \"RoomDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "RoomDefinition" ), "Syntax Error! Names of the elements of RoomDefinitions.xml should be \"RoomDefinition\" " );
		RoomDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}

	// set up room level faction map
	for (auto& def : s_definitions) {
		auto iter = s_factionLevelMap.find( def.m_faction );
		if (iter != s_factionLevelMap.end()) {
			iter->second[def.m_roomLevel].push_back( &def );
		}
		else {
			s_factionLevelMap[def.m_faction] = std::vector<std::vector<RoomDefinition*>>( 6 );
			s_factionLevelMap[def.m_faction][def.m_roomLevel].push_back( &def );
		}
	}
}

RoomDefinition const& RoomDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "No Such Room Definition %s", name.c_str() ) );
}

RoomDefinition const& RoomDefinition::GetRandomDefinition( std::string const& faction, int level )
{
	auto iter = s_factionLevelMap.find( faction );
	if (iter != s_factionLevelMap.end()) {
		if (level != -1) {
			level = GetClamped( level, 0, 5 );
			if ((int)iter->second[level].size() > 0) {
				int rnd;
				do {
					rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)iter->second[level].size() );
				} while (iter->second[level][rnd]->m_appeared && iter->second[level][rnd]->m_appearOnlyOnce);
				iter->second[level][rnd]->m_appeared = true;
				return *(iter->second[level][rnd]);
			}
			else {
				ERROR_AND_DIE( Stringf( "No level %d of room in faction %s!", level, faction.c_str() ) );
			}
		}
		else {
			int count = 0;
			int rndLevel = 0;
			do {
				if (count > 30) {
					ERROR_AND_DIE( Stringf( "Cannot find a random room in faction %s! Possible infinite loop!", faction.c_str() ) );
				}
				// no boss can be chosen
				rndLevel = g_theGame->m_randNumGen->RollRandomIntLessThan( 5 );
				count++;
			} while ((int)iter->second[rndLevel].size() == 0);
			int rnd;
			do {
				rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)iter->second[rndLevel].size() );
			} while (iter->second[rndLevel][rnd]->m_appeared && iter->second[rndLevel][rnd]->m_appearOnlyOnce);
			iter->second[rndLevel][rnd]->m_appeared = true;
			return *(iter->second[rndLevel][rnd]);
		}
	}
	else {
		ERROR_AND_DIE( Stringf( "No faction %s!", faction.c_str() ) );
	}
}

void RoomDefinition::ResetAllDefinitionsForEachFloor()
{
	for (auto& def : s_definitions) {
		if (def.m_faction != "Event") {
			def.m_appeared = false;
		}
	}
}

void RoomDefinition::ResetAllDefinitionsForEachGame()
{
	for (auto& def : s_definitions) {
		def.m_appeared = false;
	}
}

ItemInfo::ItemInfo( XmlElement* iter )
{
	m_draftingPool = ParseXmlAttribute( *iter, "pool", m_draftingPool );
	m_position = ParseXmlAttribute( *iter, "localPos", m_position );
}

ItemInfo::ItemInfo()
{

}

EnemySpawnInfo::EnemySpawnInfo( XmlElement* iter )
{
	m_position = ParseXmlAttribute( *iter, "localPos", m_position );
	m_name = ParseXmlAttribute( *iter, "enemyName", m_name );
	m_enemyLevel = ParseXmlAttribute( *iter, "enemyLevel", m_enemyLevel );
	m_delay = ParseXmlAttribute( *iter, "spawnDelay", m_delay );
}

Room::Room( RoomDefinition const& def, IntVec2 const& coords )
	:m_def(def)
	,m_coords(coords)
{
	Vec2 leftBottomPos = Vec2( (float)coords.x * WORLD_SIZE_X, (float)coords.y * WORLD_SIZE_Y );
	m_bounds = AABB2( leftBottomPos, leftBottomPos + Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );

}

Room::~Room()
{

}

void Room::ExitRoom()
{
	for (auto door : m_doors) {
		door->m_resetByEntering = true;
	}
}

void Room::EnterRoom()
{
	// prepare door
	for (int i = 0; i < 4; i++) {
		if (g_theGame->GetRoomInDirectionByCurRoom( (RoomDirection)i ) && !HasDoor( (RoomDirection)i )) {
			LevelPortal* door = (LevelPortal*)g_theGame->SpawnEffectToGame( EffectType::Door, GetDoorAtDir( (RoomDirection)i ) );
			door->m_dir = (RoomDirection)i;
			door->m_owner = this;
			door->BeginPlay();
			m_doors.push_back( door );
		}
	}
	if (m_isFirstEnter) {
		m_isFirstEnter = false;
		// set up shop
		if (m_def.m_type == RoomType::SHOP) {
			// shop spawn machine
			// float rndMachineExistence = GetRandGen()->RollRandomFloatZeroToOne();
			float rndMachine = GetRandGen()->RollRandomFloatZeroToOne();
			InteractableMachine* machine = (InteractableMachine*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "InteractableMachine" ), Vec2( 30.f, 20.f ) + m_bounds.m_mins, 0.f );
			machine->m_cosmeticRadius *= 1.5f;
			machine->m_physicsRadius *= 1.5f;
			if (rndMachine > 0.8f) {
				machine->m_type = InteractableMachineType::ShopOwner;
			}
			else if (rndMachine > 0.6f) {
				machine->m_type = InteractableMachineType::Recycle;
			}
			else if (rndMachine > 0.4f) {
				machine->m_type = InteractableMachineType::RefreshShop;
			}
			else {
				machine->m_type = InteractableMachineType::SaveMoney;
			}

			//float rnd = GetRandGen()->RollRandomFloatZeroToOne();
			//if (rnd < 0.5f) {
			m_hasMaxHealth = true;
			//}
			//rnd = GetRandGen()->RollRandomFloatZeroToOne();
			//if (rnd < 0.7f) {
			m_hasMaxArmor = true;
			//}
			if (m_def.m_roomLevel == 1) {
				m_hasItems = true;
				m_numOfItemsCanChoose = 4;
				Vec2 startPos( 55.f, 50.f );
				int rndDiscount = GetRandGen()->RollRandomIntInRange( 0, 3 );
				for (int i = 0; i < m_numOfItemsCanChoose; i++) {
					int level = GetRandGen()->RollRandomIntInRange( 1, 3 );
					ItemDefinition* thisDef = ItemDefinition::GetRandomDefinition( level );
					if (thisDef) {
						ItemDefinition::SetItemAvailability( thisDef->m_id, false );
						ItemDefinition::SetStatusAndPosition( thisDef->m_id, ItemStatus::In_Room, m_bounds.m_mins + startPos + Vec2( 30.f, 0.f ) * (float)i, true );
						m_items.push_back( thisDef->m_id );
						if (rndDiscount == i) {
							thisDef->m_isDiscount = true;
						}
						else {
							thisDef->m_isDiscount = false;
						}
					}
				}
			}
			else if (m_def.m_roomLevel == 2) {
				m_hasItems = true;
				m_numOfItemsCanChoose = 5;
				Vec2 startPos( 40.f, 50.f );
				int rndDiscount = GetRandGen()->RollRandomIntInRange( 0, 4 );
				for (int i = 0; i < m_numOfItemsCanChoose; i++) {
					int level = GetRandGen()->RollRandomIntInRange( 1, 4 );
					ItemDefinition* thisDef = ItemDefinition::GetRandomDefinition( level );
					if (thisDef) {
						ItemDefinition::SetItemAvailability( thisDef->m_id, false );
						ItemDefinition::SetStatusAndPosition( thisDef->m_id, ItemStatus::In_Room, m_bounds.m_mins + startPos + Vec2( 30.f, 0.f ) * (float)i, true );
						m_items.push_back( thisDef->m_id );
						if (rndDiscount == i) {
							thisDef->m_isDiscount = true;
						}
						else {
							thisDef->m_isDiscount = false;
						}
					}
				}
			}
			else if (m_def.m_roomLevel == 3) {
				m_hasItems = true;
				m_numOfItemsCanChoose = 6;
				Vec2 startPos( 70.f, 50.f );
				Vec2 startPos2( 70.f, 30.f );
				int rndDiscount = GetRandGen()->RollRandomIntInRange( 0, 5 );
				for (int i = 0; i < m_numOfItemsCanChoose; i++) {
					int level = GetRandGen()->RollRandomIntInRange( 1, 4 );
					ItemDefinition* thisDef = ItemDefinition::GetRandomDefinition( level );
					if (thisDef) {
						ItemDefinition::SetItemAvailability( thisDef->m_id, false );
						if (i <= 2) {
							ItemDefinition::SetStatusAndPosition( thisDef->m_id, ItemStatus::In_Room, m_bounds.m_mins + startPos + Vec2( 30.f, 0.f ) * (float)i, true );
						}
						else {
							ItemDefinition::SetStatusAndPosition( thisDef->m_id, ItemStatus::In_Room, m_bounds.m_mins + startPos2 + Vec2( 30.f, 0.f ) * (float)(i - 3), true );
						}
						m_items.push_back( thisDef->m_id );
						if (rndDiscount == i) {
							thisDef->m_isDiscount = true;
						}
						else {
							thisDef->m_isDiscount = false;
						}
					}
				}
			}
			else if (m_def.m_roomLevel == 4) {
				m_hasItems = true;
				m_numOfItemsCanChoose = 7;
				Vec2 startPos( 55.f, 50.f );
				Vec2 startPos2( 70.f, 30.f );
				int rndDiscount = GetRandGen()->RollRandomIntInRange( 0, 6 );
				for (int i = 0; i < m_numOfItemsCanChoose; i++) {
					int level = GetRandGen()->RollRandomIntInRange( 1, 4 );
					ItemDefinition* thisDef = ItemDefinition::GetRandomDefinition( level );
					if (thisDef) {
						ItemDefinition::SetItemAvailability( thisDef->m_id, false );
						if (i <= 3) {
							ItemDefinition::SetStatusAndPosition( thisDef->m_id, ItemStatus::In_Room, m_bounds.m_mins + startPos + Vec2( 30.f, 0.f ) * (float)i, true );
						}
						else {
							ItemDefinition::SetStatusAndPosition( thisDef->m_id, ItemStatus::In_Room, m_bounds.m_mins + startPos2 + Vec2( 30.f, 0.f ) * (float)(i - 4), true );
						}
						m_items.push_back( thisDef->m_id );
						if (rndDiscount == i) {
							thisDef->m_isDiscount = true;
						}
						else {
							thisDef->m_isDiscount = false;
						}
					}
				}
			}
			m_healthLBPos = Vec2( 150.f, 20.f ) + m_bounds.m_mins;
			m_armorLBPos = Vec2( 160.f, 20.f ) + m_bounds.m_mins;
			m_maxHealthLBPos = Vec2( 170.f, 20.f ) + m_bounds.m_mins;
			m_maxArmorLBPos = Vec2( 180.f, 20.f ) + m_bounds.m_mins;
		}
		// set up enemies
		for (auto& spawnInfo : m_def.m_spawnInfos) {
			if (spawnInfo.m_name == "Default") {
				ERROR_AND_DIE( "Cannot spawn default enemy!" );
			}
			else if (spawnInfo.m_name == "Random") {
				// #ToDo:
				EntityDefinition const& def = EntityDefinition::GetRandomDefinition( m_def.m_faction, spawnInfo.m_enemyLevel );
				Vec2 spawnWorldPos = spawnInfo.m_position + m_bounds.m_mins;
				float orientation = (g_theGame->GetPlayerEntity()->m_position - spawnWorldPos).GetOrientationDegrees();
				g_theGame->SpawnEntityToGame( def, spawnWorldPos, orientation );
			}
			else {
				EntityDefinition const& defToSpawn = EntityDefinition::GetDefinition( spawnInfo.m_name );
				Vec2 spawnWorldPos = spawnInfo.m_position + m_bounds.m_mins;
				float orientation = (g_theGame->GetPlayerEntity()->m_position - spawnWorldPos).GetOrientationDegrees();
				g_theGame->SpawnEntityToGame( defToSpawn, spawnWorldPos, orientation );
			}
		}

		// set up chest
		if (m_def.m_type == RoomType::CHEST) {
			Chest* chest = (Chest*)(g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "Chest" ), m_def.m_itemInfo.m_position + m_bounds.m_mins, 0.f ));
			chest->m_level = m_def.m_itemInfo.m_draftingPool;
			/*if (m_def.m_itemInfo.m_draftingPool == 1) {
				chest->m_color = Rgba8( 96, 96, 96 );
			}
			else if (m_def.m_itemInfo.m_draftingPool == 2) {
				chest->m_color = Rgba8( 76, 153, 0 );
			}
			else if (m_def.m_itemInfo.m_draftingPool == 3) {
				chest->m_color = Rgba8( 51, 153, 255 );
			}
			else if (m_def.m_itemInfo.m_draftingPool == 4) {
				chest->m_color = Rgba8( 204, 0, 0 );
			}*/
		}

		// set up mimic
		if (m_def.m_type == RoomType::DANGEROUS_CHEST) {
			Chest* chest = (Chest*)(g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "Chest" ), m_def.m_itemInfo.m_position + m_bounds.m_mins, 0.f ));
			chest->m_isMimic = true;
			chest->m_level = m_def.m_itemInfo.m_draftingPool;
			/*if (m_def.m_itemInfo.m_draftingPool == 1) {
				chest->m_color = Rgba8( 106, 96, 96 );
			}
			else if (m_def.m_itemInfo.m_draftingPool == 2) {
				chest->m_color = Rgba8( 86, 153, 0 );
			}
			else if (m_def.m_itemInfo.m_draftingPool == 3) {
				chest->m_color = Rgba8( 61, 153, 255 );
			}
			else if (m_def.m_itemInfo.m_draftingPool == 4) {
				chest->m_color = Rgba8( 214, 0, 0 );
			}*/
		}

		// set up machines
		if (m_def.m_roomTypeName == "Gamble") {
			InteractableMachine* machine = (InteractableMachine*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "InteractableMachine" ), Vec2( 100.f, 50.f ) + m_bounds.m_mins, 0.f );
			machine->m_type = InteractableMachineType::Gamble;
		}
		else if (m_def.m_roomTypeName == "SlotMachine") {
			InteractableMachine* machine = (InteractableMachine*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "InteractableMachine" ), Vec2( 100.f, 50.f ) + m_bounds.m_mins, 0.f );
			machine->m_type = InteractableMachineType::GiveCoin;
		}
		else if (m_def.m_roomTypeName == "SellHealth") {
			InteractableMachine* machine = (InteractableMachine*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "InteractableMachine" ), Vec2( 100.f, 50.f ) + m_bounds.m_mins, 0.f );
			machine->m_type = InteractableMachineType::SellHealth;
		}
		else if (m_def.m_roomTypeName == "SellMaxHealth") {
			InteractableMachine* machine = (InteractableMachine*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "InteractableMachine" ), Vec2( 100.f, 50.f ) + m_bounds.m_mins, 0.f );
			machine->m_type = InteractableMachineType::SellMaxHealth;
		}

		// items effect
		if (m_def.m_type == RoomType::SHOP && g_theGame->GetPlayerObject()->m_moreMoneyShopBonus && g_theGame->m_playerController->m_reward >= 100) {
			g_theGame->GetPlayerObject()->GainMaxArmor( 1.f );
		}
		if (m_def.m_type == RoomType::SHOP && g_theGame->GetPlayerObject()->m_richCanDoAnything && g_theGame->m_playerController->m_reward >= 100) {
			ItemDefinition::GetDefinition( g_theGame->GetPlayerObject()->m_skillItemID ).AddCharge( 2 );
		}
		if (m_def.m_type == RoomType::ENEMY && g_theGame->GetPlayerObject()->m_bribe) {
			ItemDefinition::GetDefinition( g_theGame->GetPlayerObject()->m_skillItemID ).AddCharge( 1 );
		}

		if (IsBossRoom()) {
			g_theAudio->StopSound( g_theGame->m_backgroundMusicID );
			g_theGame->m_backgroundMusicID = g_theAudio->StartSound( g_theAudio->CreateOrGetSound( g_theGame->m_bossMusic ), true, g_theApp->m_musicVolume * 0.6f );
		}
	}
	else {
		// re-enter the room do not let player portal between doors
		for (auto door : m_doors) {
			door->m_resetByEntering = true;
		}
	}
}

Vec2 Room::GetDoorAtDir( RoomDirection dirFrom )
{
	if (dirFrom == RoomDirection::UP) {
		return m_bounds.m_mins + Vec2( WORLD_SIZE_X * 0.5f, WORLD_SIZE_Y );
	}
	else if (dirFrom == RoomDirection::DOWN) {
		return m_bounds.m_mins + Vec2( WORLD_SIZE_X * 0.5f, 0.f );
	}
	else if (dirFrom == RoomDirection::LEFT) {
		return m_bounds.m_mins + Vec2( 0.f, WORLD_SIZE_Y * 0.5f );
	}
	else if (dirFrom == RoomDirection::RIGHT) {
		return m_bounds.m_mins + Vec2( WORLD_SIZE_X, WORLD_SIZE_Y * 0.5f );
	}
	return m_bounds.m_mins + Vec2( WORLD_SIZE_X * 0.5f, WORLD_SIZE_Y * 0.5f );
}

bool Room::IsBossRoom() const
{
	return m_def.m_roomLevel == 5;
}

void Room::SetPositionInsideRoom( Vec2& position, float cosmeticRadius )
{
	position.x = GetClamped( position.x, m_bounds.m_mins.x + cosmeticRadius, m_bounds.m_maxs.x - cosmeticRadius );
	position.y = GetClamped( position.y, m_bounds.m_mins.y + cosmeticRadius, m_bounds.m_maxs.y - cosmeticRadius );
}

NonItemMerchandise Room::GetNearestInRangeNonItemMerchandise( Vec2 const& position ) const
{
	float distanceSquaredToHealth = GetDistanceSquared2D( position, m_healthLBPos + Vec2( 2.5f, 2.5f ) );
	float distanceSquaredToMaxHealth = GetDistanceSquared2D( position, m_maxHealthLBPos + Vec2( 2.5f, 2.5f ) );
	float distanceSquaredToArmor = GetDistanceSquared2D( position, m_armorLBPos + Vec2( 2.5f, 2.5f ) );
	float distanceSquaredToMaxArmor = GetDistanceSquared2D( position, m_maxArmorLBPos + Vec2( 2.5f, 2.5f ) );
	if (distanceSquaredToArmor == Minf( distanceSquaredToArmor, Minf( distanceSquaredToHealth, Minf( distanceSquaredToMaxHealth, distanceSquaredToMaxArmor ) ) )
		&& distanceSquaredToArmor <= 25.f) {
		return NonItemMerchandise::Armor;
	}
	else if (distanceSquaredToHealth == Minf( distanceSquaredToArmor, Minf( distanceSquaredToHealth, Minf( distanceSquaredToMaxHealth, distanceSquaredToMaxArmor ) ) )
		&& distanceSquaredToHealth <= 25.f) {
		return NonItemMerchandise::Health;
	}
	else if (distanceSquaredToMaxArmor == Minf( distanceSquaredToArmor, Minf( distanceSquaredToHealth, Minf( distanceSquaredToMaxHealth, distanceSquaredToMaxArmor ) ) )
		&& distanceSquaredToMaxArmor <= 25.f) {
		return NonItemMerchandise::MaxArmor;
	}
	else if (distanceSquaredToMaxHealth == Minf( distanceSquaredToArmor, Minf( distanceSquaredToHealth, Minf( distanceSquaredToMaxHealth, distanceSquaredToMaxArmor ) ) )
		&& distanceSquaredToMaxHealth <= 25.f) {
		return NonItemMerchandise::MaxHealth;
	}
	return NonItemMerchandise::None;
}

void Room::BuyNonItemMerchandise( NonItemMerchandise target )
{
	if (target == NonItemMerchandise::Armor) {
		m_hasArmor = false;
	}
	else if (target == NonItemMerchandise::Health) {
		m_hasHealth = false;
	}
	else if (target == NonItemMerchandise::MaxArmor) {
		m_hasMaxArmor = false;
	}
	else if (target == NonItemMerchandise::MaxHealth) {
		m_hasMaxHealth = false;
	}
}

bool Room::HasDoor( RoomDirection dir ) const
{
	for (int i = 0; i < (int)m_doors.size(); i++) {
		if (m_doors[i]->m_dir == dir) {
			return true;
		}
	}
	return false;
}
