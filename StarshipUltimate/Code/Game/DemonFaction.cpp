#include "Game/DemonFaction.hpp"
#include "Game/Game.hpp"
#include "Game/Room.hpp"

LittleDemon::LittleDemon( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( 1.2f, m_clock );
	m_stateTimer->Start();
	m_state3CoolDownTimer = new Timer( 20.f, m_clock );
	m_state3CoolDownTimer->Start();
	m_state1ShootTimer = new Timer( 0.3f, m_clock );
	m_state3TeleportTimer = new Timer( 1.f, m_clock );
	m_state = 0;
}

LittleDemon::~LittleDemon()
{
	delete m_stateTimer;
	delete m_state3CoolDownTimer;
	delete m_state1ShootTimer;
	delete m_state3TeleportTimer;
}

void LittleDemon::BeginPlay()
{

}

void LittleDemon::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	Vec2 vecToPlayer = (m_target->m_position - m_position).GetNormalized();

	if (m_stateTimer->HasPeriodElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 5.f );
			m_stateTimer->Start();
			m_state1ShootTimer->SetPeriodSeconds( 0.3f );
			m_state1ShootTimer->Start();
		}
		else if (m_state == 1) {
			m_state = 2;
			m_stateTimer->SetPeriodSeconds( 3.f );
			m_stateTimer->Start();
			AddImpulse( vecToPlayer * 40.f );
			m_dashDir = vecToPlayer;
		}
		else if (m_state == 2) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 5.f );
			m_stateTimer->Start();
			m_state1ShootTimer->SetPeriodSeconds( 0.3f );
			m_state1ShootTimer->Start();
		}
		else if (m_state == 3) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 5.f );
			m_stateTimer->Start();
			m_state3CoolDownTimer->Start();
			m_state1ShootTimer->SetPeriodSeconds( 0.3f );
			m_state1ShootTimer->Start();
		}
	}

	if ((m_state == 1 || m_state == 2) && m_state3CoolDownTimer->HasPeriodElapsed()) {
		if (GetDistanceSquared2D( m_position, m_target->m_position ) < 1600.f) {
			m_state = 3;
			m_stateTimer->SetPeriodSeconds( 5.f );
			m_stateTimer->Start();
			m_state3TeleportTimer->Start();
			m_state3TeleportTimer->SetElapsedTime( 1.f );
		}
	}

	float orientation = vecToPlayer.GetOrientationDegrees();
	if (m_state == 1) {
		if (m_numOfBullets < 6 && m_state1ShootTimer->DecrementPeriodIfElapsed()) {
			m_numOfBullets++;
			Projectile* proj = g_theGame->SpawnProjectileToGame( ProjectileDefinition::GetDefinition( "DemonBullet" ), m_position, orientation, vecToPlayer * ProjectileDefinition::GetDefinition( "DemonBullet" ).m_speed );
			proj->m_damage = 1.f;
			proj->m_faction = m_def.m_faction;
			if (m_numOfBullets == 6) {
				m_state1ShootTimer->SetPeriodSeconds( 1.f );
				m_state1ShootTimer->Start();
			}
		}
		else if (m_numOfBullets == 6 && m_state1ShootTimer->HasPeriodElapsed()) {
			m_state1ShootTimer->SetPeriodSeconds( 0.3f );
			m_state1ShootTimer->Start();
			m_numOfBullets = 0;
		}
	}
	else if (m_state == 2) {
		AddForce( m_velocity * 10.f );
		BounceOffEdges( g_theGame->m_curRoom->m_bounds );
	}
	else if (m_state == 3) {
		if (m_state3TeleportTimer->DecrementPeriodIfElapsed()) {
			Vec2 positionToTeleport;
			do {
				positionToTeleport = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
			} while (GetDistanceSquared2D( positionToTeleport, m_target->m_position ) < 2500.f);
			m_position = positionToTeleport;
			// shoot to 8 directions
			float bulletOrientation = 0.f;
			constexpr float degreesPerBullet = 45.f;
			for (int i = 0; i < 8; i++) {
				Projectile* proj = g_theGame->SpawnProjectileToGame( ProjectileDefinition::GetDefinition( "DemonBullet" ), m_position, bulletOrientation, Vec2::MakeFromPolarDegrees( bulletOrientation ) * ProjectileDefinition::GetDefinition( "DemonBullet" ).m_speed );
				proj->m_damage = 1.f;
				proj->m_faction = m_def.m_faction;
				bulletOrientation += degreesPerBullet;
			}
		}
	}
}

void LittleDemon::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(0.f, 0.f), m_cosmeticRadius, Rgba8(153, 0, 0));
	AddVertsForDisc2D( verts, Vec2( m_cosmeticRadius * 0.3f, m_cosmeticRadius * 0.25f ), m_cosmeticRadius * 0.15f, Rgba8( 255, 178, 102 ) );
	AddVertsForDisc2D( verts, Vec2( -m_cosmeticRadius * 0.3f, m_cosmeticRadius * 0.25f ), m_cosmeticRadius * 0.15f, Rgba8( 255, 178, 102 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void LittleDemon::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void LittleDemon::BounceOffEdges( AABB2 const& edges )
{
	if (m_position.x > edges.m_maxs.x - m_physicsRadius) {
		m_position.x = edges.m_maxs.x - m_physicsRadius;
		m_velocity.Reflect( Vec2( -1.f, 0.f ) );
	}
	if (m_position.x < edges.m_mins.x + m_physicsRadius) {
		m_position.x = edges.m_mins.x + m_physicsRadius;
		m_velocity.Reflect( Vec2( 1.f, 0.f ) );
	}
	if (m_position.y < edges.m_mins.y + m_physicsRadius) {
		m_position.y = edges.m_mins.y + m_physicsRadius;
		m_velocity.Reflect( Vec2( 0.f, 1.f ) );
	}
	if (m_position.y > edges.m_maxs.y - m_physicsRadius) {
		m_position.y = edges.m_maxs.y - m_physicsRadius;
		m_velocity.Reflect( Vec2( 0.f, -1.f ) );
	}
}
