#pragma once
#include "Game/GameCommon.hpp"

class Room;
class PlayerShip;
struct EntityDefinition;

enum class EffectType {
	Default,
	Reward,
	Shield,
	Laser,
	Slash,
	Door,
	Missile,
	PlayerShield,
	SubWeaponLaser,
	StarshipMine,
	HealthPickup,
	ArmorPickup,
	PersistentRay,
	FloorPortal,
	BacteriaDrop,
	BacteriaSap,
	SprayAttack,
	SectorSprayAttack,
	SunFlame,
	ElectricChain,
	BacteriaLick,
	MercyKillerHarmer,
	MeteorShower,
	Meteor,
	MercyKillerCage,
	MercyKillerCageChain,
	MercyKillerRespawn,
	NUM,
};

/*
	Star ship effect class is used for anything that cannot be include into other classes, especially effects.
*/
class StarshipEffect {
public:
	StarshipEffect( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~StarshipEffect();

	virtual void BeginPlay() = 0;
	virtual void Update( float deltaTime ) = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;
	virtual void RenderUI();

	Vec2 GetForwardNormal() const;
	bool IsAlive() const;
protected:
	Mat44 GetModelMatrix() const;
public:
	Vec2 m_position;
	Vec2 m_velocity;
	float m_orientationDegrees;
	Vec2 m_accelerateVelocity;
	float m_physicsRadius;
	bool m_isDead = false;
	bool m_isGarbage = false;
	EffectType m_type;
	Rgba8 m_color;
	bool m_renderBeforeEntity = false;
protected:
	Timer* m_stateTimer = nullptr;
};


class StarshipReward : public StarshipEffect {
public:
	StarshipReward( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~StarshipReward();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

public:
	Entity* m_target = nullptr;
	bool m_gotoTarget = false;

protected:
	Vec2 m_targetPosition;
	Vec2 m_lootPosition;
	Vec2 m_startPosition;
	float m_lootRadius = 40.f;
	Entity* m_playerShip = nullptr;
	int m_state = 0;
};

class StarshipShield : public StarshipEffect {
public:
	StarshipShield( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~StarshipShield();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	float m_sectorApertureDegrees = 120.f;
	Entity* m_owner = nullptr;
protected:
	std::vector<Vertex_PCU> m_verts;
};

class RayLaser : public StarshipEffect {
public:
	RayLaser( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~RayLaser();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Entity* m_owner = nullptr;
	Vec2 m_rayEndPos;
protected:
};

class SlashEffect : public StarshipEffect {
public:
	SlashEffect( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SlashEffect();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Entity* m_owner = nullptr;
	CubicHermiteCurve2D m_curve;
protected:
	float m_length = 0.f;
	float m_curDistance = 0.f;
	float m_lastDistance = 0.f;
	Mat44 m_modelMatrix;
};

class LevelPortal : public StarshipEffect {
public:
	LevelPortal( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~LevelPortal();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Entity* m_player = nullptr;
	Room* m_owner = nullptr;
	RoomDirection m_dir = RoomDirection::CENTER;
	AABB2 m_bounds;
	bool m_isOpen = false;
	bool m_resetByEntering = true;
protected:
	bool m_isAvailableToUse = false;
};

/*
	A missile is a kind of rocket start and blasts in certain position and deal disc damage
*/
class Missle : public StarshipEffect {
public:
	Missle( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Missle();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Entity* m_owner = nullptr;
	Vec2 m_startPos;
	Vec2 m_targetPos;
	float m_flyTime = 1.f;
	float m_damage = 1.f;
	float m_maxRadius = 7.5f;
	float m_blastTime = 0.5f;
protected:
	int m_state = 0;
	float m_currentRadius = 0.f;
	float m_gravity = 300.f;
	std::string m_faction = "Default";
};

class PlayerShield : public StarshipEffect {
public:
	PlayerShield( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~PlayerShield();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Entity* m_owner = nullptr;
	float m_radius = 0.f;
	bool m_isActivated = false;
};

class StarshipLaser : public StarshipEffect {
public:
	StarshipLaser( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~StarshipLaser();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	float m_damage = 3.f;
	int m_state = 0;
	Entity* m_owner = nullptr;
	Vec2 m_rayEndPos;
	Entity* m_target = nullptr;
};

class StarshipMine : public StarshipEffect {
public:
	StarshipMine( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~StarshipMine();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	float m_blastTime = 4.f;
	float m_damage = 2.f;
	float m_explosionRadius = 20.f;
	Timer* m_redTimer = nullptr;
	float m_count = 0.05f;
	float m_step = 0.05f;
};

class HealthPickup : public StarshipEffect {
public:
	HealthPickup( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~HealthPickup();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	PlayerShip* m_playerShip;
	Room* m_room;
};

class ArmorPickup : public StarshipEffect {
public:
	ArmorPickup( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~ArmorPickup();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	PlayerShip* m_playerShip;
	Room* m_room;
};

class PersistentRay : public StarshipEffect {
public:
	PersistentRay( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~PersistentRay();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	float m_maxWidth = 0.f;
	float m_minWidth = 0.f;
	float m_maxLength = 300.f;
	float m_curLength = 0.f;
	Entity* m_owner = nullptr;
	std::string m_faction;
	float m_lifeTimeSeconds = 0.f;
	float m_width = 0.f;

	float m_damage = 1.f;
	float m_damageCoolDown = 2.f;
	bool m_updatePositionByOwner = false;
	float m_relativeOrientation = 0.f;
};

class NextFloorPortal : public StarshipEffect {
public:
	NextFloorPortal( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~NextFloorPortal();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	virtual void RenderUI() override;

public:
	Entity* m_player = nullptr;
	//Room* m_owner = nullptr;
	AABB2 m_bounds;
	bool m_isOverlap = false;
	//bool m_isOpen = false;
	//bool m_resetByEntering = true;
protected:
	//bool m_isAvailableToUse = false;
};

class BacteriaDrop : public StarshipEffect {
public:
	BacteriaDrop( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaDrop();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Vec2 m_startPos;
	Vec2 m_targetPos;
	EntityDefinition const* m_entityToSpawn = nullptr;
	float m_travelTime = 0.6f;
	Timer* m_lifeTimer = nullptr;
};

class BacteriaSap : public StarshipEffect {
public:
	BacteriaSap( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaSap();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	Vec2 m_startPos;
	Vec2 m_endPos;
	float m_capsuleRadius;
	float m_maxRadius;
	float m_numOfDisc = 10;
	float m_lifeTime = 3.f;
	Timer* m_lifeTimer = nullptr;
	std::vector<Vec2> m_centerPos;
	std::vector<float> m_discRadius;
	std::vector<unsigned char> m_a;
};

class SprayAttack : public StarshipEffect {
public:
	SprayAttack( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SprayAttack();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	float m_width = 3.f;
	float m_dist = 25.f;
	float m_damage = 1.f;
	Timer* m_attackTimer = nullptr;
	OBB2 m_boundingBox;
	Vec2 m_startPos;
	std::string m_faction;
	Timer* m_sapTimer = nullptr;
	bool m_isPoisonous = false;
};

class SectorSprayAttack : public StarshipEffect {
public:
	SectorSprayAttack( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SectorSprayAttack();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	float m_dist = 25.f;
	float m_damage = 1.f;
	float m_forwardDegrees = 0.f;
	float m_rangeDegrees = 360.f;

	float m_radius = 0.f;
	Timer* m_attackTimer = nullptr;
	Vec2 m_startPos;
	std::string m_faction;
	Timer* m_sapTimer = nullptr;
	bool m_isPoisonous = false;
};

class SunFlame : public StarshipEffect {
public:
	SunFlame( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~SunFlame();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	float m_dist = 25.f;
	float m_damage = 1.f;
	float m_radius = 0.f;
	Timer* m_attackTimer = nullptr;
	std::string m_faction;
};

class ElectricChain :public StarshipEffect {
public:
	ElectricChain( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~ElectricChain();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	int m_maxTargets = 4;
	float m_maxDist = 10.f;
	std::vector<Entity*> m_damageTargets;
	float m_damage = 1.f;
	Timer* m_lifeTimer = nullptr;
	Entity* m_owner = nullptr;
};

class BacteriaLick : public StarshipEffect {
public:
	BacteriaLick( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BacteriaLick();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	Timer* m_lifeTimer = nullptr;
	float m_length = 0.f;
	Entity* m_target = nullptr;
};

class MercyKillerHarmer : public StarshipEffect {
public:
	MercyKillerHarmer( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MercyKillerHarmer();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	Entity* m_target = nullptr;
	float m_length = 0.f;
	float m_targetOrientationDegrees = 0.f;
	float m_startOrientationDegrees = 0.f;
	Timer* m_lifeTimer = nullptr;
};

class MeteorShower : public StarshipEffect {
public:
	MeteorShower( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MeteorShower();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	Timer* m_spawnMeteorTimer = nullptr;
	Timer* m_lifeTimer = nullptr;

	float m_radius;
	float m_apertureDegrees;
};

class Meteor : public StarshipEffect {
public:
	Meteor( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Meteor();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	std::vector<Vertex_PCU> m_verts;
	Timer* m_lifeTimer = nullptr;
	PlayerShip* m_player = nullptr;
};

class MercyKillerCage : public StarshipEffect {

public:
	MercyKillerCage( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MercyKillerCage();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	Timer* m_lifeTimer = nullptr;
	Timer* m_chainTimer = nullptr;
	PlayerShip* m_player = nullptr;
	bool m_hasStartChain = false;

	AABB2 m_bounds;
};

class MercyKillerCageChain : public StarshipEffect {
public:
	MercyKillerCageChain( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MercyKillerCageChain();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	float m_width = 0.f;
	float m_height = 0.f;
	int m_direction = 0;
	Vec2 m_startPos;
	Timer* m_lifeTimer = nullptr;
	PlayerShip* m_player = nullptr;
};

class MercyKillerRespawn :public StarshipEffect {
public:
	MercyKillerRespawn( Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~MercyKillerRespawn();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

	Timer* m_lifeTimer = nullptr;
	Timer* m_particleTimer = nullptr;
	std::vector<Vec2> m_particlePos;
	std::vector<float> m_particleRadius;
};