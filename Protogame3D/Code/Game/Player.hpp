#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"

class Player : public Entity {
public:
	Player( Game* game );
	virtual ~Player();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	//virtual void RenderUI() const override;

	//virtual void BeAttacked( int hit );
public:
	Camera m_camera;
	int m_playerId = 0;
};