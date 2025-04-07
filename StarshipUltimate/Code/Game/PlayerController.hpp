#pragma once
#include "Game/Controller.hpp"
#include "Game/GameCommon.hpp"
class PlayerShip;

extern char PLAYER_UP_KEYCODE;
extern char PLAYER_DOWN_KEYCODE;
extern char PLAYER_LEFT_KEYCODE;
extern char PLAYER_RIGHT_KEYCODE;
extern char PLAYER_MAIN_WEAPON_KEYCODE;
extern char PLAYER_SUB_WEAPON_KEYCODE;
extern char PLAYER_DASH_KEYCODE;
extern char PLAYER_RECOVER_KEYCODE;
extern char PLAYER_ULTIMATE_KEYCODE;
extern char PLAYER_INTERACT_KEYCODE;
extern char PLAYER_ITEM_SCREEN_KEYCODE;
extern char PLAYER_MAP_SCREEN_KEYCODE;

extern char DEBUG_KILL_ALL_KEYCODE;
extern char DEBUG_AUTO_SHOOT_MAIN_KEYCODE;
extern char DEBUG_AUTO_SHOOT_SUB_KEYCODE;

constexpr float PLAYER_BASIC_SPEED = 270.f;

class PlayerController : public Controller
{
public:
	PlayerController();
	virtual ~PlayerController();
	virtual bool IsPlayer() const override;
	virtual bool IsAI() const override;

	virtual void Possess( Entity* entity ) override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() override;
	void RenderUI();
	void GetReward( int numOfReward = 1 );

public:
	int m_reward = 50;
private:
	PlayerShip* m_playerShip;
	Camera m_UICamera;
};