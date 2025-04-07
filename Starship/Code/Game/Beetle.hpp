#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
class PlayerShip;

class Beetle : public Entity {
public:
	Beetle( Vec2 startPos, Game* game, PlayerShip* playership );
	virtual ~Beetle() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	PlayerShip* m_targetPlayerShip;
	float shootCooldown = 5.f;
	float shootTimeCount = shootCooldown;
};