#pragma once
#include "Game/Entity.hpp"

class PlayerShip;
struct ItemDefinition;
struct ProjectileDefinition;
class PersistentRay;

class Chest :public Entity {
public:
	Chest( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Chest();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() ) override;
public:
	PlayerShip* m_player = nullptr;
	bool m_isNearPlayer = false;
	bool m_isMimic = false;
	int m_level = 0;

	SpriteSheet m_sprite;
};

// state 0: idle
// state 1: shoot coin gun
// state 2: buy mercenary
class MimicChest :public Entity {
public:
	MimicChest( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MimicChest();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;
public:
	Vec2 GetRandomOffRoomPos() const;
	PlayerShip* m_player = nullptr;
	Timer* m_stateTimer = nullptr;
	Timer* m_attackTimer = nullptr;
	Timer* m_recruitTimer = nullptr;
	int m_state = 0;
	int m_targetPosIndex = 1;
	int m_level = 0;

	Vec2 m_targetPos[4] = { Vec2( 50.f, 25.f ), Vec2( 150.f, 25.f ), Vec2( 50.f, 75.f ), Vec2( 150.f, 75.f ) };

	bool m_renderMoney = false;
	int m_numOfMoney = 0;

	SpriteSheet m_sprite;
};

enum class InteractableMachineType {
	SellHealth, SellMaxHealth, Gamble, GiveCoin, SaveMoney, RefreshShop, Recycle, ShopOwner
};

class InteractableMachine :public Entity {
public:
	InteractableMachine( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~InteractableMachine();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;

	void SpawnItem( ItemDefinition& itemDef ) const;
	void SpawnCoins( int numOfCoins ) const;
public:
	Vec2 m_startPos;
	InteractableMachineType m_type;
	PlayerShip* m_player = nullptr;
	SpriteSheet* m_sprites;
	bool m_isNearPlayer = false;

	int m_count = 0;
	bool m_isDestroyedByWeapon = true;

	Timer* m_luckinessTimer = nullptr;
};

/*
	State0: Shoot obsidian debris to any directions
	State1: Use harmer to hammer ground
	State2: Recover some health and spawn shadow enemy (only exist for a while)
	State3: Call meteor shower
	State4: Try to cage player
	State5: Get money from the player
	State6: Shoot Laser From eyes
	state7: go to one position

	state sequence:
	life 7: 0 1 3
	life 6: 0 1 2 3
	life 5: 2 0 5 3 1
	life 4: 2 0 4 5 3 1
	life 3: 4 2 6 1 0
	life 2: 4 3 2 5 1
	life 1: 4 3 2 6 1 
*/
class BossMercyKiller : public Entity {
public:
	BossMercyKiller( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BossMercyKiller();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void RenderUI() const override;
	virtual void GoToNextState();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );

protected:
	void FindANewPosition();
	void SpawnMeteorShower();
	Vec2 GetRandomPosInRoomNotNearPlayer();
protected:
	PlayerShip* m_player = nullptr;
	ProjectileDefinition const* m_obsidianDef = nullptr;
	int m_state = 0;
	Timer* m_stateTimer = nullptr;
	Timer* m_obsidianTimer = nullptr;
	Timer* m_meteorTimer = nullptr;
	Timer* m_spawnEnemyTimer = nullptr;
	Timer* m_respawnTimer = nullptr;
	Timer* m_stealMoneyTimer = nullptr;
	int m_lives = 7;
	Texture* m_texture = nullptr;
	bool m_spawnedHarmer = false;

	Vec2 m_state7Start;
	Vec2 m_state7End;
	int m_curPosIndex = 0;

	int m_curStateIndex = 0;

	float m_stateTime[8] = { 5.f, 3.5f, 8.f, 5.f, 8.f, 3.f, 3.5f, 1.5f };

	int m_stateSequenceLife7[6] = { 0, 7, 1, 7, 3, 7 };
	int m_stateSequenceLife6[8] = { 2, 7, 1, 7, 9, 7, 3 };
	int m_stateSequenceLife5[10] = { 5, 7, 0, 7, 2, 7, 3, 7, 1, 7 };
	int m_stateSequenceLife4[12] = { 1, 7, 0, 7, 4, 7, 5, 7, 3, 7, 2, 7 };
	int m_stateSequenceLife3[10] = { 4, 7, 2, 7, 6, 7, 1, 7, 0, 7 };
	int m_stateSequenceLife2[10] = { 3, 7, 4, 7, 2, 7, 5, 7, 1, 7 };
	int m_stateSequenceLife1[10] = { 6, 7, 3, 7, 2, 7, 4, 7, 1, 7 };

	int* m_stateSequence[7] = {};
	int m_stateSequenceCount[7] = { 10, 10, 10, 12, 10, 8, 6 };

	Vec2 m_standPos[4] = { Vec2( 50.f, 30.f ), Vec2( 150.f, 30.f ), Vec2( 50.f, 70.f ), Vec2( 150.f, 70.f ) };

	EntityDefinition const* m_diamondBaseDef = nullptr;
	EntityDefinition const* m_gunBaseDef = nullptr;
	EntityDefinition const* m_bacteriaBaseDef = nullptr;

	PersistentRay* m_leftRay = nullptr;
	float m_leftRayStartOrientationDegrees = 0.f;
	PersistentRay* m_rightRay = nullptr;
	float m_rightRayStartOrientationDegrees = 0.f;
};