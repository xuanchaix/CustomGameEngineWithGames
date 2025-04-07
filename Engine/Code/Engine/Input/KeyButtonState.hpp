#pragma once

struct KeyButtonState {
public:
	bool m_currentState = false;
	bool m_previousStste = false;
public:
	void Reset();
};