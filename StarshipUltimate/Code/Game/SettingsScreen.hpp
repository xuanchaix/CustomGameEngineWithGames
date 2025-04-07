#pragma once
#include "Game/GameCommon.hpp"

enum class SettingScreenState {
	GeneralSettings, VideoSettings, AudioSettings, KeySettings, BindingKey
};

class SettingsScreen {
public:
	SettingsScreen();
	virtual void GoIntoScreen();
	virtual void ExitScreen();
	virtual void Update();
	virtual void Render() const;
	static bool Event_KeyPressed( EventArgs& args );

	SettingScreenState m_state = SettingScreenState::GeneralSettings;
	int m_topSettingsPos = 0;
	int m_numOfTopButtons = 4;
	int m_settingsPos = 0;
	bool m_intoSettings = false;

	bool m_isFromGame = false;

	Strings m_generalSettingName;
	Strings m_generalSettingDesc;

	Strings m_videoSettingName;
	Strings m_videoSettingDesc;

	Strings m_audioSettingName;
	Strings m_audioSettingDesc;

	Strings m_keySettingName;
	Strings m_keySettingDesc;
};