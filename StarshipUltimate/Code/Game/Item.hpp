#pragma once
#include "Game/GameCommon.hpp"

enum class ItemStatus {
	In_Player_Hands, In_Pool, In_Room
};

struct ItemDefinition {
	// in game states
	bool m_isAvailable = true;
	bool m_isSold = false;
	bool m_isDiscount = false;
	bool m_isThrowAwayItem = false;
	ItemStatus m_status = ItemStatus::In_Pool;
	Vec2 m_position;

	int m_charge;
	int m_maxCharge = 5;

	int m_id = 0;
	int m_pool = 0;
	std::string m_name = "Default";
	std::string m_type = "Normal";
	std::string m_description = "None";
	std::string m_category = "Passive";
	std::string m_specialPool = "Default";
	std::string m_detail = "Default";

	// normal attributes
	float m_damageModifier = 0.f; // multiply
	float m_attackSpeedModifier = 0.f; // multiply
	float m_movingSpeedModifier = 0.f; // multiply
	float m_bulletSpeedModifier = 0.f; // multiply
	float m_bulletLifeTimeModifier = 0.f;
	float m_maxHealthModifier = 0.f; // add
	float m_maxArmorModifier = 0.f; // add
	float m_dashingCoolDownModifier = 0.f; // multiply
	float m_dashingDistanceModifier = 0.f; // multiply
	float m_recoverHealth = 0.f;
	int m_startCharge = 0;
	int m_chargePerLevel = 0;
	bool m_hasCharge = false;

	ItemDefinition();
	ItemDefinition( XmlElement* xmlIter );
	void AddVertsForItem( std::vector<Vertex_PCU>& verts ) const;
	void RenderItem( AABB2 const& pos ) const;
	int GetPrice() const;
	int GetBasicPrice() const;
	void AddCharge( int num );
	bool CanBeUsed() const;

	static void SetUpItemDefinitions();
	static std::vector<ItemDefinition> s_definitions;
	static std::vector<std::vector<ItemDefinition*>> s_itemPools;
	static std::map<std::string, std::vector<ItemDefinition*>> s_itemSpecialPools;
	static ItemDefinition const& GetDefinition( std::string const& name );
	static ItemDefinition& GetDefinition( int id );
	/// if level == -1, get random level
	static ItemDefinition* GetRandomDefinition( int pool = -1 );
	static ItemDefinition* GetRandomDefinitionInSpecialPool( std::string const& specialPoolName );
	static void SetItemAvailability( int id, bool available );
	static void SetStatusAndPosition( int id, ItemStatus status, Vec2 const& position, bool isSold=false );

	static void GetAllItemsInPool( int pool, std::vector<int>& out_ids );
	static void GetAvailableSkillItem( std::vector<int>& out_ids );
	static void GetAvailableSubWeaponItem( std::vector<int>& out_ids );

	static void ResetAllItems();
	static void ResetAllInRoomItems();

protected:
	static SpriteSheet* m_spriteSheet;
};
