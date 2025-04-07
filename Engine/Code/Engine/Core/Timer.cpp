#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"

Timer::Timer( float period, Clock const* clock/*= nullptr */ )
	:m_period(period), m_clock(clock)
{
	if (clock == nullptr) {
		m_clock = Clock::GetSystemClock();
	}
}

void Timer::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
	if (m_startTime == 0.f) {
		m_startTime = 0.000001f;
	}
}

void Timer::Stop()
{
	m_startTime = 0.f;
}

void Timer::Pause()
{
	if (m_pauseTime == 0.f) {
		m_pauseTime = m_clock->GetTotalSeconds();
	}
}

bool Timer::IsPaused() const
{
	return m_pauseTime != 0.f;
}

void Timer::Restart()
{
	if (m_pauseTime != 0.f) {
		m_startTime += (m_clock->GetTotalSeconds() - m_pauseTime);
		m_pauseTime = 0.f;
	}
}

float Timer::GetElapsedTime() const
{
	if (m_startTime == 0.f) {
		return 0.f;
	}
	return m_clock->GetTotalSeconds() - m_startTime;
}

void Timer::SetElapsedTime( float seconds )
{
	m_startTime -= seconds;
}

float Timer::GetElapsedFraction() const
{
	return GetClamped( GetElapsedTime() / m_period, 0.f, 1.f );
}

bool Timer::IsStopped() const
{
	return m_startTime == 0.f;
}

bool Timer::HasPeriodElapsed() const
{
	return GetElapsedTime() >= m_period && m_startTime != 0.f && !IsPaused();
}

bool Timer::HasStartedAndNotPeriodElapsed() const
{
	return GetElapsedTime() < m_period && m_startTime != 0.f || IsPaused();
}

bool Timer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed()) {
		m_startTime += m_period;
		return true;
	}
	return false;
}

void Timer::SetPeriodSeconds( float newPeriod )
{
	m_period = newPeriod;
}

float Timer::GetPeriodSeconds() const
{
	return m_period;
}
