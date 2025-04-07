#include "Game/Effect.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

Effect::Effect( Vec3 const& pos, EulerAngles const& orientation )
	:m_position(pos), m_orientation(orientation)
{

}

Effect::~Effect()
{

}

DamageNumber::DamageNumber( Vec3 const& pos, EulerAngles const& orientation )
	:Effect( pos, orientation )
{
	m_type = EffectType::Damage_Number;
}

DamageNumber::~DamageNumber()
{

}

void DamageNumber::BeginPlay()
{

}

void DamageNumber::Update()
{
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	m_timePeriod -= deltaSeconds;
	if (m_timePeriod <= 0.f) {
		m_isGarbage = true;
		return;
	}
	m_position += (Vec3( 0.f, 0.f, 0.2f ) * deltaSeconds);
}

void DamageNumber::Render()
{
	std::vector<Vertex_PCU> verts;
	g_ASCIIFont->AddVertsForText3DAtOriginXForward( verts, 0.4f, Stringf( "%d", m_damageValue ), Rgba8::WHITE, 0.5f );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_FACING, g_theGame->m_playerPOV.GetTransformMatrix(), m_position ), Rgba8( 255, 0, 0, unsigned char( 255 * m_timePeriod / DamageNumberLifeTime ) ) );
	g_theRenderer->DrawVertexArray( verts );
}

ConeSmokeParticle::ConeSmokeParticle( Vec3 const& pos /*= Vec3()*/, EulerAngles const& orientation /*= EulerAngles() */ )
	:Effect( pos, orientation )
{
	m_type = EffectType::Cone_Smoke_Particle;
}

ConeSmokeParticle::~ConeSmokeParticle()
{

}

void ConeSmokeParticle::BeginPlay()
{
	m_direction = GetRandomDirectionInCone3D( m_direction, m_maxHalfDegrees, m_maxHalfDegrees );
	m_texture = g_theRenderer->CreateOrGetTextureFromFile( Stringf( "Data/Images/Particles/Smoke0%d.png", g_theGame->m_randNumGen->RollRandomIntInRange( 1, 9 ) ).c_str() );
	m_timer = m_timePeriod;
}

void ConeSmokeParticle::Update()
{
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	m_timer -= deltaSeconds;
	if (m_timer <= 0.f) {
		m_isGarbage = true;
		return;
	}
	m_position += m_direction * m_speed * deltaSeconds;
	m_size += Vec2( 0.1f, 0.1f ) * deltaSeconds;
	m_speed = std::max( 0.f, m_speed - 0.3f * deltaSeconds );
}

void ConeSmokeParticle::Render()
{
	std::vector<Vertex_PCU> verts;
	AddVertsForQuad3D( verts, Vec3( 0.f, -m_size.x * 0.5f, -m_size.y * 0.5f ), Vec3( 0.f, m_size.x * 0.5f, -m_size.y * 0.5f ), Vec3( 0.f, m_size.x * 0.5f, m_size.y * 0.5f ), Vec3( 0.f, -m_size.x * 0.5f, m_size.y * 0.5f ), Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_FACING, g_theGame->m_playerPOV.GetTransformMatrix(), m_position ), Rgba8( 255, 255, 255, unsigned char( 255 * m_timer / m_timePeriod ) ) );
	g_theRenderer->DrawVertexArray( verts );
}

SphereFireParticle::SphereFireParticle( Vec3 const& pos /*= Vec3()*/, EulerAngles const& orientation /*= EulerAngles() */ )
	:Effect( pos, orientation )
{
	m_type = EffectType::Sphere_Fire_Particle;
}

SphereFireParticle::~SphereFireParticle()
{

}

void SphereFireParticle::BeginPlay()
{
	m_direction = GetRandomDirection3D();
	m_texture = g_theRenderer->CreateOrGetTextureFromFile( Stringf( "Data/Images/Particles/Fire0%d.png", g_theGame->m_randNumGen->RollRandomIntInRange( 1, 2 ) ).c_str() );
	m_timer = m_timePeriod;
}

void SphereFireParticle::Update()
{
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	m_timer -= deltaSeconds;
	if (m_timer <= 0.f) {
		m_isGarbage = true;
		return;
	}
	m_position += m_direction * m_speed * deltaSeconds;
	m_size += Vec2( 0.1f, 0.1f ) * deltaSeconds;
	m_speed = std::max( 0.f, m_speed - 0.3f * deltaSeconds );
}

void SphereFireParticle::Render()
{
	std::vector<Vertex_PCU> verts;
	AddVertsForQuad3D( verts, Vec3( 0.f, -m_size.x * 0.5f, -m_size.y * 0.5f ), Vec3( 0.f, m_size.x * 0.5f, -m_size.y * 0.5f ), Vec3( 0.f, m_size.x * 0.5f, m_size.y * 0.5f ), Vec3( 0.f, -m_size.x * 0.5f, m_size.y * 0.5f ), Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_FACING, g_theGame->m_playerPOV.GetTransformMatrix(), m_position ), Rgba8( 255, 255, 255, unsigned char( 255 * m_timer / m_timePeriod ) ) );
	g_theRenderer->DrawVertexArray( verts );
}

ConeMuzzleParticle::ConeMuzzleParticle( Vec3 const& pos /*= Vec3()*/, EulerAngles const& orientation /*= EulerAngles() */ )
	:Effect( pos, orientation )
{
	m_type = EffectType::Cone_Muzzle_Particle;

}

ConeMuzzleParticle::~ConeMuzzleParticle()
{

}

void ConeMuzzleParticle::BeginPlay()
{
	m_direction = GetRandomDirectionInCone3D( m_direction, m_maxHalfDegrees, m_maxHalfDegrees );
	m_texture = g_theRenderer->CreateOrGetTextureFromFile( Stringf( "Data/Images/Particles/Muzzle0%d.png", g_theGame->m_randNumGen->RollRandomIntInRange( 1, 5 ) ).c_str() );
	m_timer = m_timePeriod;
}

void ConeMuzzleParticle::Update()
{
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	m_timer -= deltaSeconds;
	if (m_timer <= 0.f) {
		m_isGarbage = true;
		return;
	}
	m_position += m_direction * m_speed * deltaSeconds;
	m_size += Vec2( 0.1f, 0.1f ) * deltaSeconds;
	m_speed = std::max( 0.f, m_speed - 0.3f * deltaSeconds );
}

void ConeMuzzleParticle::Render()
{
	std::vector<Vertex_PCU> verts;
	AddVertsForQuad3D( verts, Vec3( 0.f, -m_size.x * 0.5f, -m_size.y * 0.5f ), Vec3( 0.f, m_size.x * 0.5f, -m_size.y * 0.5f ), Vec3( 0.f, m_size.x * 0.5f, m_size.y * 0.5f ), Vec3( 0.f, -m_size.x * 0.5f, m_size.y * 0.5f ), Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_FACING, g_theGame->m_playerPOV.GetTransformMatrix(), m_position ), Rgba8( 255, 255, 255, unsigned char( 255 * m_timer / m_timePeriod ) ) );
	g_theRenderer->DrawVertexArray( verts );
}

Rocket::Rocket( Vec3 const& pos /*= Vec3()*/, EulerAngles const& orientation /*= EulerAngles() */ )
	:Effect( pos, orientation )
{
	m_type = EffectType::Art_Rocket;
	m_smokeParticleTimer = new Timer( 0.04f, Clock::GetSystemClock() );
}

Rocket::~Rocket()
{
	delete m_smokeParticleTimer;
}

void Rocket::BeginPlay()
{
	m_smokeParticleTimer->Start();
	m_position = m_startPos;
	m_timer = 0.f;
	m_pivotPos = (m_startPos + m_endPos) * 0.5f + Vec3( 0.f, 0.f, (m_endPos - m_startPos).GetLength() * 0.6f );
}

void Rocket::Update()
{
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	m_timer += deltaSeconds;
	if (m_timer >= m_timePeriod) {
		m_isGarbage = true;
		return;
	}
	float ratio = m_timer / m_timePeriod;
	ratio = SmoothStart2( ratio );
	m_position = GetPosByTimeRatio( ratio );
	m_orientation = (m_position - GetPosByTimeRatio( ratio - 0.01f )).GetOrientationEulerAngles();
	//m_orientation.m_pitchDegrees = -m_orientation.m_pitchDegrees;
	if (m_smokeParticleTimer->DecrementPeriodIfElapsed()) {
		Effect* effect = GetCurMap()->SpawnEffect( EffectType::Cone_Smoke_Particle, m_position );
		((ConeSmokeParticle*)effect)->m_direction = -m_orientation.GetIFwd();
		((ConeSmokeParticle*)effect)->m_timePeriod = 2.f;
		((ConeSmokeParticle*)effect)->m_speed = 0.2f;
		((ConeSmokeParticle*)effect)->m_maxHalfDegrees = 10.f;
		((ConeSmokeParticle*)effect)->m_size = Vec2( 0.3f, 0.3f );
		effect->BeginPlay();
	}
}

void Rocket::Render()
{
	std::vector<Vertex_PCU> verts;
	AddVertsForCylinder3D( verts, Vec3( -0.23f, 0.f, 0.f ), Vec3( -0.2f, 0.f, 0.f ), 0.07f, Rgba8( 255, 255, 0 ) );
	AddVertsForCylinder3D( verts, Vec3( -0.2f, 0.f, 0.f ), Vec3( 0.2f, 0.f, 0.f ), 0.05f, Rgba8( 255, 255, 0 ) );
	AddVertsForCone3D( verts, Vec3( 0.2f, 0.f, 0.f ), Vec3( 0.25f, 0.f, 0.f ), 0.05f, Rgba8( 255, 255, 0 ) );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants( GetModelMatrix() );
	g_theRenderer->DrawVertexArray( verts );
}

Vec3 Rocket::GetPosByTimeRatio( float ratio ) const
{
	Vec3 a = m_startPos * (1.f - ratio) + m_pivotPos * ratio;
	Vec3 b = m_pivotPos * (1.f - ratio) + m_endPos * ratio;
	return a * (1.f - ratio) + b * ratio;
}

Mat44 Rocket::GetModelMatrix() const
{
	Mat44 retMat = Mat44::CreateTranslation3D( m_position );
	retMat.Append( m_orientation.GetAsMatrix_IFwd_JLeft_KUp() );
	return retMat;
}
