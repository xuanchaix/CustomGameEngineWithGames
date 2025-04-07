#pragma once
#include "Game/GameCommon.hpp"
/*
Level and Wave system for star ship

*/
class Game;

// function pointer type define
typedef bool (*TextTrigger)(Game*);

struct Wave
{
public:
	Wave();
	Wave( const int* waveInfo );
	//Wave( int numOfAsteroids, int numOfBeetles, int numOfWasps );
	~Wave();
public:
	float m_waveEnterTime = 0.f;
	int m_numOfAsteroids = 0;
	int m_numOfBeetles = 0;
	int m_numOfWasps = 0;
	int m_numOfCarrier = 0;
	int m_numOfMotherOfCarrier = 0;
	int m_numOfDestroyer = 0;
	int m_numOfFirstExplorer = 0;
};


struct Text {
public:
	Text(int level, float lifespanTime, std::string content, float startTime=0.f, Rgba8 color = Rgba8( 192, 192, 192, 255 ), TextTrigger trigger=nullptr);
	~Text(){}
	bool m_isRendered = false;
	int m_level;
	float m_startTime;
	float m_lifespanTime;
	Rgba8 m_color;
	std::string m_content;
	TextTrigger m_trigger;
};


class Level
{
public:
	float m_timeElapsed = 0.f;
public:
	Level();
	Level( int numOfWaves, Wave** waveArray, Game* game, int numOfTexts, Text** textArray );
	~Level();

	Wave* IsWaveBegin();
	bool IsWaveFinished() const;
	void Update( float deltaTime );

private:
	Game* m_game;
	bool m_isRenderingText = false;
	Text* m_curText = nullptr;
	int m_currentWave = -1;
	int m_numOfWaves = 0;
	int m_numOfTexts = 0;
	Wave* m_waveArray[MAX_WAVES_IN_LEVEL];
	Text* m_texts[30];
};


// for smaller function, lambda is better
bool IsHealthTutorial( Game* game );