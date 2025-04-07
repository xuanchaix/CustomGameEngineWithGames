#pragma once
#include "Game/GameCommon.hpp"
class Game;

enum class EntityType { DEFAULT, NUM };

class Entity {
public:
	Entity( Game* game );
	virtual ~Entity();

	virtual void Update() = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;
	void DebugRender() const;
	virtual void RenderUI() const;

	Vec3 GetForwardNormal() const;
	bool IsAlive() const;
	virtual void BeAttacked( int hit );

	Mat44 GetModelMatrix() const;

public:
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation;
	//Vec2 m_accelerateVelocity;
	EulerAngles m_angularVelocity; // the Entity’s signed angular velocity( spin rate ), in degrees per second
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