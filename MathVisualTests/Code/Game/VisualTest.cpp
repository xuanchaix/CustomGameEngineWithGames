#include "Game/VisualTest.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

VisualTest::VisualTest()
{
	m_randNumGen = new RandomNumberGenerator();
}

VisualTest::~VisualTest()
{
	delete m_randNumGen;
	m_randNumGen = nullptr;
}

Vec2 const VisualTest::GetRandomOffMapPosition()
{
	// get a random position
	float x = m_randNumGen->RollRandomFloatZeroToOne() * WORLD_SIZE_X;
	float y = m_randNumGen->RollRandomFloatZeroToOne() * WORLD_SIZE_Y;

	// put the entity to the position just off screen
	if (x < WORLD_SIZE_X - x && x < WORLD_SIZE_Y - y && x < y) {
		x = -1.f;
	}
	else if (WORLD_SIZE_X - x < x && WORLD_SIZE_X - x < y && WORLD_SIZE_X - x < WORLD_SIZE_Y - y) {
		x = WORLD_SIZE_X + 1.f;
	}
	else if (y < WORLD_SIZE_X - x && y < x && y < WORLD_SIZE_Y - y) {
		y = -1.f;
	}
	else {
		y = WORLD_SIZE_Y + 1.f;
	}
	return Vec2( x, y );
}

