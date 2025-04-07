#include "Game/StarshipTimer.hpp"

void StarshipTimer::SetTimerAndCallBackFunction( float secondsToCallBack, void (*funcName)(), bool isLoop /*= false */ )
{
	m_secondsToCallBack = secondsToCallBack;
	m_functionToCallback = funcName;
	m_isLoop = isLoop;
}

void StarshipTimer::ClearTimer()
{
	m_callbackFuncIsSet = false;
}
