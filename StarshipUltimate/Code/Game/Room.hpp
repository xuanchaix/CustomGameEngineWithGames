#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"

enum class RoomType { DEFAULT, ENEMY, EVENT, NEUTRAL, SHOP, CHEST, DANGEROUS_CHEST, NUM };

class LevelPortal;

struct EnemySpawnInfo {
	EnemySpawnInfo( XmlElement* iter );
	Vec2 m_position;
	std::string m_name = "Default"; // "Default" will give error; "Random" will spawn random faction enemy
	int m_enemyLevel = -1; // -1 is default level, the value is higher, the enemy is stronger, only available if randomly spawn
	float m_delay = 0.f; // spawn delay seconds
};

struct ItemInfo {
	ItemInfo();
	ItemInfo( XmlElement* iter );
	Vec2 m_position;
	int m_draftingPool = -1; // give the pool where item is drafted; -1 means no item
};

class RoomDefinition {
public:
	RoomType m_type = RoomType::DEFAULT;
	std::string m_roomTypeName = "Default";
	std::string m_name = "Default";
	std::string m_faction = "Default";
	std::vector<EnemySpawnInfo> m_spawnInfos;
	ItemInfo m_itemInfo;
	int m_numOfItemsToChoose = 1;
	int m_roomLevel = 0;
	bool m_appearOnlyOnce = false;
	bool m_appeared = false;
	RoomDefinition();
	RoomDefinition( XmlElement* xmlIter );
	static void SetUpRoomDefinitions();
	static std::vector<RoomDefinition> s_definitions;
	static std::map<std::string, std::vector<std::vector<RoomDefinition*>>> s_factionLevelMap;
	static RoomDefinition const& GetDefinition( std::string const& name );
	static RoomDefinition const& GetRandomDefinition( std::string const& faction, int level );
	static void ResetAllDefinitionsForEachFloor();
	static void ResetAllDefinitionsForEachGame();
};

enum class NonItemMerchandise {
	Health, MaxHealth, Armor, MaxArmor, None,
};

class Room {
public:
	Room( RoomDefinition const& def, IntVec2 const& coords );
	~Room();
	void ExitRoom();
	void EnterRoom();
	Vec2 GetDoorAtDir( RoomDirection dirFrom );
	bool IsBossRoom() const;
	void SetPositionInsideRoom( Vec2& position, float cosmeticRadius );
	NonItemMerchandise GetNearestInRangeNonItemMerchandise( Vec2 const& position ) const;
	void BuyNonItemMerchandise( NonItemMerchandise target );

protected:
	bool HasDoor( RoomDirection dir ) const;

public:
	IntVec2 m_coords;
	AABB2 m_bounds;
	AABB2 m_boundsOnMap;
	RoomDefinition const& m_def;
	std::vector<LevelPortal*> m_doors;
	bool m_isFirstEnter = true;

	bool m_hasItems = false;
	int m_numOfItemsCanChoose = 0;
	std::vector<int> m_items;

	// shop
	bool m_hasHealth = true;
	Vec2 m_healthLBPos;
	int m_healthPrice = 20;
	bool m_hasArmor = true;
	Vec2 m_armorLBPos;
	int m_armorPrice = 20;
	bool m_hasMaxHealth = false;
	Vec2 m_maxHealthLBPos;
	int m_maxHealthPrice = 80;
	bool m_hasMaxArmor = false;
	Vec2 m_maxArmorLBPos;
	int m_maxArmorPrice = 70;

	// item effect
	bool m_itemChooseMode = false;

};