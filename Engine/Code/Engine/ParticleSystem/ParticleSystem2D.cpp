#include "Engine/ParticleSystem/ParticleSystem2D.hpp"
#include "Engine/Math/EngineMath.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"

// forward declaration
class ParticleEmitter2D;
class Particle2D;
void ParticleSystem2DClear();

static ParticleSystem2DConfig ps2D_config;
std::vector<ParticleEmitter2D*> ps2D_emitters;
std::vector<Particle2D*> ps2D_particles;
RandomNumberGenerator* ps2D_rnd = nullptr;

class Particle2D {
public:
	Particle2D( Vec2 const& position, float lifeTimeSeconds, Rgba8 const& startColor, Particle2DShape shape, float size,
		Vec2 const& velocity = Vec2(), Rgba8 const endColor = EQUAL_TO_START_COLOR, Texture* texture = nullptr, float orientationDegrees = 0.f, 
		float angularSpeedDegrees = 0.f, float gravityDrag = 0.f, float airDrag = 0.f);
	void Update( float deltaSeconds );
	void Render() const;
	~Particle2D();

	Vec2 m_position;
	Vec2 m_velocity;
	float m_orientationDegrees;
	float m_angularSpeedDegrees;
	float m_lifeTime;
	float m_curlifeTime = 0.f;
	Texture* m_texture;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	Particle2DShape m_shape;
	float m_size;

	float m_gravityDrag;
	float m_airDrag;

	VertexBuffer* m_vertexBuffer = nullptr;
};

Particle2D::Particle2D( Vec2 const& position, float lifeTimeSeconds, Rgba8 const& startColor, Particle2DShape shape, float size, Vec2 const& velocity /*= Vec2()*/, Rgba8 const endColor /*= EQUAL_TO_START_COLOR*/, Texture* texture /*= nullptr*/, float orientationDegrees /*= 0.f*/, float angularSpeedDegrees /*= 0.f*/, float gravityDrag /*= false*/, float airDrag /*= 0.f*/ )
	:m_position(position)
	,m_velocity(velocity)
	,m_orientationDegrees(orientationDegrees)
	,m_angularSpeedDegrees(angularSpeedDegrees)
	,m_lifeTime(lifeTimeSeconds)
	,m_texture(texture)
	,m_startColor(startColor)
	,m_endColor(endColor)
	,m_shape(shape)
	,m_size(size)
	,m_gravityDrag(gravityDrag)
	,m_airDrag(airDrag)
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	if (m_shape == Particle2DShape::Asteroid) {
		constexpr int NUM_OF_DEBRIS_VERTS = 48;
		float randR[NUM_OF_DEBRIS_VERTS / 3];
		for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
			randR[i] = g_engineRNG->RollRandomFloatInRange( 0.3f * size, 0.6f * size );
		}
		for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
			Vertex_PCU vert1, vert2, vert3;
			vert1.m_position = Vec3( 0, 0, 0 );
			vert1.m_color = Rgba8::WHITE;
			vert1.m_uvTexCoords = Vec2( 0, 0 );
			vert2.m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], 0 );
			vert2.m_color = Rgba8::WHITE;
			vert2.m_uvTexCoords = Vec2( 0, 0 );
			vert3.m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], 0 );
			vert3.m_color = Rgba8::WHITE;
			vert3.m_uvTexCoords = Vec2( 0, 0 );
			verts.push_back( vert1 );
			verts.push_back( vert2 );
			verts.push_back( vert3 );
		}
	}
	else if (m_shape == Particle2DShape::Box) {
		AddVertsForAABB2D( verts, AABB2( Vec2( -m_size * 0.5f, -m_size * 0.5f ), Vec2( m_size * 0.5f, m_size * 0.5f ) ), Rgba8::WHITE );
	}
	else if (m_shape == Particle2DShape::Disc) {
		AddVertsForDisc2D( verts, Vec2(), m_size, Rgba8::WHITE );
	}
	else if (m_shape == Particle2DShape::Sector) {
		AddVertsForSector2D( verts, Vec2(), Vec2( 1.f, 0.f ), 60.f, m_size, Rgba8::WHITE );
	}
	m_vertexBuffer = ps2D_config.m_renderer->CreateVertexBuffer( verts.size() );
	ps2D_config.m_renderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_vertexBuffer );
}

void Particle2D::Update( float deltaSeconds )
{
	m_curlifeTime += deltaSeconds;
	m_velocity -= m_velocity * (m_airDrag * deltaSeconds);
	m_velocity.y -= m_gravityDrag * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularSpeedDegrees * deltaSeconds;
}

void Particle2D::Render() const
{
	Renderer* renderer = ps2D_config.m_renderer;
	Mat44 modelMatrix = Mat44::CreateTranslation2D( m_position );
	modelMatrix.Append( Mat44::CreateZRotationDegrees( m_orientationDegrees ) );
	renderer->SetModelConstants( modelMatrix, Rgba8::Interpolate( m_startColor, m_endColor, Minf( m_curlifeTime / m_lifeTime, 1.f ) ) );
	renderer->BindTexture( m_texture );
	renderer->DrawVertexBuffer( m_vertexBuffer, (int)(m_vertexBuffer->GetVertexCount()) );
}

Particle2D::~Particle2D()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
}

class ParticleEmitter2D {
public:
	ParticleEmitter2D( int particlesPerSecond, float emitterPeriodTime, AABB2 const& spawnBounds, FloatRange const& particleStartSize, AABB2 const& particleStartVelocity,
		FloatRange const& particleLifeTime, Rgba8 const& particleStartColor, Particle2DShape particleShape, bool beginActive = true,
		FloatRange const& particleStartOrientation = FloatRange( 0.f, 0.f ), FloatRange const& particleStartAngularSpeed = FloatRange( 0.f, 0.f ),
		Texture* particleTexture = nullptr, Rgba8 const& particleEndColor = EQUAL_TO_START_COLOR, float particleGravityDrag = 0.f, float particleAirDrag = 0.f );
	void Update( float deltaSeconds );
	void Render();
	bool IsActive() const;
	void Restart();
	void SetActive( bool active );
	void SetCenter( Vec2 const newPos );

	bool m_isActive = true;
	int m_particlesPerSecond;
	int m_numOfParticlesEmittedThisPeriod = 0;
	float m_emitterPeriodTime;
	float m_curEmitterPeriodTime = 0.f;
	Vec2 m_centerPosition;
	AABB2 m_spawnBounds;

	FloatRange m_particleStartSize;
	FloatRange m_particleStartVelocityX;
	FloatRange m_particleStartVelocityY;
	FloatRange m_particleStartOrientation;
	FloatRange m_particleStartAngularSpeed;
	FloatRange m_particleLifeTime;
	Texture* m_particleTexture;
	Rgba8 m_particleStartColor;
	Rgba8 m_particleEndColor;
	Particle2DShape m_particleShape;
	float m_particleAirDrag;
	float m_particleGravityDrag;
};

ParticleEmitter2D::ParticleEmitter2D( int particlesPerSecond, float emitterPeriodTime, AABB2 const& spawnBounds, FloatRange const& particleStartSize, AABB2 const& particleStartVelocity, FloatRange const& particleLifeTime, Rgba8 const& particleStartColor, Particle2DShape particleShape, bool beginActive /*= true*/, FloatRange const& particleStartOrientation /*= FloatRange( 0.f, 0.f )*/, FloatRange const& particleStartAngularSpeed /*= FloatRange( 0.f, 0.f )*/, Texture* particleTexture /*= nullptr*/, Rgba8 const& particleEndColor /*= EQUAL_TO_START_COLOR*/, float particleGravityDrag /*= false*/, float particleAirDrag /*= 0.f */ )
	:m_isActive(beginActive)
	,m_particlesPerSecond(particlesPerSecond)
	,m_emitterPeriodTime(emitterPeriodTime)
	,m_spawnBounds(spawnBounds)
	,m_centerPosition(spawnBounds.GetCenter())
	,m_particleStartSize(particleStartSize)
	,m_particleStartVelocityX(FloatRange(particleStartVelocity.m_mins.x, particleStartVelocity.m_maxs.x))
	,m_particleStartVelocityY(FloatRange(particleStartVelocity.m_mins.y, particleStartVelocity.m_maxs.y))
	,m_particleStartOrientation(particleStartOrientation)
	,m_particleStartAngularSpeed(particleStartAngularSpeed)
	,m_particleLifeTime(particleLifeTime)
	,m_particleTexture(particleTexture)
	,m_particleStartColor(particleStartColor)
	,m_particleEndColor(particleEndColor)
	,m_particleShape(particleShape)
	,m_particleAirDrag(particleAirDrag)
	,m_particleGravityDrag(particleGravityDrag)
{
	if (m_particleEndColor == EQUAL_TO_START_COLOR) {
		m_particleEndColor = m_particleStartColor;
	}
}

void ParticleEmitter2D::Update( float deltaSeconds )
{
	if(!m_isActive){
		return;
	}
	if (m_curEmitterPeriodTime > m_emitterPeriodTime && m_emitterPeriodTime != -1.f) {
		return;
	}
	m_curEmitterPeriodTime += deltaSeconds;
	int totalParticlesNeededThisFrame = RoundDownToInt( m_curEmitterPeriodTime * m_particlesPerSecond );
	int particlesNeededThisFrame = totalParticlesNeededThisFrame - m_numOfParticlesEmittedThisPeriod;
	if (particlesNeededThisFrame > 0) {
		m_numOfParticlesEmittedThisPeriod = totalParticlesNeededThisFrame;
		for (int i = 0; i < particlesNeededThisFrame; i++) {
			Vec2 position = Vec2( ps2D_rnd->RollRandomFloatInRange( m_spawnBounds.m_mins.x, m_spawnBounds.m_maxs.x ),
				ps2D_rnd->RollRandomFloatInRange( m_spawnBounds.m_mins.y, m_spawnBounds.m_maxs.y ) );
			Vec2 velocity = Vec2( ps2D_rnd->RollRandomFloatInRange( m_particleStartVelocityX ), ps2D_rnd->RollRandomFloatInRange( m_particleStartVelocityY ) );
			float lifeTime = ps2D_rnd->RollRandomFloatInRange( m_particleLifeTime );
			float size = ps2D_rnd->RollRandomFloatInRange( m_particleStartSize );
			float orientation = ps2D_rnd->RollRandomFloatInRange( m_particleStartOrientation );
			float angularSpeed = ps2D_rnd->RollRandomFloatInRange( m_particleStartAngularSpeed );
			Particle2D* particle = new Particle2D( position, lifeTime, m_particleStartColor, m_particleShape, size, velocity, m_particleEndColor, m_particleTexture, orientation, angularSpeed, m_particleGravityDrag, m_particleAirDrag );
			for (int j = 0; j < (int)ps2D_particles.size(); j++) {
				if (ps2D_particles[j] == nullptr) {
					ps2D_particles[j] = particle;
					particle = nullptr;
					break;
				}
			}
			if (particle) {
				ps2D_particles.push_back( particle );
			}
		}
	}
}

void ParticleEmitter2D::Render()
{
	// Do nothing now
}

bool ParticleEmitter2D::IsActive() const
{
	return m_isActive;
}

void ParticleEmitter2D::Restart()
{
	m_curEmitterPeriodTime = 0.f;
	m_numOfParticlesEmittedThisPeriod = 0;
}

void ParticleEmitter2D::SetActive( bool active )
{
	if (m_isActive == true && active == true) {
		Restart();
	}
	else {
		m_isActive = active;
	}
}

void ParticleEmitter2D::SetCenter( Vec2 const newPos )
{
	m_centerPosition = newPos;
	m_spawnBounds.SetCenter( m_centerPosition );
}

void ParticleSystem2DClear()
{
	for (int i = 0; i < (int)ps2D_particles.size(); i++) {
		delete ps2D_particles[i];
		ps2D_particles[i] = nullptr;
	}

	for (int i = 0; i < (int)ps2D_emitters.size(); i++) {
		delete ps2D_emitters[i];
		ps2D_emitters[i] = nullptr;
	}
}


void ParticleSystem2DStartup( ParticleSystem2DConfig const& config )
{
	ps2D_config = config;
	ps2D_particles.reserve( 1000 );
	ps2D_emitters.reserve( 100 );
	ps2D_rnd = new RandomNumberGenerator();
}

void ParticleSystem2DShutdown()
{
	delete ps2D_rnd;
	ps2D_rnd = nullptr;
	ParticleSystem2DClear();
}

void ParticleSystem2DBeginFrame()
{

}

void ParticleSystem2DUpdate()
{
	float deltaSeconds = ps2D_config.m_clock->GetDeltaSeconds();
	for (auto& particle : ps2D_particles) {
		if (particle) {
			if (particle->m_curlifeTime > particle->m_lifeTime) {
				delete particle;
				particle = nullptr;
			}
			else {
				particle->Update( deltaSeconds );
			}
		}
	}
	for (auto emitter : ps2D_emitters) {
		if (emitter) {
			emitter->Update( deltaSeconds );
		}
	}
}

void ParticleSystem2DRender( Camera const& camera )
{
	Renderer* renderer = ps2D_config.m_renderer;
	renderer->BeginCamera( camera );
	for (auto particle : ps2D_particles) {
		if (particle) {
			particle->Render();
		}
	}
	for (auto emitter : ps2D_emitters) {
		if (emitter) {
			emitter->Render();
		}
	}
	renderer->EndCamera( camera );
}

void ParticleSystem2DEndFrame()
{

}

ParticleEmitter2D_UID ParticleSystem2DAddEmitter( int particlesPerSecond, float emitterPeriodTime, AABB2 const& spawnBounds, FloatRange const& particleStartSize, AABB2 const& particleStartVelocity, FloatRange const& particleLifeTime, Rgba8 const& particleStartColor, Particle2DShape particleShape, bool beginActive /*= true*/, FloatRange const& particleStartOrientation /*= FloatRange( 0.f, 0.f )*/, FloatRange const& particleStartAngularSpeed /*= FloatRange( 0.f, 0.f )*/, Texture* particleTexture /*= nullptr*/, Rgba8 const& particleEndColor /*= EQUAL_TO_START_COLOR*/, float particleGravityDrag /*= 0.f*/, float particleAirDrag /*= 0.f */ )
{
	ParticleEmitter2D* emitter = new ParticleEmitter2D( particlesPerSecond, emitterPeriodTime, spawnBounds, particleStartSize, particleStartVelocity, particleLifeTime, particleStartColor, particleShape, beginActive, particleStartOrientation, particleStartAngularSpeed, particleTexture, particleEndColor, particleGravityDrag, particleAirDrag );
	for (size_t i = 0; i < ps2D_emitters.size(); i++) {
		if (ps2D_emitters[i] == nullptr) {
			ps2D_emitters[i] = emitter;
			return (ParticleEmitter2D_UID)i;
		}
	}
	ParticleEmitter2D_UID pos = ps2D_emitters.size();
	ps2D_emitters.push_back( emitter );
	return pos;
}

bool ParticleSystem2DIsEmitterActive( ParticleEmitter2D_UID const uid )
{
	return ps2D_emitters[uid]->IsActive();
}

void ParticleSystem2DRestartEmitter( ParticleEmitter2D_UID const uid )
{
	return ps2D_emitters[uid]->Restart();
}

void ParticleSystem2DSetEmitterActive( ParticleEmitter2D_UID const uid, bool active )
{
	return ps2D_emitters[uid]->SetActive( active );
}

void ParticleSystem2DSetEmitterCenter( ParticleEmitter2D_UID const uid, Vec2 const newCenterPos )
{
	return ps2D_emitters[uid]->SetCenter( newCenterPos );
}

void ParticleSystem2DDeleteEmitter( ParticleEmitter2D_UID const uid )
{
	delete ps2D_emitters[uid];
	ps2D_emitters[uid] = nullptr;
}
