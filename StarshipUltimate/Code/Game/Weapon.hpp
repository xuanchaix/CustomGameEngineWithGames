#pragma once
#include "Game/Projectile.hpp"
#include "Game/GameCommon.hpp"

struct WeaponDefinition {
	float m_shootCoolDown = 0.01f;
	std::string m_name = "DEFAULT";
	std::string m_behavior = "DEFAULT";
	bool m_canTriggerReflection = false;
	ProjectileDefinition const* m_projectileDef = nullptr;

	WeaponDefinition();
	WeaponDefinition( XmlElement* xmlIter );
	static void SetUpWeaponDefinitions();
	static std::vector<WeaponDefinition> s_definitions;
	static WeaponDefinition const& GetDefinition( std::string const& name );
};

class Weapon {
public:
	Weapon( WeaponDefinition const& def, Entity* owner );
	virtual ~Weapon();

	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos )=0;
	ProjectileDefinition const& GetProjectileDef() const;
protected:
	Entity* m_owner = nullptr;
	WeaponDefinition const& m_weaponDef;
};

class BulletGun : public Weapon {
public:
	BulletGun( WeaponDefinition const& def, Entity* owner );
	virtual ~BulletGun();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
};

class RayShooter : public Weapon {
public:
	RayShooter( WeaponDefinition const& def, Entity* owner );
	virtual ~RayShooter();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
};

class Machete : public Weapon {
public:
	Machete( WeaponDefinition const& def, Entity* owner );
	virtual ~Machete();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
protected:
	bool m_fromRight = true;
};

class RocketShooter : public Weapon {
public:
	RocketShooter( WeaponDefinition const& def, Entity* owner );
	virtual ~RocketShooter();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
protected:
};

class EnemyBulletGun : public Weapon {
public:
	EnemyBulletGun( WeaponDefinition const& def, Entity* owner );
	virtual ~EnemyBulletGun();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
};

class MissileGun : public Weapon {
public:
	MissileGun( WeaponDefinition const& def, Entity* owner );
	virtual ~MissileGun();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
};

class Spray : public Weapon {
public:
	Spray( WeaponDefinition const& def, Entity* owner );
	virtual ~Spray();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;

	bool m_isSector = false;
	float m_sectorRangeDegrees = 360.f;
	//float m_width;
	float m_length = 25.f;
	bool m_isPoisonous = false;
};

class CoinGun : public Weapon {
public:
	CoinGun( WeaponDefinition const& def, Entity* owner );
	virtual ~CoinGun();
	virtual Projectile* Fire( Vec2 const& forwardVec, Vec2 const& startPos ) override;
};