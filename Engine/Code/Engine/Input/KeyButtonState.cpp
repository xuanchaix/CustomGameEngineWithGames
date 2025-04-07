#include "Engine/Input/KeyButtonState.hpp"

void KeyButtonState::Reset()
{
	m_currentState = false;
	m_previousStste = false;
}
