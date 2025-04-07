#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
class Game;

enum class EntityType { DEFAULT, NUM };

class Entity {
public:
	Entity(Vec2 startPos, Game* game);
	virtual ~Entity();

	virtual void Update( float deltaTime ) = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;
	void DebugRender() const;
	virtual void RenderUI() const;

	Vec2 GetForwardNormal() const;
	bool IsAlive() const;
	virtual void BeAttacked( int hit );


public:
	Vec2 m_position; // the Entity’s 2D( x, y ) Cartesian origin / center location, in world space
	Vec2 m_velocity; // the Entity’s linear 2D( x, y ) velocity, in world units per second
	float m_orientationDegrees; // its forward direction angle, in degrees( counter - clock.from + x / east )
	Vec2 m_accelerateVelocity;
	float m_angularVelocity; // the Entity’s signed angular velocity( spin rate ), in degrees per second
	float m_physicsRadius; // the Entity’s( inner, conservative ) disc - radius for all physics purposes
	float m_cosmeticRadius; // the Entity’s( outer, liberal ) disc - radius that encloses all of its vertexes
	int m_health; // how many “hits” the entity can sustain before dying
	int m_maxHealth;
	bool m_isDead = false; // whether the Entity is “dead” in the game; affects entityand game logic
	bool m_isGarbage = false; // whether the Entity should be deleted at the end of Game::Update()
	EntityType m_type = EntityType::DEFAULT;
	Rgba8 m_color;

protected:
	Game* m_game; // a pointer back to the Game instance
};