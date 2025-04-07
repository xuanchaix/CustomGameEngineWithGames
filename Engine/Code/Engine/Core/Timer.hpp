#pragma once

class Clock;

class Timer {
public:
	explicit Timer( float period, Clock const* clock=nullptr );

	void Start();

	void Stop();

	void Pause();

	bool IsPaused() const;

	void Restart();

	float GetElapsedTime() const;

	void SetElapsedTime( float seconds );

	float GetElapsedFraction() const;

	bool IsStopped() const;

	bool HasPeriodElapsed() const;

	bool HasStartedAndNotPeriodElapsed() const;

	bool DecrementPeriodIfElapsed();

	void SetPeriodSeconds( float newPeriod );

	float GetPeriodSeconds() const;

protected:
	Clock const* m_clock = nullptr;

	float m_startTime = 0.f;
	float m_period = 0.f;
	float m_pauseTime = 0.f;
};