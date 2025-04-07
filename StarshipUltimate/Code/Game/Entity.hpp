#pragma once
#include "Game/GameCommon.hpp"
class Game;
class Controller;
class Weapon;
class Projectile;
/*
		<Basic name="DiamondWarrior" faction="DIAMOND" physicsRadius="1.5" cosmeticRadius="1.5" turnSpeed="180.0" maxHealth="4" killReward="2" dealDamageOnCollide="true"/>
		<Weapons>

		</Weapons>
		<AI isEnabled="true" aiBehivior="DiamondWarrior"/>
*/
struct EntityDefinition {
	std::string m_name;
	std::string m_faction;
	float m_physicsRadius = 0.5f;
	float m_cosmeticRadius = 0.5f;
	float m_turnSpeed = 90.f;
	float m_flySpeed = 250.f;
	float m_maxHealth = 4.f;
	int m_killReward = 2;
	bool m_dealDamageOnCollide = false;
	bool m_isReflector = false;
	bool m_isEnemy = false;
	int m_enemyLevel = 0;
	Rgba8 m_deathParticleColor = Rgba8( 192, 192, 192 );

	// physics
	bool m_enableCollision = true;

	// render
	std::string m_texturePath;

	// weapon
	std::string m_weaponType = "None";
	float m_shootCoolDown = 1.f;
	float m_weaponDamage = 1.f;

	// AI
	bool m_isAIEnabled = false;
	std::string m_aiBehavior = "Default";
	bool m_isShielded = false;

	EntityDefinition();
	EntityDefinition( XmlElement* xmlIter );
	static void SetUpEntityDefinitions();
	static std::vector<EntityDefinition> s_definitions;
	static std::map<std::string, std::vector<std::vector<EntityDefinition*>>> s_factionLevelMap;
	static EntityDefinition const& GetDefinition( std::string const& name );
	/// if level == -1, get random level
	static EntityDefinition const& GetRandomDefinition( std::string const& faction, int level = -1 );
};

class Entity {
public:
	Entity( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~Entity();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const = 0;
	virtual void Die() = 0;
	void DebugRender() const;
	virtual void RenderUI() const;

	Vec2 GetForwardNormal() const;
	bool IsAlive() const;
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity=Vec2() );
	virtual void BeAttackedOnce( float hit, Vec2 const& hitNormal, float coolDownSeconds, void* damageSource, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() );

	void AddForce( Vec2 const& force, bool isAffectedByMass = true );
	void AddImpulse( Vec2 const& impulse, bool isAffectedByMass = false );

	void SetOrientationDegrees( float newOrientation );
	virtual bool Fire( Vec2 const& forwardVec, Vec2 const& startPos, bool forceToFire = false );
	virtual float GetMainWeaponDamage() const;

	virtual bool IsInvincible();

protected:
	virtual void UpdatePhysics( float deltaSeconds );
	Mat44 GetModelMatrix() const;
	void SpawnReward() const;

public:
	Clock* m_clock = nullptr;
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
	EntityDefinition const& m_def;
	Rgba8 m_color;
	float m_mass = 1.f;

	Controller* m_controller = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;

	Timer* m_mainWeaponTimer = nullptr;
	Weapon* m_mainWeapon = nullptr;

	std::map<void*, float> m_damageSourceMap;

	bool m_disableFriction = false;
	bool m_restrictIntoRoom = true;

	bool m_hasReward = true;
	bool m_isInvincible = false;
//	Room* m_room = nullptr;

	bool m_isPoisoned = false;
	float m_poisonTimer = 0.f;
	bool m_immuneToPoison = false;

	bool m_isSlowed = false;
	float m_slowTimer = 0.f;
	bool m_immuneToSlow = false;

	bool m_isShadowed = false;
	Timer* m_shadowLifeTimer = nullptr;
};