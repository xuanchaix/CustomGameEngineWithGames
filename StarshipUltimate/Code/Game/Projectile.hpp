#pragma once
#include "Game/GameCommon.hpp"

struct ProjectileDefinition {
	float m_lifeSeconds = 1.5f;
	float m_speed = 60.f;
	std::string m_name = "Bullet";
	float m_damageModifier = 1.f;
	float m_damageRange = 0.f;
	float m_rangeDamageModifier = 1.f;

	ProjectileDefinition();
	ProjectileDefinition( XmlElement* xmlIter );
	static void SetUpProjectileDefinitions();
	static std::vector<ProjectileDefinition> s_definitions;
	static ProjectileDefinition const& GetDefinition( std::string const& name );
};


class Projectile {
public:
	Projectile( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Projectile();

	virtual void BeginPlay() = 0;
	virtual void Update( float deltaTime );
	virtual void Render() const = 0;
	virtual void Die( bool dieByCollision = true ) = 0;
	virtual void SpawnCollisionEffect();
	virtual void DebugRender() const;

	Vec2 GetForwardNormal() const;
	bool IsAlive() const;

protected:
	Mat44 GetModelMatrix() const;

public:
	Vec2 m_position; // the Entity’s 2D( x, y ) Cartesian origin / center location, in world space
	Vec2 m_velocity; // the Entity’s linear 2D( x, y ) velocity, in world units per second
	float m_orientationDegrees; // its forward direction angle, in degrees( counter - clock.from + x / east )
	Vec2 m_accelerateVelocity;
	float m_angularVelocity; // the Entity’s signed angular velocity( spin rate ), in degrees per second
	float m_physicsRadius; // the Entity’s( inner, conservative ) disc - radius for all physics purposes
	float m_cosmeticRadius; // the Entity’s( outer, liberal ) disc - radius that encloses all of its vertexes
	float m_health; // how many “hits” the entity can sustain before dying
	float m_maxHealth;
	bool m_isDead = false; // whether the Entity is “dead” in the game; affects entityand game logic
	bool m_isGarbage = false; // whether the Entity should be deleted at the end of Game::Update()
	ProjectileDefinition const& m_def;
	Rgba8 m_color = Rgba8( 255, 255, 255 );
	float m_mass = 1.f;
	float m_damage = 1.f;
	float m_rangeDamage = 1.f;

	bool m_isPuncturing = false;
	bool m_hasRangeDamage = false;
	float m_rangeDamageRadius = 0.f;
	float m_rangeDamagePercentage = 0.f;
	bool m_isPoisonous = false;
	float m_poisonTime = 2.f;
	bool m_isIced = false;
	float m_slowTime = 2.f;

	float m_scale = 1.f;

	VertexBuffer* m_vertexBuffer = nullptr;
	Timer* m_lifeTimer;

	std::string m_faction;

	bool m_addDamageByLifeTime = false;
};

class Shuriken : public Projectile {
public:
	Shuriken( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Shuriken();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );
};

class Rocket : public Projectile {
public:
	Rocket( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Rocket();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );

	virtual void SpawnCollisionEffect() override;
};

class DemonBullet :public Projectile {
public:
	DemonBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DemonBullet();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );

	virtual void SpawnCollisionEffect() override;
};

class EnemyBullet : public Projectile {
public:
	EnemyBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~EnemyBullet();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );

	virtual void SpawnCollisionEffect() override;
};

class CurveMissile : public Projectile {
public:
	CurveMissile( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~CurveMissile();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );

	virtual void SpawnCollisionEffect() override;

	CubicBezierCurve2D m_curve;
	Vec2 m_targetPosition;
	bool m_goUp = false;
	Vec2 m_lastFramePos;
	float m_distance = 0.f;
};

class CoinBullet : public Projectile {
public:
	CoinBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~CoinBullet();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );

	virtual void SpawnCollisionEffect() override;
};

class SharpenedObsidian : public Projectile {
public:
	SharpenedObsidian( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SharpenedObsidian();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );
};

class Arrow : public Projectile {
public:
	Arrow( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Arrow();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );
};