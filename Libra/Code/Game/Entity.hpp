#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
class Map;
class Texture;
class Bullet;
class TileHeatMap;

constexpr int NUM_OF_BULLET_TYPES = 4;
enum class EntityType { _UNKNOWN = -1, _RUBBLE, _GOOD_PLAYER, _SCORPIO, _LEO, _ARIES, _CAPRICORN, _CANCER, _BULLET, _BOLT, _GUIDED_BULLET, _FLAME_BULLET, _EXPLOSION, _BUILDING, NUM };
enum class EntityFaction { FACTION_GOOD, FACTION_NEUTRAL, FACTION_EVIL, NUM };

class Entity {
public:
	Entity(Vec2 const& startPos, Map* map);
	virtual ~Entity();

	virtual void Update( float deltaTime ) = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;
	virtual void DebugRender() const;
	virtual void RenderUI() const;

	Vec2 const GetForwardNormal() const;
	bool IsAlive() const;
	bool IsBullet() const;
	bool IsActor() const;
	TileHeatMap const* GetTargetDistanceTileHeatMap() const;
	virtual void BeAttacked( float hit );
	virtual void GetAttackedByBullet( Bullet* b );


protected:
	void InitializeMovingWarrior();
	void ConductAIMovingWarrior( float deltaTime );
	void OptimizeRoute();

public:
	Vec2 m_position; // the Entity’s 2D( x, y ) Cartesian origin / center location, in world space
	Vec2 m_velocity; // the Entity’s linear 2D( x, y ) velocity, in world units per second
	float m_speed;
	float m_orientationDegrees; // its forward direction angle, in degrees( counter - clock.from + x / east )
	//Vec2 m_accelerateVelocity;
	float m_angularVelocity; // the Entity’s signed angular velocity( spin rate ), in degrees per second
	float m_physicsRadius; // the Entity’s( inner, conservative ) disc - radius for all physics purposes
	float m_cosmeticRadius; // the Entity’s( outer, liberal ) disc - radius that encloses all of its vertexes
	float m_health; // how many “hits” the entity can sustain before dying
	float m_maxHealth;
	float m_shootTime;
	bool m_isDead = false; // whether the Entity is “dead” in the game; affects entityand game logic
	bool m_isGarbage = false; // whether the Entity should be deleted at the end of Game::Update()
	bool m_isActor = false;
	bool m_isRanged = false;
	EntityType m_type = EntityType::_UNKNOWN;
	EntityFaction m_faction = EntityFaction::FACTION_NEUTRAL;
	//Rgba8 m_color;
	float m_damage = 1;
	Entity* m_target = nullptr;

	bool m_isPushedByEntities = false;
	bool m_doesPushEntities = false;
	bool m_isPushedByWalls = false;
	bool m_isHitByBullets = false;
	bool m_isAmphibious = false;
	bool m_isEffectedByRubble = false;

protected:
	Map* m_map; // a pointer back to the Map instance
	int m_curState = 1;
	float m_goalOrientationDegrees = 0.f;
	TileHeatMap* m_targetDistanceTileHeatMap = nullptr;
	Vec2 m_lastSeenPosition = Vec2(); // is m_goalPosition 
	Vec2 m_nextWayPointPosition = Vec2();
	float m_shootHalfAngleDegrees;
	float m_driveHalfAngleDegrees;
	std::vector<Vec2> m_pathPoints;
	int m_optimizedPathPointsIndex;
};