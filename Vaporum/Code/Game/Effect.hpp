#pragma once
#include "Game/GameCommon.hpp"

class Effect {
public:
	Effect( Vec3 const& pos = Vec3(), EulerAngles const& orientation = EulerAngles() );
	virtual ~Effect();

	virtual void BeginPlay() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

	Vec3 m_position;
	EulerAngles m_orientation;
	bool m_isGarbage = false;
	EffectType m_type = EffectType::None;
};

constexpr float DamageNumberLifeTime = 3.f;
class DamageNumber : public Effect {
public:
	DamageNumber( Vec3 const& pos = Vec3(), EulerAngles const& orientation = EulerAngles() );
	virtual ~DamageNumber();

	virtual void BeginPlay();
	virtual void Update();
	virtual void Render();

	float m_timePeriod = DamageNumberLifeTime;
	int m_damageValue = 0;
};

class ConeSmokeParticle : public Effect {
public:
	ConeSmokeParticle( Vec3 const& pos = Vec3(), EulerAngles const& orientation = EulerAngles() );
	virtual ~ConeSmokeParticle();

	virtual void BeginPlay();
	virtual void Update();
	virtual void Render();

	float m_timePeriod = 0.5f;
	float m_timer = 0.5f;
	Vec3 m_direction;
	float m_speed = 1.f;
	float m_maxHalfDegrees = 10.f;
	Vec2 m_size = Vec2( 0.75f, 0.75f );
	Texture* m_texture = nullptr;
};

class SphereFireParticle : public Effect {
public:
	SphereFireParticle( Vec3 const& pos = Vec3(), EulerAngles const& orientation = EulerAngles() );
	virtual ~SphereFireParticle();

	virtual void BeginPlay();
	virtual void Update();
	virtual void Render();

	float m_timePeriod = 1.f;
	float m_timer = 1.f;
	Vec3 m_direction;
	float m_speed = 1.f;
	Vec2 m_size = Vec2( 0.75f, 0.75f );
	Texture* m_texture = nullptr;
};

class ConeMuzzleParticle : public Effect {
public:
	ConeMuzzleParticle( Vec3 const& pos = Vec3(), EulerAngles const& orientation = EulerAngles() );
	virtual ~ConeMuzzleParticle();

	virtual void BeginPlay();
	virtual void Update();
	virtual void Render();

	float m_timePeriod = 0.5f;
	float m_timer = 0.5f;
	Vec3 m_direction;
	float m_speed = 1.f;
	float m_maxHalfDegrees = 10.f;
	Vec2 m_size = Vec2( 0.75f, 0.75f );
	Texture* m_texture = nullptr;
};

class Rocket : public Effect {
public:
	Rocket( Vec3 const& pos = Vec3(), EulerAngles const& orientation = EulerAngles() );
	virtual ~Rocket();

	virtual void BeginPlay();
	virtual void Update();
	virtual void Render();
	Vec3 GetPosByTimeRatio( float ratio ) const;
	Mat44 GetModelMatrix() const;

	Timer* m_smokeParticleTimer = nullptr;
	Vec3 m_startPos;
	Vec3 m_endPos;
	Vec3 m_pivotPos;
	float m_timePeriod = 1.5f;
	float m_timer = 1.5f;
};