#pragma once
#include <vector>

//------------------------------------------------------------------------------
//
class Clock {
public:
	Clock();
	explicit Clock( Clock& parent );
	~Clock();
	Clock( const Clock& copy ) = delete;

	void Reset();

	bool IsPaused() const;
	void Pause();
	void UnPause();
	void TogglePause();

	void StepSingleFrame();

	void SetTimeScale( float timeScale );
	float GetTimeScale() const;

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	size_t GetFrameCount() const;

	static Clock* GetSystemClock();
	static void TickSystemClock();

protected:
	void Tick();
	virtual void Advance( float deltaTimeSeconds );

	void AddChild( Clock* childClock );
	void RemoveChild( Clock* childClock );

protected:
	Clock* m_parent = nullptr;
	std::vector<Clock*> m_children;

	float m_lastUpdateTimeInSeconds = 0.f;
	float m_totalSeconds = 0.f;
	float m_deltaSeconds = 0.f;
	size_t m_frameCount = 0;

	float m_timeScale = 1.f;

	bool m_isPaused = false;

	bool m_stepSingleFrame = false;

	float m_maxDeltaSeconds = 0.1f;

	static Clock* s_systemClock;
};