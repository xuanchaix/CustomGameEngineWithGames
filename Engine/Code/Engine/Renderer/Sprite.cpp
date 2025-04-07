#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Texture.hpp"

SpriteSheet::SpriteSheet( Texture& texture, IntVec2 const& simplerGridLayout )
	:m_texture(texture)
{
	constexpr float offest = 1.f / 128.f;
	float xOffsetFactor = 1.f / simplerGridLayout.x;
	float yOffsetFactor = 1.f / simplerGridLayout.y;

	m_spriteDefs.reserve( (size_t)simplerGridLayout.x * simplerGridLayout.y );
	for (int i = 0; i < simplerGridLayout.y; i++) {
		for (int j = 0; j < simplerGridLayout.x; j++) {
			m_spriteDefs.emplace_back( *this, j + i * simplerGridLayout.x,
				Vec2( (float)j / simplerGridLayout.x + offest * xOffsetFactor, 1.f - (float)(i + 1) / simplerGridLayout.y + offest * yOffsetFactor ),
				Vec2( (float)(j + 1) / simplerGridLayout.x - offest * xOffsetFactor, 1.f - (float)i / simplerGridLayout.y - offest * yOffsetFactor ) );
		}
	}
}

SpriteSheet::~SpriteSheet()
{
}

Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumOfSprites() const
{
	return (int)m_spriteDefs.size();
}

SpriteDefinition const& SpriteSheet::GetSpriteDef( int spriteIndex ) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const
{
	m_spriteDefs[spriteIndex].GetUVs( out_uvAtMins, out_uvAtMaxs );
}

AABB2 SpriteSheet::GetSpriteUVs( int spriteIndex ) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}

SpriteDefinition::SpriteDefinition( SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs )
	:m_spriteSheet(spriteSheet)
	,m_spriteIndex(spriteIndex)
	,m_uvAtMins(uvAtMins)
	,m_uvAtMaxs(uvAtMaxs)
{
}

void SpriteDefinition::GetUVs( Vec2& out_UVAtMins, Vec2& out_uvAtMaxs ) const
{
	out_uvAtMaxs = m_uvAtMaxs;
	out_UVAtMins = m_uvAtMins;
}

AABB2 const SpriteDefinition::GetUVs() const
{
	return AABB2( m_uvAtMins, m_uvAtMaxs );
}

SpriteSheet const& SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}

Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}

float SpriteDefinition::GetAspect() const
{
	return (m_uvAtMaxs.x - m_uvAtMins.x) / (m_uvAtMaxs.y - m_uvAtMins.y);
}
