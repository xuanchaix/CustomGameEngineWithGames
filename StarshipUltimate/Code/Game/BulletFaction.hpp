#pragma once
#include "Game/Entity.hpp"

struct ProjectileDefinition;

class GunShooter : public Entity {
public:
	GunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~GunShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity=Vec2() );
	virtual void GunFire();

protected:
	Entity* m_target = nullptr;
	float m_gunOrientationDegrees = 0.f;
	Timer* m_stateTimer = nullptr;
	int m_state = 0;
	Timer* m_deathTimer = nullptr;

	float m_targetOrientation = 0.f;
	float m_targetRelativeOrientation = 0.f;

	ProjectileDefinition const& m_projDef;
};

class MachineGunShooter : public GunShooter {
public:
	MachineGunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MachineGunShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();

protected:
	bool m_isFiring = false;
	Timer* m_machineGunCoolDownTimer = nullptr;
};

class SectorGunShooter : public GunShooter {
public:
	SectorGunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SectorGunShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();

protected:
};

class ShotGunShooter : public GunShooter {
public:
	ShotGunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~ShotGunShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();

protected:
};

class Sniper : public GunShooter {
public:
	Sniper( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Sniper();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();

protected:
	Timer* m_redTimer = nullptr;

	float m_count = 0.04f;
	float m_step = 0.04f;
	Rgba8 m_rayColor;
};

class MissileShooter : public GunShooter {
public:
	MissileShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MissileShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();

protected:
	bool m_goUp = true;
};

/*
state 0: wonder
state 1: wonder + shoot machine gun
state 2: slide and shoot sector bullet
state 3: jump to another place
(only brother dies)state 4: throw grenade
brother not die 0-1-0-2-0-3-0-1...
brother die 1-2-1-3-4-1-2-0-3-4...
*/
class GunTwinYoung;
class GunTwinElder : public GunShooter {
public:
	GunTwinElder( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~GunTwinElder();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();
	virtual void GoToNextState();
	virtual void RenderUI() const override;

public:
	Clock* m_selfClock = nullptr;
protected:
	Timer* m_grenadeTimer = nullptr;
	GunTwinYoung* m_brother = nullptr;
	ProjectileDefinition const& m_missileDef;
	bool m_goUp = true;
	bool m_isAngry = false;

	int m_curStateIndex = 0;
	int m_normalStateCount = 6;
	int m_angryStateCount = 5;
	float m_state0Time = 2.f;
	float m_state1Time = 4.f;
	float m_state2Time = 0.5f;
	float m_state3Time = 2.5f;
	float m_state4Time = 4.f;
	int m_stateSequenceNormal[6] = { 0, 1, 0, 2, 1, 3 };
	int m_stateSequenceAngry[5] = { 1, 2, 1, 3, 4 };

	Vec2 m_jumpTargetPos;
};

/*
state 0: wonder
state 1: wonder + shoot machine gun
state 2: throw grenade
state 3: jump to another place
(only brother dies)state 4: slide and shoot sector bullet
brother not die 0-1-0-2-0-3-0-1...
brother die 1-4-1-3-2-1-4-0-3-2...
*/
class GunTwinYoung : public GunShooter {
public:
	GunTwinYoung( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~GunTwinYoung();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire();
	virtual void GoToNextState();
	virtual void RenderUI() const override;

public:
	GunTwinElder* m_brother;
	Clock* m_selfClock = nullptr;
protected:
	ProjectileDefinition const& m_missileDef;
	Timer* m_grenadeTimer = nullptr;
	bool m_goUp = true;
	bool m_isAngry = false;

	int m_curStateIndex = 2;
	int m_normalStateCount = 6;
	int m_angryStateCount = 5;
	float m_state0Time = 2.f;
	float m_state1Time = 4.f;
	float m_state2Time = 4.f;
	float m_state3Time = 2.5f;
	float m_state4Time = 0.5f;
	int m_stateSequenceNormal[6] = { 0, 1, 0, 2, 1, 3 };
	int m_stateSequenceAngry[5] = { 1, 4, 1, 3, 2 };

	Vec2 m_jumpTargetPos;
};

class GunAudience : public GunShooter {
public:
	GunAudience( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~GunAudience();

	//virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	//virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	//virtual void GunFire();
protected:
	bool m_isAngry = false;
	AABB2 m_bounds;
};

/*
	state 0: wonder
	state 1: teleport and shoot
	state 2: dash and shoot
	state 3: dodge and shoot

	shoot skill1: machine gun
	shoot skill2: snipe
	shoot skill3: sector attack
	shoot skill4: 

	0 1 0 2 0 3 0 1...
*/

class BossDoubleGun : public GunShooter {
public:
	BossDoubleGun( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BossDoubleGun();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
	virtual void GunFire( int skill );
	virtual void GoToNextState();
	virtual void RenderUI() const override;

protected:
	void ReducePopularity( float count );
protected:
	float m_lastFramePlayerHealth = 0.f;
	bool m_doNotHitPlayer = true;
	float m_popularity = 1200;
	std::vector<Entity*> m_audience;
	int m_curStateIndex = 0;
	int m_stateCount = 6;
	int m_stateSequence[6] = { 0, 1, 0, 2, 0, 3 };
	float m_stateTime[4] = { 6.f, 2.f, 0.5f, 0.6f };
	Timer* m_renderPopularityTimer = nullptr;
	int m_popularityToRender = 0;
	Timer* m_machineGunTimer = nullptr;
};

class BulletGun;
class RayShooter;
class Machete;
class RocketShooter;
class EnemyBulletGun;
class MissileGun;
class Spray;
class CoinGun;

class BossArmsMaster : public Entity {
public:
	BossArmsMaster( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BossArmsMaster();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );

	virtual void RenderUI() const override;

protected:
	void PerformWonder( float deltaTime );
	virtual void GoToNextWeaponMode();
protected:
	int m_curWeaponMode = 1;
	int m_rndNextNum = 0;
	bool m_isWondering = true;

	Entity* m_target = nullptr;
	float m_targetOrientation;
	float m_targetRelativeOrientation;

	Timer* m_weaponTimer = nullptr;
	Timer* m_deathTimer = nullptr;
	Timer* m_shootTimer = nullptr;

	Rgba8 m_faceColor = Rgba8( 255, 153, 51 );

	BulletGun* m_playerBulletGun = nullptr;
	RayShooter* m_rayShooter = nullptr;
	Machete* m_machete = nullptr;
	RocketShooter* m_rocketShooter = nullptr;
	EnemyBulletGun* m_enemyBulletGun = nullptr;
	MissileGun* m_missileGun = nullptr;
	Spray* m_sprayWeapon = nullptr;
	CoinGun* m_coinGun = nullptr;

	int m_rayShooterCounter = 0;
	bool m_rocketDodge = false;
	bool m_dodgeLeft = false;
	bool m_missileGoUp = false;
	bool m_isSprayDash = true;
	bool m_macheteGoLeft = true;
	bool m_macheteFromTop = false;
	int m_macheteState = 0;
	Vec2 m_macheteStartPos;
	SpriteSheet m_weaponSprites;
};