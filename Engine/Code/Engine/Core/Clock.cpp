#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"

Clock* Clock::s_systemClock = new Clock();

Clock::Clock()
{
	if (s_systemClock != nullptr) {
		s_systemClock->AddChild( this );
		m_parent = s_systemClock;
	}
}

Clock::Clock( Clock& parent )
{
	parent.AddChild( this );
	m_parent = &parent;
}

Clock::~Clock()
{
	if (m_parent) {
		m_parent->RemoveChild( this );
	}
	// ToDo: tell all children that this clock is not parent
	// but now it seems no need to do so
}

void Clock::Reset()
{
	m_totalSeconds = 0.f;
	m_deltaSeconds = 0.f;
	m_frameCount = 0;
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::UnPause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_isPaused = false;
	m_stepSingleFrame = true;
}

void Clock::SetTimeScale( float timeScale )
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock* Clock::GetSystemClock()
{
	return s_systemClock;
}

void Clock::TickSystemClock()
{
	s_systemClock->Tick();
}

void Clock::Tick()
{
	float curTime = (float)GetCurrentTimeSeconds();
	float deltaSeconds = curTime - m_lastUpdateTimeInSeconds;
	m_lastUpdateTimeInSeconds = curTime;
	deltaSeconds = GetClamped( deltaSeconds, 0.0f, m_maxDeltaSeconds );
	Advance( deltaSeconds );
}

void Clock::Advance( float deltaTimeSeconds )
{
	if (m_isPaused) {
		deltaTimeSeconds = 0.f;
	}
	deltaTimeSeconds *= m_timeScale;
	m_deltaSeconds = deltaTimeSeconds;
	m_totalSeconds += m_deltaSeconds;
	m_frameCount += 1;
	if (m_stepSingleFrame) {
		m_isPaused = true;
		m_stepSingleFrame = false;
	}
	for (auto clock : m_children) {
		clock->Advance( deltaTimeSeconds );
	}
}

void Clock::AddChild( Clock* childClock )
{
	m_children.push_back( childClock );
}

void Clock::RemoveChild( Clock* childClock )
{
	for (int i = 0; i < (int)m_children.size(); i++) {
		if (m_children[i] == childClock) {
			m_children.erase( m_children.begin() + i );
			return;
		}
	}
}

