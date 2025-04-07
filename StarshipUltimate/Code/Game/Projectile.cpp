#include "Game/Projectile.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

std::vector<ProjectileDefinition> ProjectileDefinition::s_definitions;

Projectile::Projectile( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	: m_position( startPos )
	, m_def( def )
	, m_orientationDegrees( startOrientation )
	, m_velocity( startVelocity )
{
	m_lifeTimer = new Timer( m_def.m_lifeSeconds, g_theGame->m_gameClock );
	m_lifeTimer->Start();
	m_rangeDamageRadius = m_def.m_damageRange;
	m_rangeDamagePercentage = m_def.m_damageModifier;
	m_damage *= m_def.m_damageModifier;
	if (m_rangeDamageRadius > 0.f) {
		m_hasRangeDamage = true;
	}
}

Projectile::~Projectile()
{
	delete m_lifeTimer;
	m_lifeTimer = nullptr;
}

void Projectile::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_addDamageByLifeTime) {
		m_damage += deltaTime;
	}

	if (m_lifeTimer->HasPeriodElapsed()) {
		Die( false );
	}
}

void Projectile::SpawnCollisionEffect()
{

}

void Projectile::DebugRender() const
{
	DebugDrawLine( m_position, m_orientationDegrees, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 0, 255 ) );
	DebugDrawLine( m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.2f, Rgba8( 0, 255, 0, 255 ) );
	DebugDrawRing( m_position, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 255, 255 ) );
	DebugDrawRing( m_position, m_physicsRadius, 0.2f, Rgba8( 0, 255, 255, 255 ) );
	DebugDrawLine( m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.2f, Rgba8( 255, 255, 0, 255 ) );
}

Vec2 Projectile::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}

bool Projectile::IsAlive() const
{
	return !m_isDead;
}

Mat44 Projectile::GetModelMatrix() const
{
	Mat44 translationMat = Mat44::CreateTranslation2D( m_position );
	translationMat.Append( Mat44::CreateZRotationDegrees( m_orientationDegrees ) );
	return translationMat;
}

Shuriken::Shuriken( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile( def, startPos, startOrientation, startVelocity )
{
	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.5f;
	m_damage = 1.f;
}

Shuriken::~Shuriken()
{

}

void Shuriken::BeginPlay()
{

}

void Shuriken::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	if (g_theGame->m_worldCamera.m_cameraBox.IsPointInside( m_position )) {
		m_position += m_velocity * deltaTime;
		m_orientationDegrees += 360.f * deltaTime;
	}
}

void Shuriken::Render() const
{
	Rgba8 bladeColor = Rgba8( 96, 96, 96 );
	float sideLength = 0.6f;
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
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
	AddVertsForDisc2D( verts, Vec2( 0.f, 0.f ), 0.8f, Rgba8( 192, 192, 192 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void Shuriken::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

ProjectileDefinition::ProjectileDefinition()
{

}

ProjectileDefinition::ProjectileDefinition( XmlElement* xmlIter )
{
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_lifeSeconds = ParseXmlAttribute( *xmlIter, "lifeSeconds", m_lifeSeconds );
	m_speed = ParseXmlAttribute( *xmlIter, "basicSpeed", m_speed );
	m_damageModifier = ParseXmlAttribute( *xmlIter, "damageModifier", m_damageModifier );
	m_damageRange = ParseXmlAttribute( *xmlIter, "damageRange", m_damageRange );
	m_rangeDamageModifier = ParseXmlAttribute( *xmlIter, "rangeDamageModifier", m_rangeDamageModifier );

}

void ProjectileDefinition::SetUpProjectileDefinitions()
{
	ProjectileDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/ProjectileDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document ProjectileDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ProjectileDefinitions" ), "Syntax Error! Name of the root of ProjectileDefinitions.xml should be \"ProjectileDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ProjectileDefinition" ), "Syntax Error! Names of the elements of ProjectileDefinitions.xml should be \"ProjectileDefinition\" " );
		ProjectileDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

ProjectileDefinition const& ProjectileDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "No Such Projectile Definition %s", name.c_str() ) );
}

Rocket::Rocket( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile(def, startPos, startOrientation, startVelocity)
{
	m_cosmeticRadius = 3.f;
	m_physicsRadius = 2.f;
}

Rocket::~Rocket()
{

}

void Rocket::BeginPlay()
{
	m_rangeDamageRadius = m_def.m_damageRange;
	m_rangeDamagePercentage = m_def.m_damageModifier;
	m_damage *= m_def.m_damageModifier;
	if (m_rangeDamageRadius > 0.f) {
		m_hasRangeDamage = true;
	}
}

void Rocket::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	m_position += m_velocity * deltaTime;
}

void Rocket::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForCapsule2D( verts, Vec2( -1.f, 0.f ), Vec2( 2.f, 0.f ), 0.5f, Rgba8( 102, 178, 255 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void Rocket::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

void Rocket::SpawnCollisionEffect()
{
	ParticleSystem2DAddEmitter( 400, 0.05f, AABB2( m_position - Vec2(m_cosmeticRadius, m_cosmeticRadius), m_position + Vec2( m_cosmeticRadius, m_cosmeticRadius ) ),
		FloatRange( m_cosmeticRadius * 0.2f, m_cosmeticRadius * 0.5f ),
		AABB2( Vec2( 0.f, 20.f ), -Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * 15.f + Vec2( 0.f, 20.f ) ),
		FloatRange( 0.6f, 1.f ), Rgba8( 0, 0, 255, 150 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 40.f, 75.f ), nullptr,
		Rgba8( 0, 0, 255, 0 ), 60.f, 0.f );
}

DemonBullet::DemonBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile(def, startPos, startOrientation, startVelocity)
{
	m_cosmeticRadius = 1.5f;
	m_physicsRadius = 1.6f;
}

DemonBullet::~DemonBullet()
{

}

void DemonBullet::BeginPlay()
{

}

void DemonBullet::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	m_position += m_velocity * deltaTime;
}

void DemonBullet::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8(204, 0, 0));
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void DemonBullet::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

void DemonBullet::SpawnCollisionEffect()
{
	ParticleSystem2DAddEmitter( 400, 0.05f, AABB2( m_position - Vec2( m_cosmeticRadius, m_cosmeticRadius ), m_position + Vec2( m_cosmeticRadius, m_cosmeticRadius ) ),
		FloatRange( m_cosmeticRadius * 0.2f, m_cosmeticRadius * 0.5f ),
		AABB2( Vec2( 0.f, 20.f ), -Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * 15.f + Vec2( 0.f, 20.f ) ),
		FloatRange( 0.6f, 1.f ), Rgba8( 204, 0, 0, 150 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 40.f, 75.f ), nullptr,
		Rgba8( 204, 0, 0, 0 ), 60.f, 0.f );
}

EnemyBullet::EnemyBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile(def, startPos, startOrientation, startVelocity)
{
	m_cosmeticRadius = 1.3f;
	m_physicsRadius = 1.6f;
}

EnemyBullet::~EnemyBullet()
{

}

void EnemyBullet::BeginPlay()
{

}

void EnemyBullet::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	m_position += m_velocity * deltaTime;
}

void EnemyBullet::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );
	constexpr int NUM_SIDES = 20;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	float radius = m_cosmeticRadius * 0.7f;
	float thickness = m_cosmeticRadius * 0.6f;
	Rgba8 outerColor = Rgba8( 255, 102, 102, 100 );
	Rgba8 innerColor = Rgba8( 255, 102, 102 );
	for (int i = 0; i < NUM_SIDES; i++) {
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius ), innerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness) ), outerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius ), innerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius ), innerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness) ), outerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness) ), outerColor );
	}

	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius * 0.7f, Rgba8( 255, 255, 255 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void EnemyBullet::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

void EnemyBullet::SpawnCollisionEffect()
{

}

CurveMissile::CurveMissile( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile(def, startPos, startOrientation, startVelocity)
{
	m_physicsRadius = 0.5f;
	m_cosmeticRadius = 0.6f;
}

CurveMissile::~CurveMissile()
{

}

void CurveMissile::BeginPlay()
{
	m_curve.m_startPos = m_position;
	m_curve.m_endPos = m_targetPosition;
	float distance = GetDistance2D( m_targetPosition, m_position );
	Vec2 middlePos = (m_position + m_targetPosition) * 0.5f;

	Vec2 sideVec = (m_targetPosition - m_position).GetNormalized();
	if (m_goUp) {
		sideVec.Rotate90Degrees();
	}
	else {
		sideVec.RotateMinus90Degrees();
	}
	m_curve.m_guidePos1 = middlePos + sideVec * distance * 0.3f;
	m_curve.m_guidePos2 = m_curve.m_guidePos1;
}

void CurveMissile::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	m_distance += deltaTime * m_def.m_speed;
	m_lastFramePos = m_position;
	m_position = m_curve.EvaluateAtApproximateDistance( m_distance );
	m_orientationDegrees = (m_position - m_lastFramePos).GetOrientationDegrees();

	if (Starship_IsVec2NearlyZero( m_position - m_targetPosition )) {
		// die
		Die();
	}
}

void CurveMissile::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	Starship_AddVertsForArch( verts, Vec2( -0.5f, 0.f ), Vec2( 0.5f, 0.f ), 0.5f, Rgba8( 255, 178, 102 ) );
	verts.emplace_back( Vec2( -0.5f, 0.5f ), Rgba8( 255, 178, 102 ) );
	verts.emplace_back( Vec2( -4.f, 0.f ), Rgba8( 255, 178, 102, 100 ) );
	verts.emplace_back( Vec2( -0.5f, -0.5f ), Rgba8( 255, 178, 102 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void CurveMissile::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;

	ParticleSystem2DAddEmitter( 300, 0.05f, AABB2( m_position, m_position ),
		FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 1.2f ),
		AABB2( Vec2( -10.f, -10.f ), Vec2( 10.f, 10.f ) ),
		FloatRange( 0.2f, 0.4f ), Rgba8( 255, 178, 102 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 40.f, 75.f ), nullptr,
		Rgba8( 255, 178, 102, 0 ) );
}

void CurveMissile::SpawnCollisionEffect()
{

}

CoinBullet::CoinBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile( def, startPos, startOrientation, startVelocity )
{
	m_cosmeticRadius = 0.6f;
	m_physicsRadius = 0.8f;
}

CoinBullet::~CoinBullet()
{

}

void CoinBullet::BeginPlay()
{

}

void CoinBullet::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	m_position += m_velocity * deltaTime;
}

void CoinBullet::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );
	constexpr int NUM_SIDES = 20;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	float radius = m_cosmeticRadius * 0.7f;
	float thickness = m_cosmeticRadius * 0.6f;
	Rgba8 outerColor = Rgba8( 255, 153, 51, 100 );
	Rgba8 innerColor = Rgba8( 255, 153, 51 );
	for (int i = 0; i < NUM_SIDES; i++) {
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius ), innerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness) ), outerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius ), innerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius ), innerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness) ), outerColor );
		verts.emplace_back( Vec2( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness) ), outerColor );
	}

	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius * 0.7f, Rgba8( 255, 255, 51 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void CoinBullet::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

void CoinBullet::SpawnCollisionEffect()
{
	ParticleSystem2DAddEmitter( 300, 0.05f, AABB2( m_position, m_position ),
		FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 1.2f ),
		AABB2( Vec2( -10.f, -10.f ), Vec2( 10.f, 10.f ) ),
		FloatRange( 0.2f, 0.4f ), Rgba8( 255, 178, 102 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 40.f, 75.f ), nullptr,
		Rgba8( 255, 178, 102, 0 ) );

	// return money
	float rnd = GetRandGen()->RollRandomFloatZeroToOne();
	if (rnd < 0.8f) {
		g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
	}
	else {
		g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
		g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
	}
}

SharpenedObsidian::SharpenedObsidian( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile(def, startPos, startOrientation, startVelocity)
{
	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.8f;
	m_damage = 1.f;
}

SharpenedObsidian::~SharpenedObsidian()
{

}

void SharpenedObsidian::BeginPlay()
{

}

void SharpenedObsidian::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	if (g_theGame->m_worldCamera.m_cameraBox.IsPointInside( m_position )) {
		m_position += m_velocity * deltaTime;
	}
}

void SharpenedObsidian::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );

	constexpr int NUM_OF_DEBRIS_VERTS = 48;
	float randR[NUM_OF_DEBRIS_VERTS / 3];
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		randR[i] = g_engineRNG->RollRandomFloatInRange( 0.8f * m_cosmeticRadius, 1.f * m_cosmeticRadius );
	}
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		Vertex_PCU vert1, vert2, vert3;
		vert1.m_position = Vec3( 0, 0, 0 );
		vert1.m_color = Rgba8( 51, 0, 102 );
		vert1.m_uvTexCoords = Vec2( 0, 0 );
		vert2.m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], 0 );
		vert2.m_color = Rgba8( 51, 0, 102 );
		vert2.m_uvTexCoords = Vec2( 0, 0 );
		vert3.m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], 0 );
		vert3.m_color = Rgba8( 51, 0, 102 );
		vert3.m_uvTexCoords = Vec2( 0, 0 );
		verts.push_back( vert1 );
		verts.push_back( vert2 );
		verts.push_back( vert3 );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void SharpenedObsidian::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

Arrow::Arrow( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile( def, startPos, startOrientation, startVelocity )
{
	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 0.8f;
	m_damage = 1.f;
}

Arrow::~Arrow()
{

}

void Arrow::BeginPlay()
{

}

void Arrow::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	if (g_theGame->m_worldCamera.m_cameraBox.IsPointInside( m_position )) {
		m_velocity += (Vec2( 0.f, -50.f ) * deltaTime);
		m_orientationDegrees = m_velocity.GetOrientationDegrees();
		m_position += m_velocity * deltaTime;
	}
}

void Arrow::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 30 );
	
	AddVertsForAABB2D( verts, AABB2( Vec2( -3.f * m_cosmeticRadius, -0.5f * m_cosmeticRadius ), Vec2( 0.f, 0.5f * m_cosmeticRadius ) ), Rgba8( 102, 51, 0 ) );
	verts.emplace_back( Vec2( 0.f, m_cosmeticRadius ), Rgba8( 224, 224, 224 ) );
	verts.emplace_back( Vec2( 0.f, -m_cosmeticRadius ), Rgba8( 224, 224, 224 ) );
	verts.emplace_back( Vec2( 0.5f * m_cosmeticRadius, 0.f ), Rgba8( 224, 224, 224 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void Arrow::Die( bool dieByCollision /*= true */ )
{
	UNUSED( dieByCollision );
	m_isDead = true;
}

