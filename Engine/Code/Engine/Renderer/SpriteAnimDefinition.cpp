#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"

SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds,
	SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::LOOP */ )
	:m_spriteSheet(sheet)
	,m_startSpriteIndex(startSpriteIndex)
	,m_endSpriteIndex(endSpriteIndex)
	,m_durationSeconds(durationSeconds)
	,m_playbackType(playbackType)
{
}

const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime( float seconds ) const
{
	if (m_playbackType == SpriteAnimPlaybackType::LOOP) {
		while (seconds >= m_durationSeconds) {
			// make seconds inside [0, duration]
			seconds -= m_durationSeconds;
		}
		while (seconds < 0.f) {
			seconds += m_durationSeconds;
		}
		return m_spriteSheet.GetSpriteDef( GetClamped( RoundDownToInt( Interpolate( (float)m_startSpriteIndex, (float)(m_endSpriteIndex + 1.f), seconds / m_durationSeconds ) ), m_startSpriteIndex, m_endSpriteIndex ) );
	}
	else if (m_playbackType == SpriteAnimPlaybackType::ONCE) {
		// if seconds >= duration: always last frame
		if (seconds >= m_durationSeconds) {
			return m_spriteSheet.GetSpriteDef( m_endSpriteIndex );
		}
		else if (seconds < 0.f) {
			return m_spriteSheet.GetSpriteDef( m_startSpriteIndex );
		}
		else {
			return m_spriteSheet.GetSpriteDef( GetClamped( RoundDownToInt( Interpolate( (float)m_startSpriteIndex, (float)(m_endSpriteIndex + 1.f), seconds / m_durationSeconds ) ), m_startSpriteIndex, m_endSpriteIndex ) );
		}
	}
	else if (m_playbackType == SpriteAnimPlaybackType::PINGPONG) {
		float numOfFrame = (float)m_endSpriteIndex - (float)m_startSpriteIndex;
		float actualDurationSeconds = m_durationSeconds / numOfFrame * (numOfFrame - 1.f);
		// period is 2 * duration
		while (seconds >= actualDurationSeconds * 2.f) {
			seconds -= actualDurationSeconds * 2.f;
		}
		while (seconds < 0.f) {
			seconds += actualDurationSeconds * 2.f;
		}
		// reversed loop
		if (seconds >= actualDurationSeconds) {
			seconds = 2.f * actualDurationSeconds - seconds;
			return m_spriteSheet.GetSpriteDef( 1 + GetClamped( RoundDownToInt( Interpolate( (float)m_startSpriteIndex, (float)(m_endSpriteIndex + 1.f), seconds / actualDurationSeconds ) ), m_startSpriteIndex, m_endSpriteIndex ) );
		}
		// normal loop
		else {
			return m_spriteSheet.GetSpriteDef( GetClamped( RoundDownToInt( Interpolate( (float)m_startSpriteIndex, (float)(m_endSpriteIndex + 1.f), seconds / actualDurationSeconds ) ), m_startSpriteIndex, m_endSpriteIndex ) );
		}
	}
	ERROR_AND_DIE("Do not have this playbackType!")
}

Texture* SpriteAnimDefinition::GetTexture() const
{
	return &(m_spriteSheet.GetTexture());
}

bool SpriteAnimDefinition::IsCompleted( float seconds ) const
{
	if (m_playbackType == SpriteAnimPlaybackType::ONCE) {
		if (seconds >= m_durationSeconds) {
			return true;
		}
	}
	return false;
}

SpriteAnimGroupDefinition::SpriteAnimGroupDefinition( XmlElement* xmlElement, SpriteSheet const& spriteSheet )
{
	GUARANTEE_OR_DIE( !strcmp( xmlElement->Name(), "AnimationGroup" ), "Error! Name of XML element should be AnimationGroup!" );
	m_name = ParseXmlAttribute( *xmlElement, "name", "Default" );
	m_scaleBySpeed = ParseXmlAttribute( *xmlElement, "scaleBySpeed", m_scaleBySpeed );
	float secondsPerFrame = ParseXmlAttribute( *xmlElement, "secondsPerFrame", 0.01f );
	std::string playbackModeStr = ParseXmlAttribute( *xmlElement, "playbackMode", "Default" );
	if (playbackModeStr == "Loop") {
		m_playbackType = SpriteAnimPlaybackType::LOOP;
	}
	else if (playbackModeStr == "Once") {
		m_playbackType = SpriteAnimPlaybackType::ONCE;
	}
	else if (playbackModeStr == "PingPong") {
		m_playbackType = SpriteAnimPlaybackType::PINGPONG;
	}
	XmlElement* iter = xmlElement->FirstChildElement();
	GUARANTEE_OR_DIE( iter != nullptr, "Error! Group Animation should have some directions!" );
	while (iter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( iter->Name(), "Direction" ), "Error! Name of XML element should be Direction!" );
		Vec3 dir = ParseXmlAttribute( *iter, "vector", Vec3( 1.f, 0.f, 0.f ) ).GetNormalized();
		XmlElement* animElement = iter->FirstChildElement();
		GUARANTEE_OR_DIE( !strcmp( animElement->Name(), "Animation" ), "Error! Name of XML element should be Animation!" );
		int startFrame = ParseXmlAttribute( *animElement, "startFrame", -1 );
		int endFrame = ParseXmlAttribute( *animElement, "endFrame", -1 );
		m_totalTimeSeconds = (endFrame - startFrame + 1) * secondsPerFrame;
		m_directionAnimationDict.push_back( std::pair<Vec3, SpriteAnimDefinition>( dir, SpriteAnimDefinition( spriteSheet, startFrame, endFrame, m_totalTimeSeconds, m_playbackType ) ) );
		iter = iter->NextSiblingElement();
	}
}

SpriteAnimDefinition const& SpriteAnimGroupDefinition::GetSpriteAnimDefByDirection( Vec3 const& viewDirNormal ) const
{
	SpriteAnimDefinition const* res = &m_directionAnimationDict[0].second;
	float maxDotProduct = DotProduct3D( viewDirNormal, m_directionAnimationDict[0].first );
	for (auto& pair : m_directionAnimationDict) {
		float thisDotProduct = DotProduct3D( viewDirNormal, pair.first );
		if (maxDotProduct < thisDotProduct) {
			res = &pair.second;
		}
	}
	return *res;
}

SpriteDefinition const& SpriteAnimGroupDefinition::GetSpriteAnimDefByDirectionAndTime( Vec3 const& viewDirNormal, float seconds ) const
{
	SpriteAnimDefinition const* res = &m_directionAnimationDict[0].second;
	float maxDotProduct = DotProduct3D( viewDirNormal, m_directionAnimationDict[0].first );
	for (auto& pair : m_directionAnimationDict) {
		float thisDotProduct = DotProduct3D( viewDirNormal, pair.first );
		if (maxDotProduct < thisDotProduct) {
			res = &pair.second;
			maxDotProduct = thisDotProduct;
		}
	}
	return res->GetSpriteDefAtTime( seconds );
}

