#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


enum class PowerUpType { enhance, health, ammo, friendShip, NUM };

class PowerUp : public Entity
{
public:
	PowerUp( Vec2 startPos, Game* game, PowerUpType type, Entity const* entity );
	virtual ~PowerUp() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
public:
	PowerUpType m_powerUpType;
private:
	float m_lifespan = POWER_UP_LIFETIME_SECONDS;
	int NUM_OF_POWER_UP_VERTS[(int)PowerUpType::NUM] = { 21, 12, 6, 6 };
	Vertex_PCU m_powerUpVerts[48];
};