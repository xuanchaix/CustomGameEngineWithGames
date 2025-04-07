#pragma once
#include "Game/Entity.hpp"

enum class EnemyState {
	Running, Attacking, Dying, Idle,
};

class EnemyDefinition {
public:
	SpriteSheet m_runSprite0;
	SpriteSheet m_runSprite1;
	SpriteSheet m_runSprite2;
	SpriteSheet m_runSprite3;
	SpriteSheet m_attackSprite0;
	SpriteSheet m_attackSprite1;
	SpriteSheet m_attackSprite2;
	SpriteSheet m_attackSprite3;
	SpriteSheet m_dieSprite0;
	SpriteSheet m_dieSprite1;
	SpriteSheet m_dieSprite2;
	SpriteSheet m_dieSprite3;
	std::string m_name;
	int m_id = -1;
	float m_damage = 1.f;
	float m_maxHealth = 2.f;
	float m_movingSpeed = 2.f;
	float m_turnSpeed = 180.f;
	float m_attackCooldown = 1.f;
	float m_range = 0.f;
	int m_AI = -1;
	Vec2 m_physicsBounds = Vec2();
	Vec2 m_visualBounds = Vec2();
	Rgba8 m_color = Rgba8::WHITE;
	EnemyDefinition( XmlElement* xmlIter );
	static void SetUpEnemyDefinitions();
	static std::vector<EnemyDefinition> s_definitions;
	static EnemyDefinition const& GetDefinition( std::string const& name );
	static EnemyDefinition const& GetDefinition( int id );
};


class EnemyBase : public Entity{
public:
	EnemyBase( Vec2 const& startPos, EnemyDefinition const& def );
	virtual ~EnemyBase();

	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

	virtual bool ChooseTarget();
	virtual void AttackTarget( Entity* target );

	EnemyDefinition const& m_def;
	Clock m_clock;
	Timer m_movementAnimTimer;
	Timer m_attackAnimTimer;
	Timer m_deathTimer;
	EnemyState m_enemyState = EnemyState::Running;

	Vec2 m_movingTargetPos;
	Entity* m_attackingTarget = nullptr;
};

