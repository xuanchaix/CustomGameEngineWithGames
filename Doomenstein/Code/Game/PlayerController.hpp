#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Controller.hpp"
#include "Game/ActorUID.hpp"

class Actor;
class Game;
class Map;

enum class PlayerControlMode {
	ACTOR,
	FREE_FLY,
};

class PlayerController : public Controller {
public:
	PlayerController();
	virtual ~PlayerController();

	void Update();
	void RenderUI() const;
	virtual bool IsPlayer() const override;
	virtual bool IsAI() const override;
	virtual void Possess( ActorUID uid ) override;
	virtual void Damagedby( ActorUID uid, bool isLethal ) override;

private:
	void UpdateInput();
	void UpdateCamera();

public:
	Camera m_worldCamera;
	Camera m_uiCamera;
	int m_playerId = 0;
	PlayerControlMode m_cameraMode = PlayerControlMode::ACTOR;
	int m_controllerIndex = -1;

	int m_numOfPlayerKills = 0;
	int m_numOfDeaths = 0;

	bool m_isVictory = true;
private:
	Timer* m_hitTimer = nullptr;
};