#pragma once
#include "Game/Entity.hpp"

class PlayerShip;

class SmallBacteria : public Entity {
public:
	SmallBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SmallBacteria();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void SpawnDeathEffect() const;
	//virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );

protected:
	CatmullRomSpline2D m_flagella;
	std::vector<Vec2> m_flagellaPoints;
	Timer* m_flagellaTimer = nullptr;
	bool m_flagellaState0 = true;
	PlayerShip* m_target = nullptr;
	EntityDefinition const* m_deathEntity = nullptr;
};

class TinyBacteria : public SmallBacteria {
public:
	TinyBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~TinyBacteria();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	//virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
};

class MediumBacteria : public SmallBacteria {
public:
	MediumBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MediumBacteria();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	//virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
};

class LargeBacteria : public SmallBacteria {
public:
	LargeBacteria( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~LargeBacteria();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	//virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );
};

class BacteriaMothership : public Entity {
public:
	BacteriaMothership( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaMothership();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	void ResetWonderingTarget();
protected:
	PlayerShip* m_target = nullptr;
	Timer* m_spawnTimer = nullptr;
	Timer* m_switchWonderingPosTimer = nullptr;
	EntityDefinition const* m_smallBacteria = nullptr;
	EntityDefinition const* m_deathEntity = nullptr;
	Vec2 m_wonderingPos;
};

class BacteriaSpawn : public Entity {
public:
	BacteriaSpawn( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaSpawn();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

public:
	Timer* m_spawnTimer = nullptr;
protected:
	EntityDefinition const* m_smallBacteria = nullptr;
	EntityDefinition const* m_deathEntity = nullptr;
};

class BacteriaBreeder : public BacteriaMothership {
public:
	BacteriaBreeder( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaBreeder();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
};

class BacteriaSprayer : public Entity {
public:
	BacteriaSprayer( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaSprayer();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	Timer* m_stateTimer = nullptr;
	PlayerShip* m_target = nullptr;
	float m_targetOrientation = 0.f;
	int m_state = 0;
	EntityDefinition const* m_tinyBacteria = nullptr;
	float m_basicCosmeticRadius;
	float m_basicPhysicsRadius;
};

class BacteriaFusion : public Entity {
public:
	BacteriaFusion( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaFusion();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

protected:
	Timer* m_stateTimer = nullptr;
	PlayerShip* m_target = nullptr;
	int m_state = 0;
	EntityDefinition const* m_smallBacteria = nullptr;
};

/*
	state 0: wonder
	state 1: spray to path
	state 2: spray to range
	state 3: jump to player position
	state 4: push player out of range
	0 2 0 3 4 0 1
*/
class BacteriaBossTheGreatFusion : public Entity {
public:
	BacteriaBossTheGreatFusion( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaBossTheGreatFusion();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void GoToNextState();
	virtual void RenderUI() const override;

protected:
	float m_basicPhysicsRadius;
	float m_basicCosmeticRadius;
	bool m_isLeft = false;
	bool m_isSmall = false;
	Timer* m_stateTimer = nullptr;
	Timer* m_spawnSprayerTimer = nullptr;
	PlayerShip* m_target = nullptr;
	int m_state = 0;
	EntityDefinition const* m_bacteriaFusion = nullptr;
	EntityDefinition const* m_bacteriaSprayer = nullptr;

	int m_curStateIndex = 0;
	int m_stateCount = 7;
	int m_stateSequence[7] = { 0, 2, 0, 3, 4, 0, 1 };
	float m_stateTime[5] = { 2.f, 1.2f, 1.2f, 1.2f, 1.f };

	Vec2 m_jumpTargetPos;
	Clock* m_selfClock = nullptr;
};

/*
	state 0: wonder
	state 1: go to spawn and eat it
	state 2: hide inside marsh and call breeder
	state 3: chase player
	state 4: lick player
	0 3 4 2 1
*/

class BacteriaBossMarshKing : public Entity {
public:
	BacteriaBossMarshKing( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaBossMarshKing();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void GoToNextState();
	virtual void RenderUI() const override;

protected:
	void SetSpawnToGo();
protected:
	Timer* m_stateTimer = nullptr;
	Timer* m_spawnBreederTimer = nullptr;
	Timer* m_spawnSpawnTimer = nullptr;
	PlayerShip* m_target = nullptr;
	EntityDefinition const* m_bacteriaBreeder = nullptr;
	EntityDefinition const* m_bacteriaSpawn = nullptr;
	int m_state = 0;
	int m_curStateIndex = 0;
	int m_stateCount = 6;
	int m_stateSequence[6] = { 0, 3, 4, 0, 2, 1 };
	float m_stateTime[5] = { 4.f, 5.f, 4.f, 4.f, 1.f };

	Vec2 m_wonderTargetPos;
	std::vector<Entity*> m_allSpawns;
	int m_spawnToGoIndex = -1;
	std::vector<Vertex_PCU> m_verts;
};

/*
	state 0: teleport to certain position and shoot arrow
	state 1: idle(do nothing)
	state 2: go out of map then dash to certain position 
	state 3: recover other sisters

	call mother ship
*/
// manager class of three sisters
class BacteriaBossSister;

class BacteriaBossThreeSisters : public Entity {
public:
	BacteriaBossThreeSisters( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaBossThreeSisters();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void GoToNextState();
	virtual void RenderUI() const override;

	std::vector<BacteriaBossSister*> m_sisters;
	PlayerShip* m_player = nullptr;

	Timer* m_stateTimer = nullptr;
	int m_state = 0;
	int m_curStateIndex = 0;
	int m_stateCount = 3;
	int m_stateSequence[3] = { 1, 0, 2 };
	float m_stateTime[4] = { 5.f, 5.f, 5.f, 5.f };
};

struct ProjectileDefinition;

class BacteriaBossSister : public Entity {
public:
	BacteriaBossSister( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaBossSister();

	virtual void BeginPlay();
	void StartUp();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	void GoToState( int stateIndex );
	virtual void RenderUI() const override;

	PlayerShip* m_player = nullptr;
	int m_index = 0;

	std::vector<Vertex_PCU> m_verts;
	std::vector<Vertex_PCU> m_environmentVerts;
	int m_state = 0;
	int m_curPosIndex = 1;
	Vec2 m_standPos[3];
	int m_jumpState = 0;
	Vec2 m_jumpTargetPos;

	bool m_defeated = false;
	ProjectileDefinition const& m_arrowDef;
	Timer* m_shootTimer = nullptr;
	Timer* m_changePositionTimer = nullptr;
	Timer* m_jumpTimer = nullptr;
};