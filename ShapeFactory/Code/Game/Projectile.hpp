#pragma once
#include "Game/Entity.hpp"

class ProjectileDefinition {
public:
	std::string m_name;
	int m_id = -1;
	int m_productID = -1;
	bool m_isPiercing = false;
	bool m_isParalyzing = false;
	bool m_isBurning = false;
	bool m_isGuided = false;
	bool m_isSplashing = false;
	Texture* m_texture = nullptr;
	float m_damage = 1.f;
	float m_lifeSpan = 0.5f;
	float m_speed = 30.f;
	int m_projectileCount = 10;
	Vec2 m_bounds = Vec2();
	Rgba8 m_color = Rgba8::WHITE;
	ProjectileDefinition();
	ProjectileDefinition( XmlElement* xmlIter );
	static void SetUpProjectileDefinitions();
	static std::vector<ProjectileDefinition> s_definitions;
	static ProjectileDefinition const& GetDefinition( std::string const& name );
	static ProjectileDefinition const& GetDefinition( int id );
};

class Projectile : public Entity {
public:
	Projectile( Vec2 const& pos, ProjectileDefinition const& def );
	virtual ~Projectile();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

	float m_lifeTime = 0.f;
	ProjectileDefinition const& m_def;
};