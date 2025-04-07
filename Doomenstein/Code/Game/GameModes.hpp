#pragma once
#include "Game/Actor.hpp"

class GameMode : public Actor {
public:
	GameMode( Map* map );
	virtual ~GameMode();

	virtual void BeginPlay();
	virtual void Update();
public:

	static const ActorDefinition s_gameModeDef;

protected:

};

class SurvivalGameMode : public GameMode {
public:
	SurvivalGameMode( Map* map );
	virtual ~SurvivalGameMode();

	virtual void BeginPlay();
	virtual void Update();

	int m_playerDeath = 0;
protected:
	virtual void StartNewWave();
	int m_curWave = 0;
	float m_nextWaveCountDownTimer = 0.f;
	float m_waveTimer = 0.f;
	int m_curNumOfEnemySpawned = 0;
};

class KillAllGameMode : public GameMode {
public:
	KillAllGameMode( Map* map );
	virtual ~KillAllGameMode();

	virtual void BeginPlay();
	virtual void Update();
};

class KillOpponentGameMode : public GameMode {
public:
	KillOpponentGameMode( Map* map );
	virtual ~KillOpponentGameMode();

	virtual void BeginPlay();
	virtual void Update();
};