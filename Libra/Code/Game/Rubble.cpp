#include "Game/Rubble.hpp"
#include "Game/Game.hpp"

Rubble::Rubble( Vec2 const& startPos, Map* map )
	:Entity(startPos, map)
{
	m_speed = 0.f;
	m_velocity = Vec2( 0, 0 );
	//m_orientationDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
	m_orientationDegrees = 0.f;
	m_angularVelocity = 0.f;

	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.5f;

	m_maxHealth = 100.f;
	m_health = m_maxHealth;

	m_type = EntityType::_RUBBLE;
	m_faction = EntityFaction::FACTION_NEUTRAL;

	m_target = nullptr;
	m_curState = 1;

	m_shootTime = 0.f;
	m_damage = 0.f;

	m_isPushedByEntities = false;
	m_doesPushEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = false;
	m_isActor = false;
	m_isRanged = false;

	m_driveHalfAngleDegrees = 0.f;
	m_shootHalfAngleDegrees = 0.f;
	m_color = Rgba8( 50, 50, 50, 150 );

	constexpr int NUM_OF_RUBBLE_VERTS = 48;
	float randR[NUM_OF_RUBBLE_VERTS / 3];
	for (int i = 0; i < NUM_OF_RUBBLE_VERTS / 3; i++) {
		randR[i] = g_theGame->m_randNumGen->RollRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );
	}
	for (int i = 0; i < NUM_OF_RUBBLE_VERTS / 3; i++) {
		m_verts.emplace_back( m_position, m_color );
		m_verts.emplace_back( m_position + Vec2( CosRadians( 6 * PI / NUM_OF_RUBBLE_VERTS * i ) * randR[i], SinRadians( 6 * PI / NUM_OF_RUBBLE_VERTS * i ) * randR[i] ), m_color );
		m_verts.emplace_back( m_position + Vec2( CosRadians( 6 * PI / NUM_OF_RUBBLE_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_RUBBLE_VERTS / 3)], SinRadians( 6 * PI / NUM_OF_RUBBLE_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_RUBBLE_VERTS / 3)] ), m_color );
	}
}

Rubble::~Rubble()
{

}

void Rubble::Update( float deltaTime )
{
	UNUSED( deltaTime );
}

void Rubble::Render() const
{
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( m_verts );
}

void Rubble::Die()
{

}

