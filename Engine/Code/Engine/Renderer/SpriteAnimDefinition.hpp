#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <vector>
class SpriteSheet;
class SpriteDefinition;
class Texture;

//------------------------------------------------------------------------------------------------
enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame animation, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame animation, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame animation, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};

//------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition( SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex,
		float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP );

	SpriteDefinition const& GetSpriteDefAtTime( float seconds ) const;
	Texture* GetTexture() const;
	bool IsCompleted( float seconds ) const;

private:
	SpriteSheet const& m_spriteSheet;
	int	m_startSpriteIndex = -1;
	int	m_endSpriteIndex = -1;
	float m_durationSeconds = 1.f;
	SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::LOOP;
};



class SpriteAnimGroupDefinition {
public:
	SpriteAnimGroupDefinition( XmlElement* xmlElement, SpriteSheet const& spriteSheet );
	SpriteAnimDefinition const& GetSpriteAnimDefByDirection( Vec3 const& dir ) const;
	SpriteDefinition const& GetSpriteAnimDefByDirectionAndTime( Vec3 const& dir, float seconds ) const;

	SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::ONCE;
	float m_totalTimeSeconds = 0.f;
	std::string m_name;
	bool m_scaleBySpeed = false;
private:
	std::vector<std::pair<Vec3, SpriteAnimDefinition>> m_directionAnimationDict;
};