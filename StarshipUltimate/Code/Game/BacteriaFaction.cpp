#include "Game/BacteriaFaction.hpp"
#include "Game/Game.hpp"
#include "Game/Playership.hpp"
#include "Game/Room.hpp"
#include "Game/Effects.hpp"

SmallBacteria::SmallBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
	, m_deathEntity( &EntityDefinition::GetDefinition( "TinyBacteria" ))
{
	m_flagellaTimer = new Timer( 0.5f, m_clock );
	m_flagellaTimer->Start();
	m_flagellaPoints.reserve( 7 );
	m_flagellaPoints.emplace_back( -0.3f * m_cosmeticRadius, 0.f );
	m_flagellaPoints.emplace_back( -0.55f * m_cosmeticRadius, 0.1f * m_cosmeticRadius );
	m_flagellaPoints.emplace_back( -0.8f * m_cosmeticRadius, 0.f );
	m_flagellaPoints.emplace_back( -1.05f * m_cosmeticRadius, -0.1f * m_cosmeticRadius );
	m_flagellaPoints.emplace_back( -1.3f * m_cosmeticRadius, 0.f );
	m_flagellaPoints.emplace_back( -1.55f * m_cosmeticRadius, 0.1f * m_cosmeticRadius );
	m_flagellaPoints.emplace_back( -1.8f * m_cosmeticRadius, 0.f );
	m_flagella.ResetAllPoints( m_flagellaPoints );
	m_target = g_theGame->GetPlayerObject();
	m_color = Rgba8( 51, 255, 51 );
}

SmallBacteria::~SmallBacteria()
{
	delete m_flagellaTimer;
}

void SmallBacteria::BeginPlay()
{

}

void SmallBacteria::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_flagellaTimer->DecrementPeriodIfElapsed()) {
		m_flagellaState0 = !m_flagellaState0;
	}

	if (m_flagellaState0) {
		float friction = SmoothStep3( m_flagellaTimer->GetElapsedFraction() );
		m_flagellaPoints[1].y = Interpolate( 0.1f * m_cosmeticRadius, -0.1f * m_cosmeticRadius, friction );
		m_flagellaPoints[3].y = Interpolate( -0.1f * m_cosmeticRadius, 0.1f * m_cosmeticRadius, friction );
		m_flagellaPoints[5].y = Interpolate( 0.1f * m_cosmeticRadius, -0.1f * m_cosmeticRadius, friction );
	}
	else {
		float friction = SmoothStep3( m_flagellaTimer->GetElapsedFraction() );
		m_flagellaPoints[1].y = Interpolate( -0.1f * m_cosmeticRadius, 0.1f * m_cosmeticRadius, friction );
		m_flagellaPoints[3].y = Interpolate( 0.1f * m_cosmeticRadius, -0.1f * m_cosmeticRadius, friction );
		m_flagellaPoints[5].y = Interpolate( -0.1f * m_cosmeticRadius, 0.1f * m_cosmeticRadius, friction );
	}
	m_flagella.ResetAllPoints( m_flagellaPoints );

	Vec2 targetVec = m_target->m_position - m_position;
	float targetOrientation = targetVec.GetOrientationDegrees();
	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientation, m_def.m_turnSpeed * deltaTime );
	if (abs(GetShortestAngularDispDegrees( m_orientationDegrees, targetOrientation )) < 10.f) {
		AddForce( GetForwardNormal() * m_def.m_flySpeed );
	}
}

void SmallBacteria::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForCapsule2D( verts, Vec2( -0.3f * m_cosmeticRadius, 0.f ), Vec2( m_cosmeticRadius, 0.f ), m_cosmeticRadius * 0.4f, Rgba8::WHITE );
	m_flagella.AddVertsForCurve2D( verts, 0.3f, Rgba8::WHITE, 16 );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void SmallBacteria::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 2; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_deathEntity;
		drop->BeginPlay();
	}
	SpawnDeathEffect();
}

void SmallBacteria::SpawnDeathEffect() const
{
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

TinyBacteria::TinyBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:SmallBacteria(def, startPos, startOrientation, startVelocity)
{
	m_color = Rgba8( 0, 153, 0 );
}

TinyBacteria::~TinyBacteria()
{

}

void TinyBacteria::BeginPlay()
{
	SmallBacteria::BeginPlay();
}

void TinyBacteria::Update( float deltaTime )
{
	SmallBacteria::Update( deltaTime );
}

void TinyBacteria::Render() const
{
	SmallBacteria::Render();
}

void TinyBacteria::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	SpawnDeathEffect();
}

MediumBacteria::MediumBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:SmallBacteria(def, startPos, startOrientation, startVelocity)
{
	m_deathEntity = &EntityDefinition::GetDefinition( "SmallBacteria" );
	m_color = Rgba8( 178, 255, 102 );
}

MediumBacteria::~MediumBacteria()
{

}

void MediumBacteria::BeginPlay()
{
	SmallBacteria::BeginPlay();
}

void MediumBacteria::Update( float deltaTime )
{
	SmallBacteria::Update( deltaTime );
}

void MediumBacteria::Render() const
{
	SmallBacteria::Render();
}

void MediumBacteria::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 2; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_deathEntity;
		drop->BeginPlay();
	}
	SpawnDeathEffect();
}

LargeBacteria::LargeBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:SmallBacteria(def, startPos, startOrientation, startVelocity)
{
	m_deathEntity = &EntityDefinition::GetDefinition( "MediumBacteria" );
	m_color = Rgba8( 204, 255, 153 );
}

LargeBacteria::~LargeBacteria()
{

}

void LargeBacteria::BeginPlay()
{
	SmallBacteria::BeginPlay();
}

void LargeBacteria::Update( float deltaTime )
{
	SmallBacteria::Update( deltaTime );
}

void LargeBacteria::Render() const
{
	SmallBacteria::Render();
}

void LargeBacteria::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 2; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_deathEntity;
		drop->BeginPlay();
	}
	SpawnDeathEffect();
}

BacteriaMothership::BacteriaMothership( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
	,m_smallBacteria(&EntityDefinition::GetDefinition("SmallBacteria"))
{
	m_spawnTimer = new Timer( 8.f, m_clock );
	m_spawnTimer->Start();
	m_deathEntity = &EntityDefinition::GetDefinition( "TinyBacteria" );
	m_target = g_theGame->GetPlayerObject();
	ResetWonderingTarget();
	m_switchWonderingPosTimer = new Timer( 6.f, m_clock );
	m_switchWonderingPosTimer->Start();
}

BacteriaMothership::~BacteriaMothership()
{
	delete m_spawnTimer;
	delete m_switchWonderingPosTimer;
}

void BacteriaMothership::BeginPlay()
{

}

void BacteriaMothership::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_spawnTimer->DecrementPeriodIfElapsed()) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 3.f ) + m_position;
		Entity* e = g_theGame->SpawnEntityToGame( *m_smallBacteria, targetPos, angle );
		e->m_hasReward = false;
		ParticleSystem2DAddEmitter( 200, 0.05f, AABB2( targetPos, targetPos ),
			FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 1.2f ),
			AABB2( Vec2( -15.f, -15.f ), Vec2( 15.f, 15.f ) ),
			FloatRange( 0.2f, 0.4f ), Rgba8( 102, 255, 102 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
			FloatRange( 40.f, 75.f ), nullptr,
			Rgba8( 102, 255, 102, 0 ) );
	}

	if (m_switchWonderingPosTimer->DecrementPeriodIfElapsed()) {
		ResetWonderingTarget();
	}

	if (IsPointInsideDisc2D( m_wonderingPos, m_position, m_physicsRadius )) {
		ResetWonderingTarget();
	}
	float targetDegrees = (m_wonderingPos - m_position).GetOrientationDegrees();
	SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
	if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 10.f) {
		AddForce( GetForwardNormal() * m_def.m_flySpeed );
	}
}

void BacteriaMothership::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 500 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 0, 204, 0 ) );
	AddVertsForDisc2D( verts, Vec2( m_cosmeticRadius * 0.75f, m_cosmeticRadius * 0.75f ), m_cosmeticRadius * 0.4f, Rgba8( 204, 255, 153 ) );
	AddVertsForDisc2D( verts, Vec2( -m_cosmeticRadius * 0.75f, -m_cosmeticRadius * 0.75f ), m_cosmeticRadius * 0.4f, Rgba8( 204, 255, 153 ) );
	AddVertsForDisc2D( verts, Vec2( m_cosmeticRadius * 0.75f, -m_cosmeticRadius * 0.75f ), m_cosmeticRadius * 0.4f, Rgba8( 204, 255, 153 ) );
	AddVertsForDisc2D( verts, Vec2( -m_cosmeticRadius * 0.75f, m_cosmeticRadius * 0.75f ), m_cosmeticRadius * 0.4f, Rgba8( 204, 255, 153 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaMothership::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 6; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_deathEntity;
		drop->BeginPlay();
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

void BacteriaMothership::ResetWonderingTarget()
{
	Vec2 wonderingVec, targetVec;
	AABB2 box = AABB2( g_theGame->m_curRoom->m_bounds.m_mins + Vec2( 10.f, 10.f ), g_theGame->m_curRoom->m_bounds.m_maxs - Vec2( 10.f, 10.f ) );
	do {
		m_wonderingPos = box.GetRandomPointInside();
		wonderingVec = m_wonderingPos - m_position;
		targetVec = m_target->m_position - m_position;
	} while (GetDistanceSquared2D( m_wonderingPos, m_target->m_position ) < 3600.f && DotProduct2D( wonderingVec, targetVec ) > 0.f);
}

BacteriaSpawn::BacteriaSpawn( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_smallBacteria( &EntityDefinition::GetDefinition( "SmallBacteria" ) )
	,m_deathEntity(& EntityDefinition::GetDefinition("TinyBacteria"))
{
	m_spawnTimer = new Timer( 4.f, m_clock );
	m_spawnTimer->Start();
	m_color = Rgba8( 224, 255, 224 );
	m_hasReward = false;
}

BacteriaSpawn::~BacteriaSpawn()
{
	delete m_spawnTimer;
}

void BacteriaSpawn::BeginPlay()
{

}

void BacteriaSpawn::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_isDead) {
		return;
	}
	if (m_spawnTimer->HasPeriodElapsed()) {
		Entity* e = g_theGame->SpawnEntityToGame( *m_smallBacteria, m_position, m_orientationDegrees );
		e->m_hasReward = false;
		m_isDead = true;
		ParticleSystem2DAddEmitter( 200, 0.05f, AABB2( m_position, m_position ),
			FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 1.2f ),
			AABB2( Vec2( -15.f, -15.f ), Vec2( 15.f, 15.f ) ),
			FloatRange( 0.2f, 0.4f ), Rgba8( 224, 255, 224 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
			FloatRange( 40.f, 75.f ), nullptr,
			Rgba8( 224, 255, 224, 0 ) );
	}
}

void BacteriaSpawn::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForCapsule2D( verts, Vec2( -0.4f * m_cosmeticRadius, 0.f ), Vec2( 0.4f * m_cosmeticRadius, 0.f ), m_cosmeticRadius * 0.4f, Rgba8::WHITE );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaSpawn::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 1; i++) {
		Entity* e = g_theGame->SpawnEntityToGame( *m_deathEntity, m_position, m_orientationDegrees );
		e->m_hasReward = false;
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

BacteriaBreeder::BacteriaBreeder( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:BacteriaMothership(def, startPos, startOrientation, startVelocity)
{
	m_deathEntity = &EntityDefinition::GetDefinition( "BacteriaSpawn" );
	m_spawnTimer->SetPeriodSeconds( 6.f );
}

BacteriaBreeder::~BacteriaBreeder()
{
}

void BacteriaBreeder::BeginPlay()
{
	BacteriaMothership::BeginPlay();
}

void BacteriaBreeder::Update( float deltaTime )
{
	Entity::Update( deltaTime );
	if (m_spawnTimer->DecrementPeriodIfElapsed()) {
		float angle; Vec2 targetPos;
		do {
			angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
			targetPos = Vec2::MakeFromPolarDegrees( angle, 30.f ) + m_position;
		} while (!g_theGame->m_curRoom->m_bounds.IsPointInside( targetPos ));
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 1.2f;
		drop->m_entityToSpawn = m_deathEntity;
		drop->BeginPlay();
	}

	if (m_switchWonderingPosTimer->DecrementPeriodIfElapsed()) {
		ResetWonderingTarget();
	}

	if (IsPointInsideDisc2D( m_wonderingPos, m_position, m_physicsRadius )) {
		ResetWonderingTarget();
	}
	float targetDegrees = (m_wonderingPos - m_position).GetOrientationDegrees();
	SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
	if (Absf( GetShortestAngularDispDegrees( m_orientationDegrees, targetDegrees ) ) < 10.f) {
		AddForce( GetForwardNormal() * m_def.m_flySpeed );
	}
}

void BacteriaBreeder::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 102, 204, 0 ) );
	AddVertsForCapsule2D( verts, Vec2( -1.3f * m_cosmeticRadius, 0.f ), Vec2( -0.5f * m_cosmeticRadius, 0.f ), m_cosmeticRadius * 0.2f, Rgba8( 224, 255, 224 ) );
	AddVertsForCapsule2D( verts, Vec2( -1.1f * m_cosmeticRadius, m_cosmeticRadius * 0.6f ), Vec2( -0.5f * m_cosmeticRadius, m_cosmeticRadius * 0.1f ), m_cosmeticRadius * 0.2f, Rgba8( 224, 255, 224 ) );
	AddVertsForCapsule2D( verts, Vec2( -1.1f * m_cosmeticRadius, -m_cosmeticRadius * 0.6f ), Vec2( -0.5f * m_cosmeticRadius, -m_cosmeticRadius * 0.1f ), m_cosmeticRadius * 0.2f, Rgba8( 224, 255, 224 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaBreeder::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 3; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_deathEntity;
		drop->BeginPlay();
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

BacteriaSprayer::BacteriaSprayer( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
	,m_tinyBacteria(&EntityDefinition::GetDefinition("TinyBacteria"))
	,m_basicCosmeticRadius(m_def.m_cosmeticRadius)
	,m_basicPhysicsRadius(m_def.m_physicsRadius)
{
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer->Start();
	m_mainWeaponTimer->Start();
	m_target = g_theGame->GetPlayerObject();
	m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
	float targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -30.f, 30.f );
	m_targetOrientation += targetRelativeOrientation;
}

BacteriaSprayer::~BacteriaSprayer()
{
	delete m_stateTimer;
}

void BacteriaSprayer::BeginPlay()
{

}

void BacteriaSprayer::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 1.2f );
			m_cosmeticRadius = m_basicCosmeticRadius;
			m_physicsRadius = m_basicPhysicsRadius;
			//if (m_mainWeaponTimer->DecrementPeriodIfElapsed()) {
			m_mainWeapon->Fire( GetForwardNormal(), m_position + GetForwardNormal() * m_cosmeticRadius );
			//}
		}
		else if (m_state == 1) {
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( m_def.m_shootCoolDown );
		}
	}

	// go to the player
	if (m_state == 0) {
		Vec2 forwardVector = GetForwardNormal();
		Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, 20.f );
		Vec2 displacement = targetPosition - m_position;
		if (GetDistanceSquared2D( m_position, targetPosition ) >= 25.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
		}
		else {
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
		}
		m_physicsRadius = Interpolate( m_basicPhysicsRadius, m_basicPhysicsRadius * 1.3f, SmoothStop2( m_stateTimer->GetElapsedFraction() ) );
		m_cosmeticRadius = Interpolate( m_basicCosmeticRadius, m_basicCosmeticRadius * 1.3f, SmoothStop2( m_stateTimer->GetElapsedFraction() ) );
	}
}

void BacteriaSprayer::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 0, 204, 102 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.9f * m_cosmeticRadius, -0.2f * m_cosmeticRadius ), Vec2( 1.2f * m_cosmeticRadius, 0.2f * m_cosmeticRadius ) ), Rgba8( 76, 153, 0 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaSprayer::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 3; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_tinyBacteria;
		drop->BeginPlay();
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

BacteriaFusion::BacteriaFusion( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_smallBacteria( &EntityDefinition::GetDefinition( "SmallBacteria" ) )
{
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer->Start();
	m_target = g_theGame->GetPlayerObject();
}

BacteriaFusion::~BacteriaFusion()
{
	delete m_stateTimer;
}

void BacteriaFusion::BeginPlay()
{

}

void BacteriaFusion::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 1.2f );
			//if (m_mainWeaponTimer->DecrementPeriodIfElapsed()) {
			m_mainWeapon->Fire( GetForwardNormal(), m_position + GetForwardNormal() * m_cosmeticRadius );
			m_mainWeapon->Fire( -GetForwardNormal(), m_position - GetForwardNormal() * m_cosmeticRadius );
			Vec2 left = GetForwardNormal().GetRotated90Degrees();
			m_mainWeapon->Fire( left, m_position + left * m_cosmeticRadius );
			m_mainWeapon->Fire( -left, m_position - left * m_cosmeticRadius );
			//}
		}
		else if (m_state == 1) {
			m_state = 2;
			m_stateTimer->SetPeriodSeconds( 1.5f );
			AddImpulse( GetForwardNormal() * 200.f );
		}
		else if (m_state == 2) {
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( m_def.m_shootCoolDown );
		}
	}

	// go to the player
	if (m_state == 0) {
		Vec2 forwardVector = GetForwardNormal();
		Vec2 targetPosition = m_target->m_position;
		Vec2 displacement = targetPosition - m_position;
		if (GetDistanceSquared2D( m_position, targetPosition ) >= 4.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
		}
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	}
	else if (m_state == 2) {
		AddForce( m_velocity * 7.6f );
	}
}

void BacteriaFusion::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 0, 204, 102 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.8f * m_cosmeticRadius, -0.3f * m_cosmeticRadius ), Vec2( 1.2f * m_cosmeticRadius, 0.3f * m_cosmeticRadius ) ), Rgba8( 255, 51, 255 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( -1.2f * m_cosmeticRadius, -0.3f * m_cosmeticRadius ), Vec2( -0.8f * m_cosmeticRadius, 0.3f * m_cosmeticRadius ) ), Rgba8( 76, 153, 0 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( -0.3f * m_cosmeticRadius, 0.8f * m_cosmeticRadius ), Vec2( 0.3f * m_cosmeticRadius, 1.2f * m_cosmeticRadius ) ), Rgba8( 255, 153, 153 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( -0.3f * m_cosmeticRadius, -1.2f * m_cosmeticRadius ), Vec2( 0.3f * m_cosmeticRadius, -0.8f * m_cosmeticRadius ) ), Rgba8( 255, 255, 102 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaFusion::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 6; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.6f;
		drop->m_entityToSpawn = m_smallBacteria;
		drop->BeginPlay();
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

constexpr float SMALL_FUSION_RADIUS_FRACTION = 0.8f;

BacteriaBossTheGreatFusion::BacteriaBossTheGreatFusion( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_bacteriaFusion( &EntityDefinition::GetDefinition( "BacteriaFusion" ) )
	, m_bacteriaSprayer( &EntityDefinition::GetDefinition( "BacteriaSprayer" ) )
	, m_basicPhysicsRadius(m_def.m_physicsRadius)
	,m_basicCosmeticRadius(m_def.m_cosmeticRadius)
{
	m_selfClock = new Clock( *m_clock );
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_selfClock );
	m_stateTimer->Start();
	m_spawnSprayerTimer = new Timer( 6.f, m_selfClock );
	m_spawnSprayerTimer->Start();
	m_target = g_theGame->GetPlayerObject();
	m_curStateIndex = GetRandGen()->RollRandomIntInRange( 0, m_stateCount - 1 );
	GoToNextState();
}

BacteriaBossTheGreatFusion::~BacteriaBossTheGreatFusion()
{
	delete m_selfClock;
	delete m_stateTimer;
	delete m_spawnSprayerTimer;
}

void BacteriaBossTheGreatFusion::BeginPlay()
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1000 );
	for (int i = 0; i < 20; i++) {
		AddVertsForDisc2D( verts, GetRandomPointInDisc2D( m_cosmeticRadius * 0.8f ), GetRandGen()->RollRandomFloatInRange( m_cosmeticRadius * 0.5f, m_cosmeticRadius * 0.8f ), Rgba8( (unsigned char)GetRandGen()->RollRandomIntInRange( 30, 180 ), 255, (unsigned char)GetRandGen()->RollRandomIntInRange( 30, 180 ) ) );
	}
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_vertexBuffer );
}

void BacteriaBossTheGreatFusion::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	Entity::Update( deltaTime );

	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}

	if (m_state == 0) {
		Vec2 forwardVector = GetForwardNormal();
		Vec2 displacement = m_target->m_position - m_position;
		if (displacement.GetLengthSquared() >= 25.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
		}
		else {
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, displacement.GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
		}
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, displacement.GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
		if (m_curStateIndex == 0 || m_curStateIndex == 5) {
			if (m_isSmall) {
				m_physicsRadius = Interpolate( m_basicPhysicsRadius * SMALL_FUSION_RADIUS_FRACTION, m_basicPhysicsRadius * 1.3f * SMALL_FUSION_RADIUS_FRACTION, SmoothStop2( m_stateTimer->GetElapsedFraction() ) );
				m_cosmeticRadius = Interpolate( m_basicCosmeticRadius * SMALL_FUSION_RADIUS_FRACTION, m_basicCosmeticRadius * 1.3f * SMALL_FUSION_RADIUS_FRACTION, SmoothStop2( m_stateTimer->GetElapsedFraction() ) );
			}
			else {
				m_physicsRadius = Interpolate( m_basicPhysicsRadius, m_basicPhysicsRadius * 1.3f, SmoothStop2( m_stateTimer->GetElapsedFraction() ) );
				m_cosmeticRadius = Interpolate( m_basicCosmeticRadius, m_basicCosmeticRadius * 1.3f, SmoothStop2( m_stateTimer->GetElapsedFraction() ) );
			}
		}
	}
	else if (m_state == 1) {

	}
	else if (m_state == 2) {

	}
	else if (m_state == 3) {
		AddForce( Vec2( 0.f, -120.f ) );
	}
	else if (m_state == 4) {
		if (GetDistanceSquared2D( m_target->m_position, m_position ) < 900.f) {
			Vec2 disp = m_target->m_position - m_position;
			float length = disp.GetLength();
			Vec2 forwardNormal = disp / length;
			m_target->AddForce( forwardNormal * RangeMapClamped( length, 0.f, 30.f, 100.f, 2.f ) * 10.f );
		}
	}
}

void BacteriaBossTheGreatFusion::Render() const
{
	Mat44 modelMatrix = GetModelMatrix();
	if (m_isSmall) {
		modelMatrix.AppendScaleUniform2D( m_cosmeticRadius / m_basicCosmeticRadius * SMALL_FUSION_RADIUS_FRACTION );
	}
	else {
		modelMatrix.AppendScaleUniform2D( m_cosmeticRadius / m_basicCosmeticRadius );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( modelMatrix, m_color );
	g_theRenderer->DrawVertexBuffer( m_vertexBuffer, (int)(m_vertexBuffer->GetVertexCount()), 0 );
}

void BacteriaBossTheGreatFusion::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	if (m_isSmall) {
		for (int i = 0; i < 2; i++) {
			float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
			Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
			BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
			drop->m_targetPos = targetPos;
			drop->m_travelTime = 0.6f;
			drop->m_entityToSpawn = m_bacteriaFusion;
			drop->BeginPlay();
		}
	}
	else {
		for (int i = 0; i < 2; i++) {
			float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
			Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 1.f ) + m_position;
			BacteriaBossTheGreatFusion* smallBoss = (BacteriaBossTheGreatFusion*)g_theGame->SpawnEntityToGame( m_def, targetPos, angle );
			if (i == 1) {
				smallBoss->m_isLeft = true;
			}
			smallBoss->m_isSmall = true;
			smallBoss->m_maxHealth = smallBoss->m_maxHealth * 0.5f;
			smallBoss->m_health = smallBoss->m_health * 0.5f;
			smallBoss->m_hasReward = false;
			smallBoss->m_selfClock->SetTimeScale( 1.3f );
		}
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

void BacteriaBossTheGreatFusion::GoToNextState()
{
	if (m_state == 3) {
		m_velocity = Vec2();
	}
	m_curStateIndex = (m_curStateIndex + 1) % m_stateCount;
	m_state = m_stateSequence[m_curStateIndex];
	if (m_isSmall) {
		m_cosmeticRadius = m_basicCosmeticRadius * SMALL_FUSION_RADIUS_FRACTION;
		m_physicsRadius = m_basicPhysicsRadius * SMALL_FUSION_RADIUS_FRACTION;
	}
	else {
		m_cosmeticRadius = m_basicCosmeticRadius;
		m_physicsRadius = m_basicPhysicsRadius;
	}
	if (m_state == 0) {
		m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 1) {
		m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
		((Spray*)m_mainWeapon)->m_isSector = false;
		((Spray*)m_mainWeapon)->m_length = 40.f;
		m_mainWeapon->Fire( GetForwardNormal(), m_position + GetForwardNormal() * m_cosmeticRadius );
		m_mainWeapon->Fire( -GetForwardNormal(), m_position - GetForwardNormal() * m_cosmeticRadius );
		Vec2 left = GetForwardNormal().GetRotated90Degrees();
		m_mainWeapon->Fire( left, m_position + left * m_cosmeticRadius );
		m_mainWeapon->Fire( -left, m_position - left * m_cosmeticRadius );
	}
	else if (m_state == 2) {
		m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
		((Spray*)m_mainWeapon)->m_isSector = true;
		((Spray*)m_mainWeapon)->m_length = 30.f;
		((Spray*)m_mainWeapon)->m_sectorRangeDegrees = 360.f;
		m_mainWeapon->Fire( GetForwardNormal(), m_position);
	}
	else if (m_state == 3) {
		m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_jumpTargetPos = m_target->m_position;
		m_disableFriction = true;
		m_isInvincible = true;
		m_restrictIntoRoom = false;
		float vx = (m_jumpTargetPos.x - m_position.x) / m_stateTime[m_state];
		float vy = (m_jumpTargetPos.y - m_position.y) / m_stateTime[m_state] + 60.f * m_stateTime[m_state];
		AddImpulse( Vec2( vx, vy ) );
	}
	else if (m_state == 4) {
		m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
		ParticleSystem2DAddEmitter( 400, m_stateTime[m_state] * 0.8f, AABB2( m_position + Vec2(-m_cosmeticRadius, -m_cosmeticRadius), m_position + Vec2(m_cosmeticRadius, m_cosmeticRadius) ),
			FloatRange( 1.f, 3.f ), AABB2( Vec2( -30.f, -30.f ), Vec2( 30.f, 30.f ) ),
			FloatRange( 1.f, 1.f ), Rgba8( 100, 220, 100 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
			FloatRange( 0.f, 0.f ), nullptr,
			Rgba8( 100, 220, 100, 0 ), 0.f, 0.f );
		if (!m_isSmall) {
			while(m_spawnSprayerTimer->DecrementPeriodIfElapsed()){
				float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
				Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
				BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
				drop->m_targetPos = targetPos;
				drop->m_travelTime = m_stateTime[m_state] * 0.8f;
				drop->m_entityToSpawn = m_bacteriaSprayer;
				drop->BeginPlay();
			}
		}
	}
	m_stateTimer->Start();
}

void BacteriaBossTheGreatFusion::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX, endX;
	if (m_isLeft) {
		startX = 50.f;
		endX = 550.f;
	}
	else {
		startX = 1050.f;
		endX = 1550.f;
	}
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Fusion The Great" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
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

BacteriaBossMarshKing::BacteriaBossMarshKing( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_bacteriaBreeder( &EntityDefinition::GetDefinition( "BacteriaBreeder" ) )
	, m_bacteriaSpawn( &EntityDefinition::GetDefinition( "BacteriaSpawn" ) )
{
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	m_stateTimer->Start();
	m_spawnBreederTimer = new Timer( 2.5f, m_clock );
	m_spawnSpawnTimer = new Timer( 8.f, m_clock );
	m_spawnSpawnTimer->Start();
	m_spawnSpawnTimer->SetElapsedTime( 6.f );
	m_target = g_theGame->GetPlayerObject();
	m_curStateIndex = 4;
	GoToNextState();

	m_verts.reserve( 1000 );
	for (int i = 0; i < 20; i++) {
		AddVertsForDisc2D( m_verts, GetRandomPointInDisc2D( m_cosmeticRadius * 0.8f ), GetRandGen()->RollRandomFloatInRange( m_cosmeticRadius * 0.5f, m_cosmeticRadius * 0.8f ), Rgba8( 153, 153, (unsigned char)GetRandGen()->RollRandomIntInRange( 10, 50 ) ) );
	}
}

BacteriaBossMarshKing::~BacteriaBossMarshKing()
{
	delete m_stateTimer;
	delete m_spawnSpawnTimer;
	delete m_spawnBreederTimer;
}

void BacteriaBossMarshKing::BeginPlay()
{

}

void BacteriaBossMarshKing::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	Entity::Update( deltaTime );

	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}

	if (m_spawnSpawnTimer->DecrementPeriodIfElapsed()) {
		Vec2 spawnPos;
		do {
			spawnPos = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
		} while (GetDistanceSquared2D( spawnPos, m_target->m_position ) < 3600.f);
		BacteriaSpawn* spawn = (BacteriaSpawn*)g_theGame->SpawnEntityToGame( *m_bacteriaSpawn, spawnPos, GetRandGen()->RollRandomFloatInRange( 0.f, 360.f ) );
		spawn->m_spawnTimer->SetPeriodSeconds( 10.f ); 
	}

	if (m_state == 0) {
		Vec2 dir = Starship_GetStrongestPart( m_wonderTargetPos - m_position );
		//m_orientationDegrees = dir.GetOrientationDegrees();
		AddForce( dir * m_def.m_flySpeed );
	}
	else if (m_state == 1) {
		if (m_spawnToGoIndex != -1 && m_allSpawns[m_spawnToGoIndex]->m_isDead) {
			SetSpawnToGo();
		}
		if(m_spawnToGoIndex == -1) {
			m_wonderTargetPos = m_target->m_position;
			Vec2 dir = Starship_GetStrongestPart( m_wonderTargetPos - m_position );
			//m_orientationDegrees = dir.GetOrientationDegrees();
			AddForce( dir * ((PlayerShip*)m_target)->GetMovingSpeed() * 0.96f );
		}
		else {
			Vec2 dir = Starship_GetStrongestPart( m_wonderTargetPos - m_position );
			//m_orientationDegrees = dir.GetOrientationDegrees();
			AddForce( dir * m_def.m_flySpeed );
		}
		if (m_spawnToGoIndex != -1 && GetDistanceSquared2D( m_position, m_allSpawns[m_spawnToGoIndex]->m_position ) < (m_physicsRadius + 3.f) * (m_physicsRadius + 3.f)) {
			m_health += 10.f;
			m_health = GetClamped( m_health, 0.f, m_maxHealth );
			m_allSpawns[m_spawnToGoIndex]->m_isDead = true;
		}
	}
	else if (m_state == 2) {
		if (m_spawnBreederTimer->DecrementPeriodIfElapsed()) {
			Vec2 spawnPos = GetRandomPointInDisc2D( 3.f * m_cosmeticRadius, m_position );
			g_theGame->SpawnEntityToGame( *m_bacteriaBreeder, spawnPos, GetRandGen()->RollRandomFloatInRange( 0.f, 360.f ) );
		}
	}
	else if (m_state == 3) {
		m_wonderTargetPos = m_target->m_position;
		Vec2 dir = Starship_GetStrongestPart( m_wonderTargetPos - m_position );
		//m_orientationDegrees = dir.GetOrientationDegrees();
		AddForce( dir * ((PlayerShip*)m_target)->GetMovingSpeed() * 0.96f );
	}
	else if (m_state == 4) {

	}
}

void BacteriaBossMarshKing::Render() const
{
	if (m_state == 2) {
		return;
	}
	//std::vector<Vertex_PCU> verts;

	//AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 153, 153, 0 ) );
	//AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, -m_cosmeticRadius * 0.6f ), Vec2( m_cosmeticRadius * 1.3f, m_cosmeticRadius * 0.6f ) ), Rgba8( 102, 102, 0 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
}

void BacteriaBossMarshKing::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < 10; i++) {
		float angle = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
		Vec2 targetPos = Vec2::MakeFromPolarDegrees( angle, m_cosmeticRadius * 6.f ) + m_position;
		BacteriaDrop* drop = (BacteriaDrop*)g_theGame->SpawnEffectToGame( EffectType::BacteriaDrop, m_position, angle );
		drop->m_targetPos = targetPos;
		drop->m_travelTime = 0.3f;
		drop->m_entityToSpawn = m_bacteriaSpawn;
		drop->BeginPlay();
	}
	BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
	sap->m_startPos = m_position;
	sap->m_endPos = m_position;
	sap->m_capsuleRadius = m_cosmeticRadius * 2.f;
	sap->m_maxRadius = m_cosmeticRadius * 1.5f;
	sap->m_numOfDisc = 10;
	sap->m_lifeTime = 3.f;
	sap->m_color = Rgba8( 0, 153, 0 );
	sap->BeginPlay();
}

void BacteriaBossMarshKing::GoToNextState()
{
	m_curStateIndex = (m_curStateIndex + 1) % m_stateCount;
	m_state = m_stateSequence[m_curStateIndex];
	m_isInvincible = false;

	m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
	if (m_state == 0) {
		do {
			m_wonderTargetPos = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
		} while (GetDistanceSquared2D( m_wonderTargetPos, m_target->m_position ) < 3600.f);
	}
	else if (m_state == 1) {
		g_theGame->GetAllEntityByDef( m_allSpawns, *m_bacteriaSpawn );
		// no spawn? go to the player
		if ((int)m_allSpawns.size() == 0) {
			m_spawnToGoIndex = -1;
		}
		else {
			SetSpawnToGo();
		}
	}
	else if (m_state == 2) {
		m_spawnBreederTimer->Start();
		m_isInvincible = true;
		BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_position );
		sap->m_startPos = m_position;
		sap->m_endPos = m_position;
		sap->m_capsuleRadius = m_cosmeticRadius * 3.f;
		sap->m_maxRadius = m_cosmeticRadius * 2.f;
		sap->m_numOfDisc = 20;
		sap->m_lifeTime = 6.f;
		sap->m_color = Rgba8( 153, 153, 0 );
		sap->BeginPlay();
	}
	else if (m_state == 3) {

	}
	else if (m_state == 4) {
		m_velocity = Vec2();
		g_theGame->SpawnEffectToGame( EffectType::BacteriaLick, m_position, (m_target->m_position - m_position).GetOrientationDegrees() );
	}
	m_stateTimer->Start();
}

void BacteriaBossMarshKing::RenderUI() const
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
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Marsh King" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
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

void BacteriaBossMarshKing::SetSpawnToGo()
{
	float nearestSpawnDist = FLT_MAX;
	int nearestIndex = -1;
	for (int i = 0; i < (int)m_allSpawns.size(); i++) {
		if (m_allSpawns[i] && m_allSpawns[i]->IsAlive()) {
			if (GetDistanceSquared2D( m_allSpawns[i]->m_position, m_position ) < nearestSpawnDist) {
				nearestIndex = i;
				nearestSpawnDist = GetDistanceSquared2D( m_allSpawns[i]->m_position, m_position );
			}
		}
	}
	m_spawnToGoIndex = nearestIndex;
	if (m_spawnToGoIndex != -1) {
		m_wonderTargetPos = m_allSpawns[m_spawnToGoIndex]->m_position;
	}
	else {
		m_wonderTargetPos = m_target->m_position;
	}
}

constexpr float g_sistersRespawnTime = 5.f;

BacteriaBossThreeSisters::BacteriaBossThreeSisters( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_stateTimer = new Timer( 2.f, m_clock );
	m_stateTimer->Start();
	m_sisters.reserve( 3 );
	EntityDefinition const& sisterDef = EntityDefinition::GetDefinition( "BacteriaBossSister" );
	m_player = g_theGame->GetPlayerObject();
	for (int i = 0; i < 3; i++) {
		Vec2 spawnPos = g_theGame->m_curRoom->m_bounds.m_mins + Vec2( 50.f + i * 50.f, 50.f );
		float spawnOrientation = (m_player->m_position - spawnPos).GetOrientationDegrees();
		BacteriaBossSister* sister = (BacteriaBossSister*)g_theGame->SpawnEntityToGame( sisterDef, spawnPos, spawnOrientation );
		sister->m_standPos[0] = Vec2( 50.f + i * 50.f, 20.f ) + g_theGame->m_curRoom->m_bounds.m_mins;
		sister->m_standPos[1] = Vec2( 50.f + i * 50.f, 50.f ) + g_theGame->m_curRoom->m_bounds.m_mins;
		sister->m_standPos[2] = Vec2( 50.f + i * 50.f, 80.f ) + g_theGame->m_curRoom->m_bounds.m_mins;
		sister->m_index = i;
		sister->StartUp();
		m_sisters.push_back( sister );
	}
	m_isInvincible = true;
	m_disableFriction = true;
	m_restrictIntoRoom = false;
	m_curStateIndex = -1;
	GoToNextState();
}

BacteriaBossThreeSisters::~BacteriaBossThreeSisters()
{
	delete m_stateTimer;
}

void BacteriaBossThreeSisters::BeginPlay()
{

}

void BacteriaBossThreeSisters::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	Entity::Update( deltaTime );
	bool allDied = true;
	bool hasOneDied = false;
	for (int i = 0; i < (int)m_sisters.size(); i++) {
		if (m_sisters[i]->IsAlive()) {
			allDied = false;
		}
		else {
			hasOneDied = true;
		}
	}
	if (allDied) {
		Die();
		return;
	}

	if (hasOneDied && m_state != 3) {
		// go to state 3
		m_stateTimer->SetPeriodSeconds( m_stateTime[3] );
		m_state = 3;
		for (int i = 0; i < (int)m_sisters.size(); i++) {
			m_sisters[i]->GoToState( 3 );
		}
	}
	
	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}
}

void BacteriaBossThreeSisters::Render() const
{
	if (m_state == 3) {
		std::vector<Vertex_PCU> environmentVerts;

		AddVertsForLineSegment2D( environmentVerts, m_sisters[0]->m_position, m_sisters[1]->m_position, 2.f, Rgba8( 60, 60, 60, 100 ) );
		AddVertsForLineSegment2D( environmentVerts, m_sisters[1]->m_position, m_sisters[2]->m_position, 2.f, Rgba8( 60, 60, 60, 100 ) );
		AddVertsForLineSegment2D( environmentVerts, m_sisters[2]->m_position, m_sisters[0]->m_position, 2.f, Rgba8( 60, 60, 60, 100 ) );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( environmentVerts );
	}
}

void BacteriaBossThreeSisters::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	for (int i = 0; i < (int)m_sisters.size(); i++) {
		m_sisters[i]->m_defeated = true;
	}
}

void BacteriaBossThreeSisters::GoToNextState()
{
	if (m_state == 3) {
		// recover all dead sister
		for (int i = 0; i < (int)m_sisters.size(); i++) {
			if (m_sisters[i]->m_isDead) {
				m_sisters[i]->m_isDead = false;
			}
		}
	}

	m_curStateIndex = (m_curStateIndex + 1) % m_stateCount;
	m_state = m_stateSequence[m_curStateIndex];
	m_isInvincible = false;

	m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
	for (int i = 0; i < (int)m_sisters.size(); i++) {
		m_sisters[i]->GoToState( m_state );
	}
	if (m_state == 0) {
		for (int i = 0; i < (int)m_sisters.size(); i++) {
			m_sisters[i]->m_shootTimer->SetElapsedTime( 0.333f * i );
			if (i <= 1) {
				m_sisters[i]->m_changePositionTimer->SetElapsedTime( 0.333f * i );
			}
			else {
				m_sisters[i]->m_changePositionTimer->SetElapsedTime( 0.333f * i - 0.5f );
			}
			
		}
	}
	else if (m_state == 1) {
		for (int i = 0; i < (int)m_sisters.size(); i++) {
			m_sisters[i]->m_shootTimer->SetElapsedTime( 0.333f * i );
		}
	}
	else if (m_state == 2) {
		for (int i = 0; i < (int)m_sisters.size(); i++) {
			m_sisters[i]->m_jumpTimer->SetPeriodSeconds( 0.5f * i );
		}
	}
	m_stateTimer->Start();
}

void BacteriaBossThreeSisters::RenderUI() const
{
	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1050.f, 46.f ), Vec2( 1500.f, 81.f ) ), 35.f, Stringf( "Three Sisters" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

BacteriaBossSister::BacteriaBossSister( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_arrowDef(ProjectileDefinition::GetDefinition("Arrow"))
{
	m_verts.reserve( 1000 );
	m_environmentVerts.reserve( 300 );
	m_player = g_theGame->GetPlayerObject();
	m_shootTimer = new Timer( 0.8f, m_clock );
	//m_shootTimer->Start();
	m_changePositionTimer = new Timer( 0.5f, m_clock );
	m_curPosIndex = GetRandGen()->RollRandomIntInRange( 0, 2 );
	m_position = m_standPos[m_curPosIndex];
	m_jumpTimer = new Timer( 1.f, GetGameClock() );
}

BacteriaBossSister::~BacteriaBossSister()
{
	delete m_shootTimer;
	delete m_changePositionTimer;
	delete m_jumpTimer;
}

void BacteriaBossSister::BeginPlay()
{

}

void BacteriaBossSister::StartUp()
{
	AddVertsForDisc2D( m_environmentVerts, m_standPos[0], m_cosmeticRadius * 2.f, Rgba8( 204, 255, 153 ) );
	AddVertsForDisc2D( m_environmentVerts, m_standPos[1], m_cosmeticRadius * 2.f, Rgba8( 204, 255, 153 ) );
	AddVertsForDisc2D( m_environmentVerts, m_standPos[2], m_cosmeticRadius * 2.f, Rgba8( 204, 255, 153 ) );
	for (int i = 0; i < 20; i++) {
		AddVertsForDisc2D( m_verts, GetRandomPointInDisc2D( m_cosmeticRadius * 0.8f ), GetRandGen()->RollRandomFloatInRange( m_cosmeticRadius * 0.5f, m_cosmeticRadius * 0.8f ), Rgba8( (unsigned char)GetRandGen()->RollRandomIntInRange( 10, 50 ), 50, (unsigned char)GetRandGen()->RollRandomIntInRange( 10, 50 ) ) );
	}
}

void BacteriaBossSister::Update( float deltaTime )
{
	if (m_defeated && m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_defeated) {
		m_isDead = true;
		return;
	}
	if (m_isDead && m_state == 3) {
		m_health = Interpolate( 0.f, m_maxHealth * 0.5f, m_shootTimer->GetElapsedFraction() );
		return;
	}
	Entity::Update( deltaTime );
	if (!m_defeated) {
		m_isGarbage = false;
	}
	m_position = m_standPos[m_curPosIndex];

	if (m_state == 0) {
		if (m_changePositionTimer->HasPeriodElapsed()) {
			m_changePositionTimer->Stop();
			int newPosIndex;
			do {
				newPosIndex = GetRandGen()->RollRandomIntInRange( 0, 2 );
			} while (newPosIndex == m_curPosIndex);
			m_curPosIndex = newPosIndex;
			m_position = m_standPos[m_curPosIndex];
		}
		if (m_shootTimer->DecrementPeriodIfElapsed()) {
			float arrowOrientation = (m_player->m_position - m_position).GetOrientationDegrees();
			arrowOrientation = GetTurnedTowardDegrees( arrowOrientation, 90.f, GetRandGen()->RollRandomFloatInRange( 10.f, 30.f ) );
			Projectile* projectile = g_theGame->SpawnProjectileToGame( m_arrowDef, m_position, arrowOrientation, Vec2::MakeFromPolarDegrees( arrowOrientation, m_arrowDef.m_speed ) );
			projectile->m_faction = m_def.m_faction;
			m_changePositionTimer->Start();
		}
	}
	else if (m_state == 1) {
		if (m_shootTimer->DecrementPeriodIfElapsed()) {
			float arrowOrientation = (m_player->m_position - m_position).GetOrientationDegrees();
			arrowOrientation = GetTurnedTowardDegrees( arrowOrientation, 90.f, GetRandGen()->RollRandomFloatInRange( 10.f, 30.f ) );
			Projectile* projectile = g_theGame->SpawnProjectileToGame( m_arrowDef, m_position, arrowOrientation, Vec2::MakeFromPolarDegrees( arrowOrientation, m_arrowDef.m_speed ) );
			projectile->m_faction = m_def.m_faction;
		}
	}
	else if (m_state == 2) {
		if (m_jumpTimer->HasPeriodElapsed()) {
			if (m_jumpState == 0) {
				m_restrictIntoRoom = false;
				m_disableFriction = true;
				m_jumpState = 1;
				m_jumpTimer->SetPeriodSeconds( 1.5f );
				m_jumpTimer->Start();
			}
			else if (m_jumpState == 1) {
				m_jumpState = 2;
				m_jumpTargetPos = m_player->m_position;
				m_jumpTimer->SetPeriodSeconds( 1.5f );
				m_jumpTimer->Start();
			}
			else if (m_jumpState == 2) {
				m_restrictIntoRoom = true;
				m_disableFriction = false;
				m_jumpState = 3;
			}
		}

		if (m_jumpState == 1) {
			m_position = Interpolate( m_standPos[m_curPosIndex], Vec2( (m_index - 1) * 60.f + m_standPos[m_curPosIndex].x, m_standPos[m_curPosIndex].y + 100.f ), SmoothStop2( m_jumpTimer->GetElapsedFraction() ) );
		}
		else if (m_jumpState == 2) {
			if (m_jumpTimer->GetElapsedFraction() < 0.5f) {
				m_jumpTargetPos = m_player->m_position;
			}
			Vec2 curPos = Vec2( (m_index - 1) * 60.f + m_standPos[m_curPosIndex].x, m_standPos[m_curPosIndex].y + 100.f );
			Vec2 dispFwd = (m_jumpTargetPos - curPos).GetNormalized();
			m_position = Interpolate( curPos, m_jumpTargetPos + dispFwd * 50.f, SmoothStart2( m_jumpTimer->GetElapsedFraction() ) );
		}
	}
}

void BacteriaBossSister::Render() const
{
	if (m_isDead) {
		return;
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
	
}

void BacteriaBossSister::Die()
{
	m_isDead = true;
}

void BacteriaBossSister::GoToState( int stateIndex )
{
	m_state = stateIndex;
	m_restrictIntoRoom = true;
	m_disableFriction = false;
	if (m_state == 0) {
		m_shootTimer->SetPeriodSeconds( 1.f );
		m_shootTimer->Start();
		m_changePositionTimer->Start();
		//m_changePositionTimer->SetElapsedTime();
	}
	else if (m_state == 1) {
		m_shootTimer->SetPeriodSeconds( 1.f );
		m_shootTimer->Start();
	}
	else if (m_state == 2) {
		// wait state
		m_jumpState = 0;
		m_jumpTimer->SetPeriodSeconds( 1.f );
		m_jumpTimer->Start();
	}
	else if (m_state == 3) {
		m_shootTimer->SetPeriodSeconds( 4.f );
		m_shootTimer->Start();
	}
}

void BacteriaBossSister::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = m_index * 500.f + 100.f;
	float endX = startX + 400.f;

	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}
