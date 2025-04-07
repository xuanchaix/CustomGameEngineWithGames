#pragma once
#include "Game/GameCommon.hpp"

constexpr unsigned char BLOCK_BIT_IS_SKY = 0b1;
constexpr unsigned char BLOCK_BIT_IS_LIGHT_DIRTY = 0b10;
constexpr unsigned char BLOCK_BIT_IS_FULL_OPAQUE = 0b100;
constexpr unsigned char BLOCK_BIT_IS_SOLID = 0b1000;
constexpr unsigned char BLOCK_BIT_IS_VISIBLE = 0b10000;

constexpr unsigned char BLOCK_BIT_IS_NOT_SKY = 0b11111110;
constexpr unsigned char BLOCK_BIT_IS_NOT_LIGHT_DIRTY = 0b11111101;

constexpr unsigned char BLOCK_INDOOR_LIGHT_MASK = 0b1111;
constexpr unsigned char BLOCK_OUTDOOR_LIGHT_MASK = 0b11110000;

struct BlockDefinition {

	std::string m_name;
	unsigned char m_index = (unsigned char)-1;
	bool m_visible = true;
	bool m_solid = true;
	bool m_opaque = true;
	int m_lightStrength = 0;

	AABB2 m_topUV = AABB2::IDENTITY;
	AABB2 m_bottomUV = AABB2::IDENTITY;
	AABB2 m_sideUV = AABB2::IDENTITY;

	BlockDefinition();
	BlockDefinition( XmlElement* xmlIter );
	static void SetUpDefinitions();
	static std::vector<BlockDefinition> s_definitions;
	static BlockDefinition const& GetDefinitionByIndex( unsigned char index );
	static BlockDefinition const& GetDefinitionByName( std::string const& name );
	static unsigned char GetDefinitionIndexByName( std::string const& name );
};

struct Block {
public:
	Block();
	Block( unsigned char typeIndex );
	BlockDefinition const& GetDefinition() const;
	void SetType( unsigned char type );
	unsigned char GetType() const;

	unsigned char GetIndoorLightInfluence() const;
	void SetIndoorLightInfluence( unsigned char influence );
	unsigned char GetOutdoorLightInfluence() const;
	void SetOutdoorLightInfluence( unsigned char influence );

	bool IsSky() const;
	void SetSky( bool isSky );

	bool IsLightDirty() const;
	void SetLightDirty( bool isDirty );

	bool IsSolid() const;
	bool IsOpaque() const;

public:
	unsigned char m_type = 0;
	unsigned char m_lightInfluenceData = 0;
	unsigned char m_bitFlags = 0;
};


inline BlockDefinition const& Block::GetDefinition() const
{
	return BlockDefinition::GetDefinitionByIndex( m_type );
}

inline void Block::SetType( unsigned char type )
{
	m_type = type;
}

inline unsigned char Block::GetIndoorLightInfluence() const {
	return m_lightInfluenceData & BLOCK_INDOOR_LIGHT_MASK;
}

inline void Block::SetIndoorLightInfluence( unsigned char influence ) {
	m_lightInfluenceData = m_lightInfluenceData & BLOCK_OUTDOOR_LIGHT_MASK | influence;
}

inline unsigned char  Block::GetOutdoorLightInfluence() const {
	return m_lightInfluenceData >> 4;
}

inline void Block::SetOutdoorLightInfluence( unsigned char influence ) {
	m_lightInfluenceData = m_lightInfluenceData & BLOCK_INDOOR_LIGHT_MASK | (influence << 4);
}

inline unsigned char Block::GetType() const {
	return m_type;
}

inline bool Block::IsSky() const {
	return m_bitFlags & BLOCK_BIT_IS_SKY;
}

inline void Block::SetSky( bool isSky ) {
	if (isSky) {
		m_bitFlags |= BLOCK_BIT_IS_SKY;
	}
	else {
		m_bitFlags &= BLOCK_BIT_IS_NOT_SKY;
	}
}

inline bool Block::IsLightDirty() const {
	return m_bitFlags & BLOCK_BIT_IS_LIGHT_DIRTY;
}

inline void Block::SetLightDirty( bool isDirty ) {
	if (isDirty) {
		m_bitFlags |= BLOCK_BIT_IS_LIGHT_DIRTY;
	}
	else {
		m_bitFlags &= BLOCK_BIT_IS_NOT_LIGHT_DIRTY;
	}
}

inline bool Block::IsSolid() const {
	return GetDefinition().m_solid;
}

inline bool Block::IsOpaque() const {
	return GetDefinition().m_opaque;
}