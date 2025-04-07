#pragma once
#include "Game/Entity.hpp"
#include "Game/Projectile.hpp"

class PersistentRay;

class DiamondWarrior : public Entity
{
public:
	DiamondWarrior( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondWarrior();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	int m_state = 0;
};

class DiamondRayShooter : public Entity
{
public:
	DiamondRayShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondRayShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	Timer* m_shootTimer = nullptr;
	int m_state = 0;
};

class DiamondStriker : public Entity
{
public:
	DiamondStriker( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondStriker();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
protected:
	void ResetRandomWonderingPos();
	void BounceOffEdges( AABB2 const& edges );

public:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	int m_state = 0;
	Vec2 m_wonderingPos;
};

class DiamondReflector : public Entity
{
public:
	DiamondReflector( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondReflector();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

	void TriggerShootRays( Vec2 const& dirction );
protected:
	void ShootRay( Vec2 const& forwardVec );
protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	Timer* m_reflectionTimer = nullptr;
	int m_state = 0;
	Vec2 m_wonderingPos;
	int m_numOfRays = 6;
	float m_targetDegrees;
	Vec2 m_reflectionDirection;
	int m_numOfReflections = 3;
	bool m_isTriggeredByReflection = false;
};

class DiamondDoubleRayShooter : public Entity
{
public:
	DiamondDoubleRayShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondDoubleRayShooter();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	virtual void Shoot( float displacement );

protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer1 = nullptr;
	Timer* m_stateTimer2 = nullptr;
	Timer* m_shootTimer1 = nullptr;
	Timer* m_shootTimer2 = nullptr;
	int m_state1 = 0;
	int m_state2 = 0;
	float m_displacement = 1.2f;
};

class DiamondMiner : public Entity
{
public:
	DiamondMiner( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondMiner();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	virtual void ThrowBomb( Vec2 const& position, float flyTime );
	bool CollideWithEdges();
protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	Vec2 m_wonderingPos = Vec2();
	int m_state = 0;
	int m_bombCount = 5;
	bool m_isRushing = false;
};



/*
	Generate reflection pillars
	Stage2:
	Stage1:
		//throwing bombs
		dash to player
		//Generating Shield
		//Shooting lots of rays
		Turning and bouncing
		hang on the wall and shoot
		dodge attack and flash to attack
		dash to dodge
		//call diamond allies

		state 0: dash to player
		state 1: turning and bouncing - and call striker
		state 2: hang on the wall and shoot
		state 3: dodge attack and flash to attack
		state 4: begin state
		//trick 0: dash to dodge
*/
class DiamondBossAssassin : public Entity {
public:
	DiamondBossAssassin( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondBossAssassin();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;

protected:
	virtual void GotoState( int stateToGo );
	//virtual void Shoot( float displacement );
	void BounceOffEdges( AABB2 const& edges );

protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	int m_state = 4;
	Timer* m_dodgeCoolDownTimer = nullptr;
	Timer* m_shurikenShootTimer = nullptr;
	CubicHermiteCurve2D m_enterSpline;
	float m_enterCurveLength = 0.f;
	ProjectileDefinition const& m_projDef;
	Vec2 m_hangingPos = Vec2(0.f, 0.f);
	bool m_isImpulseAdded = false;
	bool m_hasDoneFlash = false;
	bool m_readyToDash = false;
};

// 0: go out and throw missile
// 1: call miner
class DiamondBossSuperMiner : public Entity {
public:
	DiamondBossSuperMiner( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondBossSuperMiner();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;


protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	int m_state = 0;

	Vec2 m_north1Arch = Vec2( 50.f, 91.5f );
	Vec2 m_north2Arch = Vec2( 150.f, 91.5f );
	Vec2 m_south1Arch = Vec2( 50.f, 8.5f );
	Vec2 m_south2Arch = Vec2( 150.f, 8.5f );
	Vec2 m_west1Arch = Vec2( 8.5f, 75.f );
	Vec2 m_west2Arch = Vec2( 8.5f, 25.f );
	Vec2 m_east1Arch = Vec2( 191.5f, 75.f );
	Vec2 m_east2Arch = Vec2( 191.5f, 25.f );
	float m_archHeight = 5.f;
	float m_archRadius = 3.5f;

	float m_timePassPath = 3.f;
	Timer* m_pathTimer = nullptr;
	Timer* m_shootTimer = nullptr;
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
	int m_curPath = 0;
	Vec2 m_startPos[8];
	Vec2 m_endPos[8];
	int m_callAlly[8];
	Timer* m_allyTimer = nullptr;

	Timer* m_enhanceTimer = nullptr;
	bool m_enhanced = false;
	float m_pathTimerModifier = 1.f;

	EntityDefinition const& m_minerDef;
};

/*
	state 0: shoot rays / shoot one ray
	state 1: draw rays / draw rays
	state 2: None / dash and shoot shuriken (only small)

	dodge; call reflector
*/
class DiamondBossRayChannel : public Entity {
public:
	DiamondBossRayChannel( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiamondBossRayChannel();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;

protected:
	void ReArrangeRayPos();
protected:
	Entity* m_target = nullptr;
	EntityDefinition const& m_reflectorDef;
	ProjectileDefinition const& m_projDef;
	Timer* m_stateTimer = nullptr;
	Timer* m_selectTimer = nullptr;
	Timer* m_shootTimer = nullptr;
	Timer* m_dodgeTimer = nullptr;
	int m_state = 0;

	Vec2 m_rayPos[8];
	int m_numSelected = 0;

	PersistentRay* m_rays[6];

	bool m_ccw = true;
	bool m_isSmall = false;

	float m_orientationToTarget = 0.f;
};