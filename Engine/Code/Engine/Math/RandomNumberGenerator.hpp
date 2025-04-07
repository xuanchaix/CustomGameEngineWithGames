#pragma once

//-----------random generator 9.2.2023---------------
struct IntRange;
struct FloatRange;

class RandomNumberGenerator {
public:
	RandomNumberGenerator( unsigned int seed=0 );
	void SetSeed( unsigned int newSeed ) { m_seed = newSeed; m_position = 0; }
	unsigned int GetSeed() const { return m_seed; }
	int RollRandomIntLessThan( int maxNotInclusive ); // -
	int RollRandomIntInRange( int minInclusive, int maxInclusive ); // -
	int RollRandomIntInRange( IntRange const& inclusiveIntRange ); // -
	int RollRandomPositiveNegative();
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange( float minInclusive, float maxInclusive );
	float RollRandomFloatInRange( FloatRange const& inclusiveFloatRange );
private:
	unsigned int m_seed = 0;
	int m_position = 0;
};