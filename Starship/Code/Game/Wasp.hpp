#pragma once
#include "Game/Entity.hpp"
class PlayerShip;

class Wasp : public Entity {
public:
	Wasp( Vec2 startPos, Game* game, PlayerShip* playership );
	virtual ~Wasp() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	PlayerShip* m_targetPlayerShip;
	float shootCooldown = 5.f;
	float shootTimeCount = shootCooldown;
};