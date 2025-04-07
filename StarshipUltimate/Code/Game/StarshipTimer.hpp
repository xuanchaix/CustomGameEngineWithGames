#pragma once
#include "Game/GameCommon.hpp"

// a Timer class inherit engine's clock
class StarshipTimer : Clock {
	void SetTimerAndCallBackFunction( float secondsToCallBack, void (*funcName)(), bool isLoop = false );
	void ClearTimer();
protected:
	virtual void Advance( float deltaTimeSeconds );

protected:
	bool m_callbackFuncIsSet = false;
	float m_secondsToCallBack = 0.f;
	void (*m_functionToCallback)() = nullptr;
	bool m_isLoop = false;
};