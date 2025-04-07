#pragma once
#include "Entity.hpp"
#include "Game/GameCommon.hpp"
class Map;
class Texture;

constexpr float MAX_FUEL = 5.f;

/// <summary>
/// player tank class I think in the future I need a tank class to be the parent of it
/// make it easy to refactor
/// </summary>
class PlayerTank : public Entity {
public:
	bool m_canCallReinforcements = false;
public:
	PlayerTank( Vec2 const& startPos, Map* map );
	virtual ~PlayerTank();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;
	virtual void RenderUI() const override;

	virtual void BeAttacked( float hit ) override;
	void Reborn();
	void TransferToMap( Map* map );

public:
	float m_turretAngularVelocity;
	float m_turretOrientationDegrees;

private:
	bool m_isImmortal = false;
	bool m_isBodyTurning;
	bool m_isTurretTurning;
	float m_turretGoalOrientationDegrees;
	float m_turretRelativeOrientationDegrees;
	float m_shootCoolDown;
	float m_playerSelfRepairRate = 0.5f;
	float m_fuel = MAX_FUEL;

	Texture* m_tankBaseTexture = nullptr;
	Texture* m_tankTurretTexture = nullptr;
};