#include "Game/Wave.hpp"
#include "Game/Game.hpp"

Wave::Wave()
{

}

Wave::Wave( const int* waveInfo )
	: m_waveEnterTime( (float)waveInfo[0] )
	, m_numOfAsteroids( waveInfo[1] )
	, m_numOfBeetles( waveInfo[2] )
	, m_numOfWasps( waveInfo[3] )
	, m_numOfFirstExplorer( waveInfo[4] )
	, m_numOfDestroyer( waveInfo[5] )
	, m_numOfCarrier(waveInfo[6])
	, m_numOfMotherOfCarrier(waveInfo[7])
{

}

/*
Wave::Wave( int numOfAsteroids, int numOfBeetles, int numOfWasps )
	:m_numOfAsteroids(numOfAsteroids)
	,m_numOfBeetles(numOfBeetles)
	,m_numOfWasps(numOfWasps)
{

}
*/

Wave::~Wave()
{

}

Level::Level()
{
	for (int i = 0; i < MAX_WAVES_IN_LEVEL; i++) {
		m_waveArray[i] = nullptr;
	}
	for (int i = 0; i < 30; i++) {
		m_texts[i] = nullptr;
	}
	m_game = nullptr;
}

Level::Level( int numOfWaves, Wave** waveArray, Game* game, int numOfTexts, Text** inputTextArray )
{
	for (int i = 0; i < numOfWaves; i++) {
		m_waveArray[i] = waveArray[i];
	}
	m_numOfWaves = numOfWaves;
	for (int i = 0; i < numOfTexts; i++) {
		m_texts[i] = inputTextArray[i];
	}
	m_numOfTexts = numOfTexts;
	m_game = game;
}

Level::~Level()
{
	for (int i = 0; i < m_numOfWaves; i++) {
		if (m_waveArray[i]) {
			delete m_waveArray[i];
			m_waveArray[i] = 0;
		}
	}
	for (int i = 0; i < m_numOfTexts; i++) {
		if (m_texts[i]) {
			delete m_texts[i];
			m_texts[i] = 0;
		}
	}
}

Wave* Level::IsWaveBegin()
{
	if (IsWaveFinished()) {
		return nullptr;
	}
	// m_currentWave + 1 < m_numOfWaves
	float timeSum = 0.f;
	for (int i = 0; i <= m_currentWave + 1; i++) {
		timeSum += m_waveArray[i]->m_waveEnterTime;
	}
	if (timeSum <= m_timeElapsed) {
		m_currentWave++;
		return m_waveArray[m_currentWave];
	}
	return nullptr;
}

bool Level::IsWaveFinished() const
{
	return m_numOfWaves == m_currentWave + 1;
}

void Level::Update( float deltaTime )
{
	m_timeElapsed += deltaTime;
	if (!m_isRenderingText) {
		for (int i = 0; i < m_numOfTexts; i++) {
			if (m_texts[i] && !(m_texts[i]->m_isRendered)) {
				if (m_texts[i]->m_trigger) {
					if (m_texts[i]->m_trigger( m_game )) {
						m_texts[i]->m_startTime = m_timeElapsed;
						m_isRenderingText = true;
						m_texts[i]->m_isRendered = true;
						m_curText = m_texts[i];
						m_game->RegisterRenderText( m_texts[i] );
					}
				}
				else if (m_timeElapsed >= m_texts[i]->m_startTime) {
					m_isRenderingText = true;
					m_texts[i]->m_isRendered = true;
					m_curText = m_texts[i];
					m_game->RegisterRenderText( m_texts[i] );
				}
			}
		}
	}
	else {
		if (m_curText && m_timeElapsed - m_curText->m_startTime >= m_curText->m_lifespanTime) {
			m_isRenderingText = false;
			m_curText = nullptr;
			m_game->StopRenderText();
		}
		
	}
	
}

Text::Text( int level, float lifespanTime, std::string content, float startTime/*=0.f*/, Rgba8 color, TextTrigger trigger/*=nullptr*/ )
	:m_level(level),
	m_lifespanTime(lifespanTime),
	m_content(content),
	m_startTime(startTime),
	m_trigger(trigger),
	m_color(color)
{

}

bool IsHealthTutorial( Game* game )
{
	if (!game->m_healthTutorialCompleted) {
		if (game->m_healthTutorial) {
			game->m_healthTutorialCompleted = true;
		}
		return game->m_healthTutorial;
	}
	else {
		return false;
	}
}
