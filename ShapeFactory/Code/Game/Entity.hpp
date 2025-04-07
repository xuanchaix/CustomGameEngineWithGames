#pragma once
#include "Game/GameCommon.hpp"
class Game;

enum class EntityType { Default, Building, Projectile, Enemy, NUM };

class Entity {
public:
	Entity(Vec2 const& startPos, Game* game);
	virtual ~Entity();

	virtual void Update( float deltaTime ) = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;
	void DebugRender() const;
	virtual void RenderUI() const;
	Mat44 GetModelConstants() const;
	Mat44 GetPositionModelConstants() const;
	virtual void EntityAddVertsForHealthBar( std::vector<Vertex_PCU>& verts ) const;

	Vec2 GetForwardNormal() const;
	bool IsAlive() const;
	virtual void BeAttacked( float hit );
	AABB2 GetWorldPhysicsBounds() const;

public:
	EntityType m_entityType = EntityType::Default;
	float m_health = 1.f;
	float m_maxHealth = 1.f;
	Vec2 m_position = Vec2( 0.f, 0.f );
	float m_speed = 0.f;
	float m_orientationDegrees = 0.f;
	AABB2 m_physicsBounds = AABB2();
	AABB2 m_visualBounds = AABB2();
	bool m_isDead = false;
	bool m_isGarbage = false;
	Rgba8 m_color;
	float m_damage = 0.f;
	float m_physicsDiscRadius = 0.f;

protected:
	Game* m_game; // a pointer back to the Game instance
};