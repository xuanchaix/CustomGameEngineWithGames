#pragma once
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/FloatRange.hpp"

class Renderer;
class Camera;
class Clock;
class Texture;
struct Vec2;
struct Rgba8;
struct AABB2;

static Rgba8 const EQUAL_TO_START_COLOR = Rgba8( 0, 0, 0, 0 );
typedef size_t ParticleEmitter2D_UID;

struct ParticleSystem2DConfig {
	Renderer* m_renderer = nullptr;
	Clock* m_clock = nullptr;
};

enum class Particle2DShape {
	Box, Disc, Sector, Asteroid, Custom
};


namespace ParticleSystem2D {
	enum class Vec2GeneratorShape {
		Disc, AABB2, OBB2
	};
	class RandomVec2Generator {
		RandomVec2Generator( Vec2GeneratorShape shape );

		Vec2GeneratorShape m_shape;
	};



	
}

/*
enum class ParticleEmitter2D {

};*/

void ParticleSystem2DStartup( ParticleSystem2DConfig const& config );
void ParticleSystem2DShutdown();

void ParticleSystem2DBeginFrame();
void ParticleSystem2DUpdate();
void ParticleSystem2DRender( Camera const& camera );
void ParticleSystem2DEndFrame();

/// <summary>
/// 
/// </summary>
/// <param name="particlesPerSecond"></param>
/// <param name="emitterPeriodTime"> '-1' means loop </param>
/// <param name="spawnBounds"></param>
/// <param name="particleStartSize"></param>
/// <param name="particleStartVelocity"></param>
/// <param name="particleLifeTime"></param>
/// <param name="particleStartColor"></param>
/// <param name="particleShape"></param>
/// <param name="beginActive"></param>
/// <param name="particleStartOrientation"></param>
/// <param name="particleStartAngularSpeed"></param>
/// <param name="particleTexture"></param>
/// <param name="particleEndColor"></param>
/// <param name="particleGravityDrag"></param>
/// <param name="particleAirDrag"></param>
/// <returns></returns>
ParticleEmitter2D_UID ParticleSystem2DAddEmitter( int particlesPerSecond, float emitterPeriodTime, AABB2 const& spawnBounds, FloatRange const& particleStartSize, AABB2 const& particleStartVelocity,
	FloatRange const& particleLifeTime, Rgba8 const& particleStartColor, Particle2DShape particleShape, bool beginActive = true,
	FloatRange const& particleStartOrientation = FloatRange( 0.f, 0.f ), FloatRange const& particleStartAngularSpeed = FloatRange( 0.f, 0.f ),
	Texture* particleTexture = nullptr, Rgba8 const& particleEndColor = EQUAL_TO_START_COLOR, float particleGravityDrag = 0.f, float particleAirDrag = 0.f );
bool ParticleSystem2DIsEmitterActive( ParticleEmitter2D_UID const uid );
void ParticleSystem2DRestartEmitter( ParticleEmitter2D_UID const uid );
void ParticleSystem2DSetEmitterActive( ParticleEmitter2D_UID const uid, bool active );
void ParticleSystem2DSetEmitterCenter( ParticleEmitter2D_UID const uid, Vec2 const newPos );
void ParticleSystem2DDeleteEmitter( ParticleEmitter2D_UID const uid );
