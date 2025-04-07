#include "Game/DiamondFraction.hpp"
#include "Game/Game.hpp"
#include "Game/Controller.hpp"
#include "Game/Room.hpp"

void DiamondFactionSpawnDeathEffect( Entity* deadEntity ) {
	ParticleSystem2DAddEmitter( 200, 0.1f, AABB2( deadEntity->m_position, deadEntity->m_position ),
		FloatRange( deadEntity->m_cosmeticRadius * 0.6f, deadEntity->m_cosmeticRadius ),
		AABB2( Vec2( -40.f, -40.f ) + deadEntity->m_velocity * 0.5f, Vec2( 40.f, 40.f ) + deadEntity->m_velocity * 0.5f ),
		FloatRange( 0.4f, 0.7f ), deadEntity->m_def.m_deathParticleColor, Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 40.f, 75.f ), nullptr,
		Rgba8( deadEntity->m_def.m_deathParticleColor.r, deadEntity->m_def.m_deathParticleColor.g, deadEntity->m_def.m_deathParticleColor.b, 0 ) );
}

DiamondDoubleRayShooter::DiamondDoubleRayShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_target = g_theGame->GetPlayerEntity();
	float rnd = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, m_def.m_shootCoolDown * 0.2f );
	m_stateTimer1 = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer1->Start();
	m_stateTimer1->SetElapsedTime( m_def.m_shootCoolDown * 0.5f + rnd );
	m_stateTimer2 = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer2->Start();
	m_stateTimer1->SetElapsedTime( rnd );
}

DiamondDoubleRayShooter::~DiamondDoubleRayShooter()
{
	delete m_stateTimer1;
	delete m_stateTimer2;
}

void DiamondDoubleRayShooter::BeginPlay()
{

}

void DiamondDoubleRayShooter::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_stateTimer1->DecrementPeriodIfElapsed()) {
		if (m_state1 == 0) {
			//if (GetDistanceSquared2D( m_position, m_target->m_position ) <= 2500.f) {
			m_state1 = 1;
			// aiming
			m_stateTimer1->SetPeriodSeconds( 0.6f );
			m_stateTimer1->Start();
			//}
		}
		else if (m_state1 == 1) {
			// shoot
			m_state1 = 2;
			m_stateTimer1->SetPeriodSeconds( 0.2f );
			m_stateTimer1->Start();
			Shoot( m_displacement );
		}
		else if (m_state1 == 2) {
			// wondering
			m_state1 = 0;
			m_stateTimer1->SetPeriodSeconds( m_def.m_shootCoolDown );
			m_stateTimer1->Start();
		}
	}

	if (m_stateTimer2->DecrementPeriodIfElapsed()) {
		if (m_state2 == 0) {
			if (GetDistanceSquared2D( m_position, m_target->m_position ) <= 2500.f) {
				m_state2 = 1;
				// aiming
				m_stateTimer2->SetPeriodSeconds( 0.6f );
				m_stateTimer2->Start();
			}
		}
		else if (m_state2 == 1) {
			// shoot
			m_state2 = 2;
			m_stateTimer2->SetPeriodSeconds( 0.2f );
			m_stateTimer2->Start();
			Shoot( -m_displacement );
		}
		else if (m_state2 == 2) {
			// wondering
			m_state2 = 0;
			m_stateTimer2->SetPeriodSeconds( m_def.m_shootCoolDown );
			m_stateTimer2->Start();
		}
	}

	if (m_state1 == 0 && m_state2 == 0) {
		Vec2 forwardNormal = GetForwardNormal();
		float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
		if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 10.f
			&& GetDistanceSquared2D( m_position, m_target->m_position ) >= 1600.f) {
			AddForce( forwardNormal * m_def.m_flySpeed );
		}
	}
}

void DiamondDoubleRayShooter::Render() const
{
	Vec2 forwardVec = GetForwardNormal();
	Vec2 leftVec = forwardVec.GetRotated90Degrees();
	if (m_state1 == 1) {
		DebugDrawLine( m_position + leftVec * m_displacement, m_orientationDegrees, 50.f, 0.3f, Rgba8::Interpolate( Rgba8( 255, 0, 0, 0 ), Rgba8( 255, 0, 0, 200 ), m_stateTimer1->GetElapsedFraction() ) );
	}
	if (m_state2 == 1) {
		DebugDrawLine( m_position - leftVec * m_displacement, m_orientationDegrees, 50.f, 0.3f, Rgba8::Interpolate( Rgba8( 255, 0, 0, 0 ), Rgba8( 255, 0, 0, 200 ), m_stateTimer2->GetElapsedFraction() ) );
	}

	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, AABB2( Vec2( -2.3f, -2.3f ), Vec2( 2.3f, 2.3f ) ), Rgba8( 192, 192, 192 ) );
	AddVertsForDisc2D( verts, Vec2( 1.9f, m_displacement ), 1.f, Rgba8( 192, 192, 192 ) );
	AddVertsForDisc2D( verts, Vec2( 1.9f, -m_displacement ), 1.f, Rgba8( 192, 192, 192 ) );
	if (m_def.m_isShielded) {
		AddVertsForDisc2D( verts, Vec2(), 0.4f, Rgba8( 102, 178, 255, 128 ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondDoubleRayShooter::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void DiamondDoubleRayShooter::Shoot( float displacement )
{
	Vec2 forwardVec = GetForwardNormal();
	Vec2 rayStartPos = m_position + forwardVec.GetRotated90Degrees() * displacement;
	Fire( forwardVec, rayStartPos, true );
}

DiamondReflector::DiamondReflector( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	m_reflectionTimer = new Timer( 0.2f, m_clock );
	m_stateTimer->Start();
	m_stateTimer->SetElapsedTime( g_theGame->m_randNumGen->RollRandomFloatInRange( Minf( 0.5f, m_def.m_shootCoolDown ), m_def.m_shootCoolDown ) );
	m_state = 0;
	m_targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees() + g_theGame->m_randNumGen->RollRandomPositiveNegative() * g_theGame->m_randNumGen->RollRandomFloatInRange( 5.f, 10.f );

}

DiamondReflector::~DiamondReflector()
{
	delete m_reflectionTimer;
	delete m_stateTimer;
}

void DiamondReflector::BeginPlay()
{

}

void DiamondReflector::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_reflectionTimer->HasPeriodElapsed()) {
		m_reflectionTimer->Stop();
		// shoot
		m_state = 2;
		m_stateTimer->SetPeriodSeconds( 0.2f );
		m_stateTimer->Start();
		m_isTriggeredByReflection = true;
		for (int i = 0; i < m_numOfReflections; i++) {
			float forwardDegrees = m_reflectionDirection.GetOrientationDegrees() - 60.f;
			Vec2 forwardVec = Vec2::MakeFromPolarDegrees( forwardDegrees + i * 120.f / (m_numOfReflections - 1) );
			ShootRay( forwardVec );
		}
	}

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			// aiming
			m_stateTimer->SetPeriodSeconds( 0.6f );
			m_stateTimer->Start();
		}
		else if (m_state == 1) {
			// shoot
			m_state = 2;
			m_stateTimer->SetPeriodSeconds( 0.2f );
			m_stateTimer->Start();
			m_isTriggeredByReflection = false;
			for (int i = 0; i < m_numOfRays; i++) {
				Vec2 forwardVec = Vec2::MakeFromPolarDegrees( m_orientationDegrees + i * 360.f / m_numOfRays );
				ShootRay( forwardVec );
			}
		}
		else if (m_state == 2) {
			// slowly wondering
			m_state = 0;
			m_targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees() + g_theGame->m_randNumGen->RollRandomPositiveNegative() * g_theGame->m_randNumGen->RollRandomFloatInRange( 5.f, 10.f );
			m_stateTimer->SetPeriodSeconds( m_def.m_shootCoolDown );
			m_stateTimer->Start();
		}
	}

	if (m_state == 0) {
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, m_targetDegrees, m_def.m_turnSpeed * deltaTime ) );
		AddForce( GetForwardNormal() * m_def.m_flySpeed );
	}
}

void DiamondReflector::Render() const
{
	if (m_state == 1) {
		for (int i = 0; i < m_numOfRays; i++) {
			DebugDrawLine( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + 360.f / (float)m_numOfRays * i ) * m_cosmeticRadius,
				m_orientationDegrees + 360.f / (float)m_numOfRays * i, 50.f, 0.3f,
				Rgba8::Interpolate( Rgba8( 255, 0, 0, 0 ), Rgba8( 255, 0, 0, 200 ), m_stateTimer->GetElapsedFraction() ) );
		}
	}

	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	for (int i = 0; i < m_numOfRays; i++) {
		AddVertsForDisc2D( verts, Vec2::MakeFromPolarDegrees( 360.f / (float)m_numOfRays * i ) * m_cosmeticRadius, 1.2f, Rgba8( 192, 192, 192 ) );
	}
	AddVertsForDisc2D( verts, Vec2( 0.f, 0.f ), m_cosmeticRadius, Rgba8( 192, 192, 192 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondReflector::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void DiamondReflector::TriggerShootRays( Vec2 const& dirction )
{
	if (m_reflectionTimer->IsStopped()) {
		m_numOfReflections = 3;
		m_reflectionTimer->Start();
		m_reflectionDirection = dirction;
	}
	else {
		m_numOfReflections = GetClamped( m_numOfReflections + 2, 0, m_numOfRays * 2 );
	}
}

void DiamondReflector::ShootRay( Vec2 const& forwardVec )
{
	Vec2 startPos = m_position + forwardVec * m_cosmeticRadius;
	Fire( forwardVec, startPos, true );
}

DiamondStriker::DiamondStriker( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( 3.f, m_clock );
	m_stateTimer->Start();
	m_stateTimer->SetElapsedTime( g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, 1.5f ) );
	m_state = 0;
	ResetRandomWonderingPos();
}

DiamondStriker::~DiamondStriker()
{
	delete m_stateTimer;
}

void DiamondStriker::BeginPlay()
{
}

void DiamondStriker::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			// goto striking
			m_velocity = (m_target->m_position - m_position).GetNormalized() * 50.f;
			m_stateTimer->SetPeriodSeconds( 4.f );
			m_stateTimer->Start();
		}
		else if (m_state == 1) {
			// goto wonder
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( 3.f );
			m_stateTimer->Start();
			ResetRandomWonderingPos();
		}
	}

	// wonder
	if (m_state == 0) {
		if (IsPointInsideDisc2D( m_wonderingPos, m_position, m_physicsRadius )) {
			ResetRandomWonderingPos();
		}
		float targetDegrees = (m_wonderingPos - m_position).GetOrientationDegrees();
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
		if (m_stateTimer->GetElapsedFraction() < 0.9f) {
			if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 10.f) {
				AddForce( GetForwardNormal() * m_def.m_flySpeed );
			}
		}
		else {
			m_orientationDegrees += m_def.m_turnSpeed * 10.f * deltaTime;
		}
	}
	else if (m_state == 1) {
		AddForce( m_velocity.GetNormalized() * m_def.m_flySpeed * 5.f );
		m_orientationDegrees += m_def.m_turnSpeed * 5.f * deltaTime;
		BounceOffEdges( g_theGame->m_curRoom->m_bounds );
	}
}

void DiamondStriker::Render() const
{
	Rgba8 bladeColor = Rgba8( 128, 128, 128 );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	verts.emplace_back( Vec2( 1.2f, 1.2f ), bladeColor );
	verts.emplace_back( Vec2( 1.2f, -1.2f ), bladeColor );
	verts.emplace_back( Vec2( 3.6f, 0.f ), bladeColor );
	verts.emplace_back( Vec2( 1.2f, 1.2f ), bladeColor );
	verts.emplace_back( Vec2( 0.f, 3.6f ), bladeColor );
	verts.emplace_back( Vec2( -1.2f, 1.2f ), bladeColor );
	verts.emplace_back( Vec2( -1.2f, 1.2f ), bladeColor );
	verts.emplace_back( Vec2( -3.6f, 0.f ), bladeColor );
	verts.emplace_back( Vec2( -1.2f, -1.2f ), bladeColor );
	verts.emplace_back( Vec2( -1.2f, -1.2f ), bladeColor );
	verts.emplace_back( Vec2( 0.f, -3.6f ), bladeColor );
	verts.emplace_back( Vec2( 1.2f, -1.2f ), bladeColor );
	AddVertsForDisc2D( verts, Vec2( 0.f, 0.f ), 2.3f, Rgba8( 192, 192, 192 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondStriker::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void DiamondStriker::ResetRandomWonderingPos()
{
	Vec2 wonderingVec, targetVec;
	AABB2 box = AABB2( g_theGame->m_curRoom->m_bounds.m_mins + Vec2( 10.f, 10.f ), g_theGame->m_curRoom->m_bounds.m_maxs - Vec2( 10.f, 10.f ) );
	do {
		m_wonderingPos = box.GetRandomPointInside();
		wonderingVec = m_wonderingPos - m_position;
		targetVec = m_target->m_position - m_position;
	} while (GetDistanceSquared2D( m_wonderingPos, m_target->m_position ) < 3600.f && DotProduct2D( wonderingVec, targetVec ) > 0.f);
}

void DiamondStriker::BounceOffEdges( AABB2 const& edges )
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

DiamondRayShooter::DiamondRayShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer->Start();
	m_stateTimer->SetElapsedTime( g_theGame->m_randNumGen->RollRandomFloatInRange( 0.1f, m_def.m_shootCoolDown ) );
	m_state = 0;
}

DiamondRayShooter::~DiamondRayShooter()
{
	delete m_stateTimer;
}

void DiamondRayShooter::BeginPlay()
{

}

void DiamondRayShooter::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	Entity* player = g_theGame->GetPlayerEntity();
	if (m_target->m_isDead) {
		m_target = player;
	}

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			//if (GetDistanceSquared2D( m_position, m_target->m_position ) <= 2500.f) {
			m_state = 1;
			// aiming
			m_stateTimer->SetPeriodSeconds( 0.6f );
			m_stateTimer->Start();
			//}
		}
		else if (m_state == 1) {
			// shoot
			m_state = 2;
			m_stateTimer->SetPeriodSeconds( 0.2f );
			m_stateTimer->Start();
			Fire( GetForwardNormal(), m_position );
		}
		else if (m_state == 2) {
			// wondering
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( m_def.m_shootCoolDown );
			m_stateTimer->Start();
		}
	}

	if (m_state == 0) {
		Vec2 forwardNormal = GetForwardNormal();
		std::vector<DiamondReflector*> allReflectors = g_theGame->GetAllDiamondReflectors();
		for (auto reflector : allReflectors) {
			if (reflector->IsAlive()
				&& GetDistanceSquared2D( reflector->m_position, m_position ) < GetDistanceSquared2D( m_target->m_position, m_position )
				&& DotProduct2D( (reflector->m_position - m_position).GetNormalized(), (player->m_position - reflector->m_position).GetNormalized() ) > 0.5f) {
				m_target = reflector;
			}
		}
		if (GetDistanceSquared2D( player->m_position, m_position ) - 1000.f < GetDistanceSquared2D( m_target->m_position, m_position )) {
			m_target = player;
		}
		float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
		if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 10.f
			&& GetDistanceSquared2D( m_position, m_target->m_position ) >= 1600.f) {
			AddForce( forwardNormal * m_def.m_flySpeed );
		}
	}
}

void DiamondRayShooter::Render() const
{
	if (m_state == 1) {
		DebugDrawLine( m_position, m_orientationDegrees, 50.f, 0.3f, Rgba8::Interpolate( Rgba8( 255, 0, 0, 0 ), Rgba8( 255, 0, 0, 200 ), m_stateTimer->GetElapsedFraction() ) );
	}
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, AABB2( Vec2( -1.5f, -1.5f ), Vec2( 1.5f, 1.5f ) ), Rgba8( 192, 192, 192 ) );
	AddVertsForDisc2D( verts, Vec2( 1.3f, 0.f ), 1.f, Rgba8( 192, 192, 192 ) );
	if (m_def.m_isShielded) {
		AddVertsForDisc2D( verts, Vec2(), 1.f, Rgba8( 102, 178, 255, 128 ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondRayShooter::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

DiamondWarrior::DiamondWarrior( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( 1.f, m_clock );
	m_stateTimer->SetElapsedTime( g_theGame->m_randNumGen->RollRandomFloatInRange( 0.5f, 1.f ) );
	m_stateTimer->Start();
}

DiamondWarrior::~DiamondWarrior()
{

}

void DiamondWarrior::BeginPlay()
{

}

void DiamondWarrior::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			// Dash
			m_state = 1;
			AddImpulse( GetForwardNormal() * 150.f );
			m_stateTimer->SetPeriodSeconds( 0.5f );
			m_stateTimer->Start();
		}
		else {
			// Turn to player
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( 1.f );
			m_stateTimer->Start();
		}
	}

	if (m_state == 0) {
		float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
	}
	else if (m_state == 1) {
		AddForce( GetForwardNormal() * m_def.m_flySpeed );
	}
}

void DiamondWarrior::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForAABB2D( verts, AABB2( Vec2( -1.5f, -1.5f ), Vec2( 1.5f, 1.5f ) ), Rgba8( 192, 192, 192 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondWarrior::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

DiamondMiner::DiamondMiner( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_target = g_theGame->GetPlayerEntity();
	float rnd = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, m_def.m_shootCoolDown * 0.5f );
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer->Start();
	m_stateTimer->SetElapsedTime( rnd );
	m_wonderingPos = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
}

DiamondMiner::~DiamondMiner()
{
	delete m_stateTimer;
}

void DiamondMiner::BeginPlay()
{

}

void DiamondMiner::Update( float deltaTime )
{
	if (m_isDead) {
		if (m_stateTimer->HasPeriodElapsed()) {
			m_isGarbage = true;
			if (m_hasReward) {
				SpawnReward();
			}
		}
		return;
	}
	Entity::Update( deltaTime );
	// if have bomb
	if (m_bombCount > 0 && m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			// go to throw bomb
			m_stateTimer->SetPeriodSeconds( 0.5f );
			m_stateTimer->Start();
			Vec2 targetPos = GetRandomPointInDisc2D( m_target->m_physicsRadius * 3.f, m_target->m_position );
			ThrowBomb( targetPos, 1.2f );
			m_bombCount--;
			if (m_bombCount == 0) {
				m_stateTimer->SetPeriodSeconds( 3.f );
				m_stateTimer->Start();
				return;
			}
		}
		else {
			m_state = 0;
			// go to wonder
			m_stateTimer->SetPeriodSeconds( m_def.m_shootCoolDown );
			m_stateTimer->Start();
			m_wonderingPos = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
		}
	}

	// has no bomb: rush to player and explode
	if (m_bombCount == 0) {
		if (m_stateTimer->GetElapsedFraction() > 0.3f) {
			float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
			if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 5.f || m_isRushing) {
				m_isRushing = true;
				m_wonderingPos = m_target->m_position;
				AddForce( GetForwardNormal() * m_def.m_flySpeed * 7.f );
			}
			else {
				SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime * 2.f ) );
			}
			if (m_stateTimer->HasPeriodElapsed() || (m_isRushing && IsPointInsideDisc2D( m_wonderingPos, m_position, m_physicsRadius * 1.5f ))
				|| CollideWithEdges()) {
				Die();
			}
		}
	}
	else if (m_state == 0) {
		if (IsPointInsideDisc2D( m_wonderingPos, m_position, m_physicsRadius )) {
			m_wonderingPos = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
		}
		if (m_stateTimer->GetElapsedFraction() < 0.6f) {
			float targetDegrees = (m_wonderingPos - m_position).GetOrientationDegrees();
			SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
			if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 10.f) {
				AddForce( GetForwardNormal() * m_def.m_flySpeed );
			}
		}
		else {
			float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
			SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
		}
	}
	else if (m_state == 1) {
		float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
	}
	
}

void DiamondMiner::Render() const
{
	if (m_isDead) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForAABB2D( verts, AABB2( Vec2( -1.8f, -1.8f ), Vec2( 1.8f, 1.8f ) ), Rgba8( 192, 192, 192 ) );
	float minDegrees = 120.f;
	float stepDegrees = 120.f / (float)(m_bombCount + 1);
	for (int i = 0; i < m_bombCount; i++) {
		AddVertsForDisc2D( verts, Vec2::MakeFromPolarDegrees( minDegrees + (float)(i + 1) * stepDegrees, 2.3f ), 0.8f, Rgba8( 153, 76, 0 ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondMiner::Die()
{
	ThrowBomb( m_position, 0.01f );
	DiamondFactionSpawnDeathEffect( this );
	m_stateTimer->SetPeriodSeconds( 0.6f );
	m_stateTimer->Start();
	m_isDead = true;
}

void DiamondMiner::ThrowBomb( Vec2 const& position, float flyTime )
{
	Missle* missile = (Missle*)g_theGame->SpawnEffectToGame(EffectType::Missile, m_position);
	missile->m_startPos = m_position;
	missile->m_targetPos = position;
	missile->m_owner = this;
	missile->m_flyTime = flyTime;
	missile->BeginPlay();
}

bool DiamondMiner::CollideWithEdges()
{
	AABB2 const& bounds = g_theGame->m_curRoom->m_bounds;
	return m_position.x + m_physicsRadius > bounds.m_maxs.x || m_position.x - m_physicsRadius < bounds.m_mins.x
		|| m_position.y + m_physicsRadius > bounds.m_maxs.y || m_position.y - m_physicsRadius < bounds.m_mins.y;
}

DiamondBossAssassin::DiamondBossAssassin( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	,m_enterSpline(g_theGame->m_curRoom->m_bounds.m_maxs - Vec2(m_cosmeticRadius, m_cosmeticRadius), Vec2(-50.f, -50.f), g_theGame->m_curRoom->m_bounds.GetCenter(), Vec2(50.f, -50.f))
	,m_projDef(ProjectileDefinition::GetDefinition("Shuriken"))
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( 1.5f, m_clock );
	m_shurikenShootTimer = new Timer( 0.8f, m_clock );
	m_stateTimer->Start();
	m_dodgeCoolDownTimer = new Timer( 10.f, m_clock );
	m_dodgeCoolDownTimer->Start();
	m_enterCurveLength = m_enterSpline.GetApproximateLength();
}

DiamondBossAssassin::~DiamondBossAssassin()
{
	delete m_stateTimer;
	delete m_shurikenShootTimer;
	delete m_dodgeCoolDownTimer;
	m_stateTimer = nullptr;
	m_shurikenShootTimer = nullptr;
	m_dodgeCoolDownTimer = nullptr;
}

void DiamondBossAssassin::BeginPlay()
{

}

void DiamondBossAssassin::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_state == 4) {
		// enter the game
		if (m_stateTimer->HasPeriodElapsed()) {
			GotoState( 0 );
		}
		else {
			m_orientationDegrees += deltaTime * 180.f;
			m_position = m_enterSpline.EvaluateAtApproximateDistance( SmoothStop2( m_stateTimer->GetElapsedFraction() ) * m_enterCurveLength );
		}
	}
	else if (m_state == 0) {
		// dash to player
		if (m_stateTimer->HasPeriodElapsed()) {
			GotoState( 1 );
		}
		else {
			// shoot shuriken to 8 directions
			if (!m_shurikenShootTimer->HasStartedAndNotPeriodElapsed()) {
				constexpr float stepDegrees = 360.f / 8.f;
				for (int i = 0; i < 8; i++) {
					Projectile* projectile = g_theGame->SpawnProjectileToGame( m_projDef, m_position, m_orientationDegrees + stepDegrees * i, Vec2::MakeFromPolarDegrees( m_orientationDegrees + stepDegrees * i ) * m_projDef.m_speed );
					projectile->m_faction = m_def.m_faction;
				}
				m_shurikenShootTimer->Start();
			}
			// follow the player in a slow speed
			Vec2 targetVec = m_target->m_position - m_position;
			float targetDegrees = targetVec.GetOrientationDegrees();
			if (abs( targetVec.x ) > abs( targetVec.y )) {
				targetVec.y = 0.f;
				targetVec.x = targetVec.x / abs( targetVec.x );
			}
			else {
				targetVec.x = 0.f;
				targetVec.y = targetVec.y / abs( targetVec.y );
			}
			AddForce( targetVec * m_def.m_flySpeed * 0.35f );
			SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
		}
	}
	else if (m_state == 1) {
		if (m_stateTimer->HasPeriodElapsed()) {
			m_disableFriction = false;
			GotoState( 2 );
		}
		else {
			if (m_stateTimer->GetElapsedFraction() < 0.1f) {
				m_orientationDegrees += m_def.m_turnSpeed * 10.f * deltaTime;
			}
			else {
				if (m_readyToDash) {
					Vec2 targetVec = (m_target->m_position - m_position).GetNormalized();
					m_disableFriction = true;
					AddImpulse( targetVec * 100.f );
					m_readyToDash = false;
				}
				AddForce( Vec2( 0.f, -1.f ) * m_def.m_flySpeed );
				m_orientationDegrees += m_def.m_turnSpeed * 6.f * deltaTime;
				BounceOffEdges( g_theGame->m_curRoom->m_bounds );
			}
		}
	}
	else if (m_state == 2) {
		if (m_stateTimer->HasPeriodElapsed()) {
			GotoState( 0 );
			m_disableFriction = false;
		}
		else {
			// always rotating to player
			float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
			SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );

			// climb to rope
			if (m_stateTimer->GetElapsedFraction() > 0.1f && m_stateTimer->GetElapsedFraction() < 0.2f) {
				AddForce( (m_hangingPos - m_position) * 5.f );
			}
			else if (m_stateTimer->GetElapsedFraction() >= 0.2f) {
				float length = (m_hangingPos - m_position).GetLength();
				Vec2 forwardNormal = (m_hangingPos - m_position) / length;
				float speed = 30.f;
				float a = speed * speed / length;
				AddForce( forwardNormal * a );
				if (!m_isImpulseAdded) {
					m_velocity = Vec2();
					m_isImpulseAdded = true;
					if (forwardNormal.x < 0.f) {
						AddImpulse( forwardNormal.GetRotated90Degrees() * speed );
					}
					else {
						AddImpulse( forwardNormal.GetRotatedMinus90Degrees() * speed );
					}
				}
				// shoot shuriken
				if (!m_shurikenShootTimer->HasStartedAndNotPeriodElapsed()) {
					Projectile* projectile = g_theGame->SpawnProjectileToGame( m_projDef, m_position, m_orientationDegrees, Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * m_projDef.m_speed );
					projectile->m_faction = m_def.m_faction;
					m_shurikenShootTimer->Start();
				}
			}
		}
	}
	else if (m_state == 3) {
		if (m_stateTimer->HasPeriodElapsed()) {
			GotoState( g_theGame->m_randNumGen->RollRandomIntInRange( 0, 2 ) );
			m_isInvincible = false;
		}
		else if (m_stateTimer->GetElapsedFraction() >= 0.6f) {
			if (!m_hasDoneFlash) {
				m_hasDoneFlash = true;
				m_position = m_target->m_position - m_target->GetForwardNormal() * (m_target->m_cosmeticRadius + m_cosmeticRadius + 0.6f);
				DiamondFactionSpawnDeathEffect( this );
			}
			m_orientationDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
			if (m_stateTimer->GetElapsedFraction() >= 0.65f) {
				Fire( GetForwardNormal(), m_position );
			}
		}
	}
}

void DiamondBossAssassin::Render() const
{
	if (m_state == 3 && m_stateTimer->GetElapsedFraction() < 0.6f) {
		return;
	}
	Rgba8 bladeColor = Rgba8( 255, 215, 0 );
	Rgba8 eyeColor = Rgba8( 51, 255, 255 );
	float sideLength = 1.4f;
	std::vector<Vertex_PCU> verts;
	verts.reserve( 200 );
	verts.emplace_back( Vec2( sideLength, sideLength ), bladeColor );
	verts.emplace_back( Vec2( sideLength, -sideLength ), bladeColor );
	verts.emplace_back( Vec2( sideLength * 3.f, 0.f ), bladeColor );
	verts.emplace_back( Vec2( sideLength, sideLength ), bladeColor );
	verts.emplace_back( Vec2( 0.f, sideLength * 3.f ), bladeColor );
	verts.emplace_back( Vec2( -sideLength, sideLength ), bladeColor );
	verts.emplace_back( Vec2( -sideLength, sideLength ), bladeColor );
	verts.emplace_back( Vec2( -sideLength * 3.f, 0.f ), bladeColor );
	verts.emplace_back( Vec2( -sideLength, -sideLength ), bladeColor );
	verts.emplace_back( Vec2( -sideLength, -sideLength ), bladeColor );
	verts.emplace_back( Vec2( 0.f, -sideLength * 3.f ), bladeColor );
	verts.emplace_back( Vec2( sideLength, -sideLength ), bladeColor );
	AddVertsForDisc2D( verts, Vec2( 0.f, 0.f ), 2.6f, Rgba8( 192, 192, 192 ) );
	verts.emplace_back( Vec2( 0.f, 1.6f ), eyeColor );
	verts.emplace_back( Vec2( 0.f, -1.6f ), eyeColor );
	verts.emplace_back( Vec2( 1.2f, 0.f ), eyeColor );

	if (m_state == 2) {
		if (m_stateTimer->GetElapsedFraction() <= 0.1f) {
			DebugDrawLine( Interpolate(m_position, m_hangingPos, m_stateTimer->GetElapsedFraction() * 10.f ), m_position, 0.6f, Rgba8( 192, 192, 192 ) );
		}
		else {
			DebugDrawLine( m_hangingPos, m_position, 0.6f, Rgba8( 192, 192, 192 ) );
		}
		//AddVertsForLineSegment2D( verts, m_position, m_hangingPos, 1.f, Rgba8( 192, 192, 192 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondBossAssassin::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void DiamondBossAssassin::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = 1050.f;
	float endX = 1550.f;
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2(startX, Y), Vec2(endX, Y), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Diamond Assassin" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondBossAssassin::GotoState( int stateToGo )
{
	if (!m_dodgeCoolDownTimer->HasStartedAndNotPeriodElapsed()){
		if (g_theGame->IsPlayerBulletInRange( m_position, m_physicsRadius * 3.f ) || g_theGame->m_randNumGen->RollRandomFloatZeroToOne() < 0.3f) {
			m_dodgeCoolDownTimer->Start();
			m_state = 3;
			m_isInvincible = true;
			m_hasDoneFlash = false;
			m_stateTimer->SetPeriodSeconds( 2.2f );
			m_stateTimer->Start();
			DiamondFactionSpawnDeathEffect( this );
			return;
		}
	}
	if (stateToGo == 0) {
		// accelerate to player and shoot shuriken
		m_state = 0;
		m_stateTimer->SetPeriodSeconds( 6.f );
		m_stateTimer->Start();
		m_shurikenShootTimer->SetPeriodSeconds( 1.f );
	}
	else if (stateToGo == 1) {
		// bounce off edges and spawn strikers
		m_state = 1;
		m_stateTimer->SetPeriodSeconds( 5.f );
		m_stateTimer->Start();
		m_readyToDash = true;
		for (int i = 0; i < 2; i++) {
			DiamondStriker* striker = (DiamondStriker*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondStriker" ), g_theGame->m_curRoom->m_bounds.GetPointAtUV( Vec2( 0.3f + 0.4f * (float)i, 1.f ) ), 0.f );
			striker->m_state = 1;
			striker->m_velocity = (m_target->m_position - striker->m_position).GetNormalized() * 50.f;
			striker->m_stateTimer->SetPeriodSeconds( 4.f );
			striker->m_stateTimer->Start();
			striker->m_health = 3.f;
			striker->m_hasReward = false;
		}
	}
	else if (stateToGo == 2) {
		// hang on wall and shoot
		m_state = 2;
		m_stateTimer->SetPeriodSeconds( 5.f );
		m_stateTimer->Start();
		m_shurikenShootTimer->SetPeriodSeconds( 0.5f );
		m_disableFriction = true;
		m_isImpulseAdded = false;
		// choose a point to hang
		if (m_position.x < g_theGame->m_curRoom->m_bounds.GetCenter().x) {
			m_hangingPos = Vec2( g_theGame->m_curRoom->m_bounds.GetCenter().x + 50.f, g_theGame->m_curRoom->m_bounds.m_maxs.y );
		}
		else {
			m_hangingPos = Vec2( g_theGame->m_curRoom->m_bounds.GetCenter().x - 50.f, g_theGame->m_curRoom->m_bounds.m_maxs.y );
		}
		m_velocity = Vec2();
	}
}

void DiamondBossAssassin::BounceOffEdges( AABB2 const& edges )
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

DiamondBossSuperMiner::DiamondBossSuperMiner( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_minerDef(EntityDefinition::GetDefinition("DiamondMiner"))
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( 24.f, m_clock );
	m_stateTimer->Start();
	m_state = 0;
	m_shootTimer = new Timer( 0.8f, m_clock );
	m_shootTimer->Start();
	m_allyTimer = new Timer( 0.5f, m_clock );
	m_allyTimer->Start();
	m_enhanceTimer = new Timer( 24.f, m_clock );

	m_restrictIntoRoom = false;
	m_disableFriction = true;
	m_orientationDegrees = 0.f;

	m_north1Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_north2Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_south1Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_south2Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_west1Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_west2Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_east1Arch += g_theGame->m_curRoom->m_bounds.m_mins;
	m_east2Arch += g_theGame->m_curRoom->m_bounds.m_mins;

	/*
		north1-south1 0
		north2-south2 1
		west1-east1 2
		west2-east2 3
		south2-north2 4
		south1-north1 5
		east2-west2 6
		east1-west1 7
	*/

	float displacement = 15.f;
	float doubleDisplacement = 30.f;
	m_startPos[0] = m_north1Arch + Vec2( 0.f, displacement );
	m_startPos[1] = m_north2Arch + Vec2( 0.f, displacement );
	m_startPos[2] = m_west1Arch - Vec2( doubleDisplacement, 0.f );
	m_startPos[3] = m_west2Arch - Vec2( doubleDisplacement, 0.f );
	m_startPos[4] = m_south2Arch - Vec2( 0.f, displacement );
	m_startPos[5] = m_south1Arch - Vec2( 0.f, displacement );
	m_startPos[6] = m_east2Arch + Vec2( doubleDisplacement, 0.f );
	m_startPos[7] = m_east1Arch + Vec2( doubleDisplacement, 0.f );

	m_endPos[0] = m_south1Arch - Vec2( 0.f, displacement );
	m_endPos[1] = m_south2Arch - Vec2( 0.f, displacement );
	m_endPos[2] = m_east1Arch + Vec2( doubleDisplacement, 0.f );;
	m_endPos[3] = m_east2Arch + Vec2( doubleDisplacement, 0.f );;
	m_endPos[4] = m_north2Arch + Vec2( 0.f, displacement );
	m_endPos[5] = m_north1Arch + Vec2( 0.f, displacement );
	m_endPos[6] = m_west2Arch - Vec2( doubleDisplacement, 0.f );;
	m_endPos[7] = m_west1Arch - Vec2( doubleDisplacement, 0.f );;

	m_curPath = GetRandGen()->RollRandomIntInRange( 0, 7 );
	if (m_curPath == 0 || m_curPath == 1 || m_curPath == 4 || m_curPath == 5) {
		m_pathTimer = new Timer( m_timePassPath, m_clock );
	}
	else {
		m_pathTimer = new Timer( m_timePassPath * 2.f, m_clock );
	}
	m_pathTimer->Start();
}

DiamondBossSuperMiner::~DiamondBossSuperMiner()
{
	delete m_stateTimer;
	delete m_pathTimer;
	delete m_shootTimer;
	delete m_allyTimer;
	delete m_enhanceTimer;
}

void DiamondBossSuperMiner::BeginPlay()
{

}

void DiamondBossSuperMiner::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	
	if (m_stateTimer->HasPeriodElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 5.f );
			m_stateTimer->Start();
			for (int i = 0; i < 8; i++) {
				m_callAlly[i] = 0;
			}
			m_allyTimer->Start();
			m_allyTimer->SetElapsedTime( 0.2f );
		}
		else if (m_state == 1) {
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( 24.f );
			m_stateTimer->Start();
		}
	}

	if (!m_enhanced && m_health <= m_maxHealth * 0.5f) {
		m_enhanceTimer->Start();
		m_enhanced = true;
		m_pathTimerModifier = 0.5f;
		m_shootTimer->SetPeriodSeconds( 0.4f );
		m_state = 0;
		m_stateTimer->SetPeriodSeconds( 24.f );
		m_stateTimer->Start();
	}

	if (m_enhanceTimer->HasPeriodElapsed()) {
		m_pathTimerModifier = 2.f;
		//m_shootTimer->SetPeriodSeconds( 0.4f );
	}

	if (m_pathTimer->HasPeriodElapsed()) {
		if (m_state == 0) {
			m_curPath = GetRandGen()->RollRandomIntInRange( 0, 7 );
			if (m_curPath == 0 || m_curPath == 1 || m_curPath == 4 || m_curPath == 5) {
				m_pathTimer->SetPeriodSeconds( m_timePassPath * m_pathTimerModifier );
			}
			else {
				m_pathTimer->SetPeriodSeconds( m_timePassPath * 2.f * m_pathTimerModifier );
			}
			m_pathTimer->Start();
		}
	}
	else {
		m_position = Interpolate( m_startPos[m_curPath], m_endPos[m_curPath], SmoothStart2( m_pathTimer->GetElapsedFraction() ) );
	}
	if (m_state == 0) {
		if (m_shootTimer->HasPeriodElapsed() && m_pathTimer->GetElapsedFraction() > 0.25f && m_pathTimer->GetElapsedFraction() < 0.75f) {
			Vec2 position = GetRandomPointInDisc2D( 30.f, m_target->m_position );
			Missle* missile = (Missle*)g_theGame->SpawnEffectToGame( EffectType::Missile, m_position );
			missile->m_startPos = m_position;
			missile->m_targetPos = position;
			missile->m_owner = this;
			missile->m_flyTime = 1.3f;
			missile->BeginPlay();
			m_shootTimer->Start();
		}
	}
	else if (m_state == 1) {
		if (m_allyTimer->DecrementPeriodIfElapsed()) {
			for (int i = 0; i < 8; i++) {
				if (m_callAlly[i] == 0) {
					float rnd = GetRandGen()->RollRandomFloatZeroToOne();
					if (rnd < 0.05f) {
						Entity* e = g_theGame->SpawnEntityToGame( m_minerDef, m_startPos[i], (m_endPos[i] - m_startPos[i]).GetOrientationDegrees() );
						e->m_hasReward = false;
						m_callAlly[i] = 1;
					}
				}
			}
		}
	}
}

void DiamondBossSuperMiner::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> envVerts;

	Starship_AddVertsForArch( envVerts, m_north1Arch, m_north1Arch + Vec2( 0.f, m_archHeight ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_north2Arch, m_north2Arch + Vec2( 0.f, m_archHeight ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_south1Arch, m_south1Arch + Vec2( 0.f, -m_archHeight ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_south2Arch, m_south2Arch + Vec2( 0.f, -m_archHeight ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_east1Arch, m_east1Arch + Vec2( m_archHeight, 0.f ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_east2Arch, m_east2Arch + Vec2( m_archHeight, 0.f ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_west1Arch, m_west1Arch + Vec2( -m_archHeight, 0.f ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );
	Starship_AddVertsForArch( envVerts, m_west2Arch, m_west2Arch + Vec2( -m_archHeight, 0.f ), m_archRadius, Rgba8( 204, 102, 0, 200 ) );

	AddVertsForAABB2D( envVerts, AABB2( Vec2( m_south1Arch.x - 0.7f * m_archRadius, m_south1Arch.y ), Vec2( m_north1Arch.x + 0.7f * m_archRadius, m_north1Arch.y ) ), Rgba8( 255, 255, 255, 150 ) );
	AddVertsForAABB2D( envVerts, AABB2( Vec2( m_south2Arch.x - 0.7f * m_archRadius, m_south2Arch.y ), Vec2( m_north2Arch.x + 0.7f * m_archRadius, m_north2Arch.y ) ), Rgba8( 255, 255, 255, 150 ) );
	AddVertsForAABB2D( envVerts, AABB2( Vec2( m_west1Arch.x, m_west1Arch.y - 0.7f * m_archRadius ), Vec2( m_east1Arch.x, m_east1Arch.y + 0.7f * m_archRadius ) ), Rgba8( 255, 255, 255, 150 ) );
	AddVertsForAABB2D( envVerts, AABB2( Vec2( m_west2Arch.x, m_west2Arch.y - 0.7f * m_archRadius ), Vec2( m_east2Arch.x, m_east2Arch.y + 0.7f * m_archRadius ) ), Rgba8( 255, 255, 255, 150 ) );

	AddVertsForAABB2D( verts, AABB2( Vec2( -m_cosmeticRadius, -m_cosmeticRadius ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 255, 178, 102 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::WHITE );
	g_theRenderer->DrawVertexArray( envVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondBossSuperMiner::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void DiamondBossSuperMiner::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = 1050.f;
	float endX = 1550.f;
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Super Miner" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

DiamondBossRayChannel::DiamondBossRayChannel( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_reflectorDef(EntityDefinition::GetDefinition("DiamondReflector"))
	,m_projDef(ProjectileDefinition::GetDefinition("Shuriken"))
{
	m_target = g_theGame->GetPlayerEntity();
	m_stateTimer = new Timer( 8.f, m_clock );
	m_stateTimer->Start();
	m_selectTimer = new Timer( 0.7f, m_clock );
	m_shootTimer = new Timer( 0.1f, m_clock );
	m_dodgeTimer = new Timer( 1.f, m_clock );
	m_state = 0;
	m_orientationDegrees = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );

	m_rayPos[0] = Vec2( 5.f, 5.f );
	m_rayPos[1] = Vec2( 70.f, 5.f );
	m_rayPos[2] = Vec2( 130.f, 5.f );
	m_rayPos[3] = Vec2( 195.f, 5.f );
	m_rayPos[4] = Vec2( 5.f, 95.f );
	m_rayPos[5] = Vec2( 70.f, 95.f );
	m_rayPos[6] = Vec2( 130.f, 95.f );
	m_rayPos[7] = Vec2( 195.f, 95.f );

	for (int i = 0; i < 8; i++) {
		m_rayPos[i] += g_theGame->m_curRoom->m_bounds.m_mins;
	}
}

DiamondBossRayChannel::~DiamondBossRayChannel()
{
	delete m_stateTimer;
	delete m_selectTimer;
	delete m_shootTimer;
	delete m_dodgeTimer;
}

void DiamondBossRayChannel::BeginPlay()
{

}

void DiamondBossRayChannel::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_stateTimer->HasPeriodElapsed()) {
		if (m_state == 0) {
			// exit state 0
			m_ccw = !m_ccw;
			for (int i = 0; i < 6; i++) {
				m_rays[i] = nullptr;
			}
			// prepare state 1
			m_stateTimer->SetPeriodSeconds( 10.f );
			m_stateTimer->Start();
			m_selectTimer->Start();
			m_state = 1;
			m_numSelected = 0;
			ReArrangeRayPos();
		}
		else if (m_state == 1) {
			if (m_isSmall) {
				// prepare state 2
				m_state = 2;
				m_stateTimer->SetPeriodSeconds( 3.f );
				m_shootTimer->Start();
			}
			else {
				// prepare state 0
				m_state = 0;
				m_stateTimer->SetPeriodSeconds( 8.f );
			}
			m_stateTimer->Start();
		}
		else if (m_state == 2) {
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( 5.f );
			m_stateTimer->Start();
			AddImpulse( GetForwardNormal() * 100.f );
			m_shootTimer->Stop();
		}
	}

	if (!m_isSmall && m_health <= m_maxHealth * 0.35f) {
		m_isSmall = true;
		m_physicsRadius *= 0.4f;
		m_cosmeticRadius *= 0.4f;
		m_dodgeTimer->Start();
		ParticleSystem2DAddEmitter( 1000, 0.1f, AABB2( m_position - 0.5f * Vec2( m_cosmeticRadius, m_cosmeticRadius ), m_position + 0.5f * Vec2( m_cosmeticRadius, m_cosmeticRadius ) ),
			FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius ),
			AABB2( Vec2( -40.f, -40.f ), Vec2( 40.f, 40.f ) ),
			FloatRange( 0.6f, 0.9f ), Rgba8( 192, 192, 192 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
			FloatRange( 90.f, 120.f ), nullptr,
			Rgba8( 192, 192, 192, 100 ) );

		for (int i = 0; i < 3; i++) {
			Vec2 position = m_position + GetRandomPointOnUnitCircle2D() * 15.f;
			Entity* e = g_theGame->SpawnEntityToGame( m_reflectorDef, position, (m_target->m_position - position).GetOrientationDegrees() );
			e->m_hasReward = false;
		}
	}

	Vec2 forwardNormal = GetForwardNormal();

	if (m_state == 0) {
		if (m_stateTimer->GetElapsedFraction() > 0.2f) {
			if (m_rays[0] == nullptr) {
				if (m_isSmall) {
					m_rays[0] = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, m_position, m_orientationDegrees );
					m_rays[0]->m_maxLength = 300.f;
					m_rays[0]->m_maxWidth = 1.6f;
					m_rays[0]->m_minWidth = 1.0f;
					m_rays[0]->m_owner = this;
					m_rays[0]->m_lifeTimeSeconds = 3.f;
					m_rays[0]->m_color = Rgba8( 255, 128, 0 );
					m_rays[0]->m_damage = 1.f;
					m_rays[0]->m_damageCoolDown = 2.f;
					m_rays[0]->BeginPlay();
				}
				else {
					for (int i = 0; i < 6; i++) {
						m_rays[i] = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, m_position, m_orientationDegrees + 60.f * i );
						m_rays[i]->m_maxLength = 300.f;
						m_rays[i]->m_maxWidth = 2.7f;
						m_rays[i]->m_minWidth = 1.8f;
						m_rays[i]->m_owner = this;
						m_rays[i]->m_lifeTimeSeconds = 6.f;
						m_rays[i]->m_color = Rgba8( 255, 128, 0 );
						m_rays[i]->m_damage = 1.f;
						m_rays[i]->m_damageCoolDown = 2.f;
						m_rays[i]->BeginPlay();
					}
				}
			}
			if (!m_isSmall) {
				float orientationChange = ((m_ccw ? 1 : -1) * m_def.m_turnSpeed * deltaTime);
				m_orientationDegrees += orientationChange;
				for (int i = 0; i < 6; i++) {
					m_rays[i]->m_orientationDegrees += orientationChange;
				}
			}
			else {
				m_rays[0]->m_position = m_position;
				m_rays[0]->m_orientationDegrees = m_orientationDegrees;
			}
		}
	}
	else if (m_state == 1) {
		if (m_numSelected < 8 && m_selectTimer->DecrementPeriodIfElapsed()) {
			m_numSelected++;
		}
		else if (m_numSelected == 8 && m_selectTimer->DecrementPeriodIfElapsed()) {
			for (int i = 0; i < 7; i++) {
				float length = (m_rayPos[i + 1] - m_rayPos[i]).GetLength();
				PersistentRay* ray = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, m_rayPos[i], (m_rayPos[i + 1] - m_rayPos[i]).GetOrientationDegrees() );
				ray->m_maxLength = length;
				ray->m_maxWidth = 9.f;
				ray->m_minWidth = 5.f;
				ray->m_owner = this;
				ray->m_lifeTimeSeconds = 3.f;
				ray->m_color = Rgba8( 255, 128, 0 );
				ray->m_damage = 1.f;
				ray->m_damageCoolDown = 2.f;
				ray->BeginPlay();
			}
			m_numSelected++;
		}
	}
	else if (m_state == 2) {
		AddForce( forwardNormal* m_def.m_flySpeed * 3.f );
		if (m_shootTimer->DecrementPeriodIfElapsed()) {
			float orientation = m_orientationDegrees + GetRandGen()->RollRandomFloatInRange( -60.f, 60.f );
			Projectile* projectile = g_theGame->SpawnProjectileToGame( m_projDef, m_position, orientation, Vec2::MakeFromPolarDegrees( orientation ) * m_projDef.m_speed );
			projectile->m_faction = m_def.m_faction;
		}
	}

	// surrounding player
	if (m_isSmall) {
		if (m_dodgeTimer->HasPeriodElapsed()) {
			float dist = 400.f;
			Projectile* projectileToDodge = nullptr;
			for (auto proj : g_theGame->m_projectileArray) {
				if (proj && proj->m_faction != m_def.m_faction && GetDistanceSquared2D( m_position, proj->m_position ) < dist
					&& DotProduct2D( proj->m_velocity, forwardNormal ) < 0.f) {
					dist = GetDistanceSquared2D( m_position, proj->m_position );
					projectileToDodge = proj;
				}
			}
			if (projectileToDodge) {
				float cross = CrossProduct2D( forwardNormal, (projectileToDodge->m_position - m_position) );
				if (cross > 0) {
					AddImpulse( forwardNormal.GetRotatedMinus90Degrees() * 30.f );
				}
				else {
					AddImpulse( forwardNormal.GetRotated90Degrees() * 30.f );
				}
				m_dodgeTimer->Start();
			}
		}
		float m_targetOrientationDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		m_orientationToTarget += (30.f * deltaTime);
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, m_targetOrientationDegrees, m_def.m_turnSpeed * deltaTime );
		Vec2 m_targetPos = m_target->m_position + Vec2::MakeFromPolarDegrees( m_orientationToTarget, m_target->m_physicsRadius * 50.f );
		AddForce( (m_targetPos - m_position).GetNormalized() * m_def.m_flySpeed * 2.f );
	}
}

void DiamondBossRayChannel::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> envVerts;
	verts.reserve( 1000 );
	envVerts.reserve( 500 );
	if (!m_isSmall) {
		AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 192, 192, 192 ) );
	}
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 204, 102, 0 ) );

	if (m_state == 0) {
		if (m_isSmall) {
			AddVertsForDisc2D( verts, Vec2::MakeFromPolarDegrees( 0.f, m_cosmeticRadius ), 1.f, Rgba8( 255, 0, 0 ) );
		}
		else {
			for (int i = 0; i < 6; i++) {
				AddVertsForDisc2D( verts, Vec2::MakeFromPolarDegrees( 60.f * i, m_cosmeticRadius ), 1.f, Rgba8( 255, 0, 0 ) );
			}
		}
	}

	if (m_state == 1) {
		for (int i = 0; i < m_numSelected; i++) {
			AddVertsForDisc2D( envVerts, m_rayPos[i], 5.f, Rgba8( 153, 76, 0 ) );
		}
	}


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray( envVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondBossRayChannel::Die()
{
	DiamondFactionSpawnDeathEffect( this );
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
}

void DiamondBossRayChannel::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = 1050.f;
	float endX = 1550.f;
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Ray Channel" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void DiamondBossRayChannel::ReArrangeRayPos()
{
	bool chosen[8] = { false };
	Vec2 newArray[8];
	for (int i = 0; i < 8; i++) {
		int index = 0;
		do {
			index = GetRandGen()->RollRandomIntInRange( 0, 7 );
		} while (chosen[index]);
		chosen[index] = true;
		newArray[i] = m_rayPos[index];
	}

	for (int i = 0; i < 8; i++) {
		m_rayPos[i] = newArray[i];
	}
}
