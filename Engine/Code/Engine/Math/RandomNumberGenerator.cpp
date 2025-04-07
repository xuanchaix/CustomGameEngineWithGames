#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"
#include <time.h>

RandomNumberGenerator::RandomNumberGenerator( unsigned int seed/*=0 */ )
	:m_seed(seed == 0 ? (unsigned int)time( NULL ) : seed)
{
	
}

int RandomNumberGenerator::RollRandomIntLessThan( int maxNotInclusive )
{
	GUARANTEE_OR_DIE( maxNotInclusive > 0, "maxNotInclusive needs to > 0" );
	return Get1dNoiseUint( m_position++, m_seed ) % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange( int minInclusive, int maxInclusive )
{
	//GUARANTEE_OR_DIE( minInclusive <= maxInclusive, "NEED: minInclusive <= maxInclusive && maxInclusive >= 0" );
	//return (int)((cosf( rand() ) / 2.f + 0.5f) * (maxInclusive - minInclusive) + minInclusive);
	if (minInclusive == maxInclusive) {
		return minInclusive;
	}
	if (minInclusive > maxInclusive) {
		return (Get1dNoiseUint( m_position++, m_seed ) & 0x7fffffff) % (minInclusive - maxInclusive + 1) + maxInclusive;
	}
	return (Get1dNoiseUint( m_position++, m_seed ) & 0x7fffffff) % (maxInclusive - minInclusive + 1) + minInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange( IntRange const& inclusiveIntRange )
{
	return RollRandomIntInRange( inclusiveIntRange.m_min, inclusiveIntRange.m_max );
}

int RandomNumberGenerator::RollRandomPositiveNegative()
{
	if (Get1dNoiseUint( m_position++, m_seed ) % 2) {
		return -1;
	}
	else {
		return 1;
	}
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return Get1dNoiseZeroToOne( m_position++, m_seed );
}

float RandomNumberGenerator::RollRandomFloatInRange( float minInclusive, float maxInclusive )
{
	//GUARANTEE_OR_DIE( minInclusive <= maxInclusive, "NEED: minInclusive <= maxInclusive && maxInclusive >= 0.f" );
	if (minInclusive == maxInclusive) {
		return minInclusive;
	}
	if (minInclusive > maxInclusive) {
		return Get1dNoiseZeroToOne( m_position++, m_seed ) * (minInclusive - maxInclusive) + maxInclusive;
	}
	return Get1dNoiseZeroToOne( m_position++, m_seed ) * (maxInclusive - minInclusive) + minInclusive;
}

float RandomNumberGenerator::RollRandomFloatInRange( FloatRange const& inclusiveFloatRange )
{
	return RollRandomFloatInRange( inclusiveFloatRange.m_min, inclusiveFloatRange.m_max );
}
