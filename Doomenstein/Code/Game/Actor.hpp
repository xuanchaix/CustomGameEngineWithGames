#pragma once
#include "Game/GameCommon.hpp"
#include "Game/ActorUID.hpp"

struct WeaponDefinition;

enum class ActorFaction {
	NEUTRAL,
	ALLY,
	ENEMY,
	PROJECTILE,
	ALL,
	NONE,
	COUNT,
};

struct ActorDefinition {
	std::string m_actorName = "Default";
	// base
	bool m_visible = false;
	float m_maxHealth = 1.f;
	float m_corpseLiveSeconds = 0.f;
	ActorFaction m_faction = ActorFaction::NEUTRAL;
	bool m_canBePossessed = false;
	bool m_dieOnSpawn = false;
	Rgba8 m_tintColor = Rgba8( 255, 255, 255 );
	bool m_spawnExplosionParticle = false;
	std::string m_particleName = "Default";
	bool m_updatePositionEvenDie = false;

	// collision
	float m_physicsRadius = 0.f;
	float m_physicsHeight = 0.f;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;
	bool m_dieOnCollide = false;
	FloatRange m_damageOnColiide = FloatRange( 0.f, 0.f );
	float m_impulseOnCollide = 0.f;
	// physics
	bool m_simulated = false;
	bool m_canFly = false;
	float m_walkSpeed = 0.f;
	float m_runSpeed = 0.f;
	float m_drag = 0.f;
	float m_turnSpeedDegrees = 0.f;
	// camera
	float m_eyeHeight = 0.f;
	float m_cameraFOVDegrees = 60.f;
	// visual
	Vec2 m_visualSize = Vec2( 1.f, 1.f );
	Vec2 m_pivot = Vec2( 0.5f, 0.5f );
	BillboardType m_billboardType = BillboardType::NONE;
	bool m_renderLit = true;
	bool m_renderRounded = true;
	std::string m_shader = "Default";
	std::string m_spriteSheet = "Default";
	IntVec2 m_cellCount = IntVec2( 1, 1 );
	SpriteSheet* m_animSprite;
	std::vector<SpriteAnimGroupDefinition> m_animGroup;
	bool m_blendAdditive = false;

	// sounds
	std::map<std::string, std::string> m_sounds;

	// AI
	bool m_AIEnabled = false;
	float m_sightRadius = 0.f;
	float m_sightAngle = 0.f;
	std::string m_aiBehavior = "Default";
	// weapon
	std::vector<WeaponDefinition const*> m_weaponNames;

	ActorDefinition();
	ActorDefinition( XmlElement* xmlIter );
	static ActorDefinition const& GetActorDefinition( std::string const& name );
	static std::vector<ActorDefinition> s_definitions;

	static void SetUpActorDefinitions();
	~ActorDefinition();
	SpriteAnimGroupDefinition const& GetGroupAnimDef( std::string name ) const;
	SpriteDefinition const& GetAnimSprite( std::string const& curAnimState, Vec3 const dir, float seconds ) const;
	SpriteDefinition const& GetDefultAnimSprite( Vec3 const viewDirNormal ) const;
};

class Map;
class Controller;
class Weapon;

class Actor {
public:
	Actor( Map* map, ActorDefinition const& def );
	virtual ~Actor();

	virtual void BeginPlay();
	virtual void Update();
	virtual void Render( Camera const* renderCamera, bool isShadow=false ) const;
	Mat44 const GetModelMatrix() const;

	//virtual void RenderUI() const override;
	//void DebugRender() const;

	void BeAttacked( float hit, bool& isLethal );
	void Die();

	Vec3 GetForwardNormal() const;
	bool IsAlive() const;

	void UpdatePhysics();
	void AddForce( Vec3 const& force );
	void AddImpulse( Vec3 const& impulse );
	//void OnCollide();
	void OnPossessed();
	void OnUnpossessed();
	void MoveInDirection( Vec3 const& movement );
	void TurnInDirection( EulerAngles const& turnDegrees );
	void SetOrientation( EulerAngles const& orientation );
	void SetYawDegrees( float yawDegrees );
	void SetAnimationState( std::string const& newState );
	void SetAnimationStateToDefault();
	void Attack();
	void EquipWeapon( int index );
	void EquipNextWeapon();
	void EquipPrevWeapon();

public:
	ActorDefinition const& m_def;
	Map* m_map = nullptr;
	Vec3 m_position;
	EulerAngles m_orientation;
	Vec3 m_velocity = Vec3( 0, 0, 0 );
	Vec3 m_acceleration = Vec3( 0, 0, 0 );
	Rgba8 m_color = Rgba8( 255, 0, 0 );

	float m_height = 0.75f;
	ActorUID m_uid;
	float m_physicsRadius = 0.35f;
	float m_cosmeticRadius;
	float m_health;
	float m_maxHealth;
	bool m_isDead = false;
	bool m_isGarbage = false;

	float m_destroyTime = 0.1f;
	Timer* m_destroyTimer = nullptr;
	Timer* m_animationTimer = nullptr;
	Clock* m_animationClock = nullptr;
	std::string m_animationState = "Default";

	Controller* m_controller = nullptr;
	Controller* m_AIController = nullptr;

	ActorUID m_owner;	// for projectile

	std::vector<Weapon*> m_weapons;
	Weapon* m_curWeapon = nullptr;
};