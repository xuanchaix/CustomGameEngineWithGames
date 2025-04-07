#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
class SpriteSheet;
class Texture;

class SpriteDefinition {
public:
	explicit SpriteDefinition( SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs );
	void GetUVs( Vec2& out_UVAtMins, Vec2& out_uvAtMaxs ) const;
	AABB2 const GetUVs() const;
	SpriteSheet const& GetSpriteSheet() const;
	Texture& GetTexture() const;
	float GetAspect() const;

protected:
	SpriteSheet const& m_spriteSheet;
	int m_spriteIndex = -1;
	Vec2 m_uvAtMins = Vec2( 0, 0 );
	Vec2 m_uvAtMaxs = Vec2( 1, 1 );
};

class SpriteSheet {
public:
	explicit SpriteSheet( Texture& texture, IntVec2 const& simplerGridLayout );
	~SpriteSheet();

	Texture& GetTexture() const;
	int GetNumOfSprites() const;
	SpriteDefinition const& GetSpriteDef( int spriteIndex ) const;
	void GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	AABB2 GetSpriteUVs( int spriteIndex ) const;
	
protected:
	Texture& m_texture;
	std::vector<SpriteDefinition> m_spriteDefs;
};