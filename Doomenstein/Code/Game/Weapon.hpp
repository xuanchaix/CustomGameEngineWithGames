#pragma once
#include "Game/GameCommon.hpp"
#include "Game/ActorUID.hpp"

struct WeaponDefinition {
	std::string m_name = "Default";

	float m_refireTime = 1.f;
	int m_rayCount = 0;
	float m_rayCone = 0.f;
	float m_rayRange = 0.f;
	FloatRange m_rayDamage = FloatRange( 0.f, 0.f );
	float m_rayImpulse = 0.f;
	std::string m_rayHitWallEffectActor = "Default";
	std::string m_rayHitActorEffectActor = "Default";
	int m_projectileCount = 0;
	float m_projectileCone = 0.f;
	float m_projectileSpeed = 0.f;
	std::string m_projectileActor = "Default";
	int m_meleeCount = 0;
	float m_meleeRange = 0.f;
	float m_meleeArc = 0.f;
	FloatRange m_meleeDamage = FloatRange( 0.f, 0.f );
	float m_meleeImpulse = 0.f;
	bool m_hasRangeDamage = false;
	FloatRange m_rangeDamage = FloatRange( 0.f, 0.f );
	float m_rangeRadius = 0.f;

	// HUD
	Shader* m_HUDshader = nullptr;
	Texture* m_baseTexture = nullptr;
	Texture* m_reticleTexture = nullptr;
	Vec2 m_reticleSize = Vec2();
	Vec2 m_spriteSize = Vec2();
	Vec2 m_spritePivot = Vec2();
	Shader* m_animationShader = nullptr;
	SpriteSheet* m_animationSpriteSheet = nullptr;
	std::map<std::string, SpriteAnimDefinition> m_animations;

	// Sound
	std::map<std::string, std::string> m_sounds;

	WeaponDefinition();
	WeaponDefinition( XmlElement* xmlIter );
	static WeaponDefinition const& GetWeaponDefinition( std::string const& name );
	static std::vector<WeaponDefinition> s_definitions;

	static void SetUpWeaponDefinitions();
};

class Weapon {
public:
	Weapon( WeaponDefinition const& def, ActorUID owner );

	void Fire();
	void Update();
	SpriteDefinition const& GetCurrentSpriteDef() const;

	WeaponDefinition const& m_def;
	std::string m_curAnimationState = "Idle";
private:
	Vec3 const GetRandomDirectionInCone( EulerAngles const& orientation, float maxDegrees );

	Timer* m_shootCoolDownTimer = nullptr;
	Timer* m_animationTimer = nullptr;
	ActorUID m_owner;
};