#include "Game/SettingsScreen.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Game.hpp"

SettingsScreen::SettingsScreen()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "KeyPressed", Event_KeyPressed );

	m_generalSettingName.reserve( 10 );
	m_generalSettingDesc.reserve( 10 );
	m_generalSettingName.emplace_back( Stringf( "Auto Fire Main Weapon" ) );
	m_generalSettingDesc.emplace_back( Stringf( g_theApp->m_autoShootMainWeapon ? "Yes" : "No" ) );
	m_generalSettingName.emplace_back( Stringf( "Auto Fire Sub-Weapon" ) );
	m_generalSettingDesc.emplace_back( Stringf( g_theApp->m_autoShootSubWeapon ? "Yes" : "No" ) );
	m_generalSettingName.emplace_back( Stringf( "Immortal Mode (Cannot Die)" ) );
	m_generalSettingDesc.emplace_back( Stringf( g_theApp->m_playerNoDie ? "Yes" : "No" ) );

	m_videoSettingName.reserve( 10 );
	m_videoSettingDesc.reserve( 10 );
	m_videoSettingName.emplace_back( Stringf( "Full Screen" ) );
	m_videoSettingDesc.emplace_back( Stringf( g_theApp->m_fullScreen ? "Yes" : "No" ) );

	m_audioSettingName.reserve( 10 );
	m_audioSettingDesc.reserve( 10 );
	m_audioSettingName.emplace_back( Stringf( "Music Volume" ) );
	m_audioSettingDesc.emplace_back( Stringf( "%d", (int)(g_theApp->m_musicVolume * 100.f) ) );
	m_audioSettingName.emplace_back( Stringf( "SFX Volume" ) );
	m_audioSettingDesc.emplace_back( Stringf( "%d", (int)(g_theApp->m_soundVolume * 100.f) ) );

	m_keySettingName.reserve( 15 );
	m_keySettingDesc.reserve( 15 );
	m_keySettingName.emplace_back( Stringf( "Move Up" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_UP_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Move Left" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_LEFT_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Move Down" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_DOWN_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Move Right" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_RIGHT_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Fire Main Weapon" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_MAIN_WEAPON_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Fire Sub-Weapon" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_SUB_WEAPON_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Dash" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_DASH_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Use Skill" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_ULTIMATE_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Interact" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_INTERACT_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Open Item Screen" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_ITEM_SCREEN_KEYCODE].c_str() ) );
	m_keySettingName.emplace_back( Stringf( "Open Map Screen" ) );
	m_keySettingDesc.emplace_back( Stringf( "%s", g_starshipKeyStringMapping[PLAYER_MAP_SCREEN_KEYCODE].c_str() ) );

}

void SettingsScreen::GoIntoScreen()
{
	m_state = SettingScreenState::GeneralSettings;
	m_topSettingsPos = 0;
	m_settingsPos = 0;
	m_intoSettings = false;
}

void SettingsScreen::ExitScreen()
{

}

void SettingsScreen::Update()
{
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
		if (m_isFromGame) {
			g_theApp->GoToAppMode( AppState::PLAY_MODE );
			m_isFromGame = false;
		}
		else {
			g_theApp->GoToAppMode( AppState::ATTRACT_MODE );
		}
	}

	if (g_theInput->WasKeyJustPressed( 'W' ) || g_theInput->WasKeyJustPressed( KEYCODE_UPARROW )) {
		if (!m_intoSettings) {
			m_topSettingsPos = (m_topSettingsPos + m_numOfTopButtons - 1) % m_numOfTopButtons;
			m_state = (SettingScreenState)m_topSettingsPos;
		}
		else {
			int numOfButtons = 0;
			if (m_state == SettingScreenState::GeneralSettings) {
				numOfButtons = (int)m_generalSettingName.size();
			}
			else if (m_state == SettingScreenState::VideoSettings) {
				numOfButtons = (int)m_videoSettingName.size();
			}
			else if (m_state == SettingScreenState::AudioSettings) {
				numOfButtons = (int)m_audioSettingName.size();
			}
			else if (m_state == SettingScreenState::KeySettings) {
				numOfButtons = (int)m_keySettingName.size();
			}
			m_settingsPos = (m_settingsPos + numOfButtons - 1) % numOfButtons;

		}
	}
	if (g_theInput->WasKeyJustPressed( 'S' ) || g_theInput->WasKeyJustPressed( KEYCODE_DOWNARROW )) {
		if (!m_intoSettings) {
			m_topSettingsPos = (m_topSettingsPos + 1) % m_numOfTopButtons;
			m_state = (SettingScreenState)m_topSettingsPos;
		}
		else {
			int numOfButtons = 0;
			if (m_state == SettingScreenState::GeneralSettings) {
				numOfButtons = (int)m_generalSettingName.size();
			}
			else if (m_state == SettingScreenState::VideoSettings) {
				numOfButtons = (int)m_videoSettingName.size();
			}
			else if (m_state == SettingScreenState::AudioSettings) {
				numOfButtons = (int)m_audioSettingName.size();
			}
			else if (m_state == SettingScreenState::KeySettings) {
				numOfButtons = (int)m_keySettingName.size();
			}
			m_settingsPos = (m_settingsPos + 1) % numOfButtons;

		}
	}
	if (g_theInput->WasKeyJustPressed( 'A' ) || g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW )) {
		if (m_intoSettings) {
			m_intoSettings = false;
		}
	}
	if (g_theInput->WasKeyJustPressed( 'D' ) || g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW )) {
		if (!m_intoSettings) {
			m_intoSettings = true;
			m_settingsPos = 0;
		}
	}

	if (g_theInput->WasKeyJustPressed( ' ' ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
		if (m_intoSettings) {
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ), false, g_theApp->m_soundVolume * 0.6f );
			if (m_state == SettingScreenState::GeneralSettings) {
				if (m_settingsPos == 0) {
					g_theApp->m_autoShootMainWeapon = !g_theApp->m_autoShootMainWeapon;
					m_generalSettingDesc[0] = g_theApp->m_autoShootMainWeapon ? "Yes" : "No";
				}
				else if (m_settingsPos == 1) {
					g_theApp->m_autoShootSubWeapon = !g_theApp->m_autoShootSubWeapon;
					m_generalSettingDesc[1] = g_theApp->m_autoShootSubWeapon ? "Yes" : "No";
				}
				else if (m_settingsPos == 2) {
					g_theApp->m_playerNoDie = !g_theApp->m_playerNoDie;
					m_generalSettingDesc[2] = g_theApp->m_playerNoDie ? "Yes" : "No";
				}
			}
			else if (m_state == SettingScreenState::KeySettings) {
				m_state = SettingScreenState::BindingKey;
			}
			else if (m_state == SettingScreenState::AudioSettings) {

			}
			else if (m_state == SettingScreenState::VideoSettings) {
				if (m_settingsPos == 0) {
					g_theApp->m_fullScreen = !g_theApp->m_fullScreen;
					g_window->SetFullScreen( g_theApp->m_fullScreen );
					m_videoSettingDesc[0] = g_theApp->m_fullScreen ? "Yes" : "No";
				}
			}
			else if (m_state == SettingScreenState::BindingKey) {
				FireEvent( "KeyPressed" );
			}
		}
	}

	if (m_state == SettingScreenState::AudioSettings) {
		float deltaTime = Clock::GetSystemClock()->GetDeltaSeconds();
		if (g_theInput->IsKeyDown( 'Z' )) {
			if (m_settingsPos == 0) {
				g_theApp->m_musicVolume -= 0.5f * deltaTime;
				g_theApp->m_musicVolume = g_theApp->m_musicVolume < 0.f ? 0.f : g_theApp->m_musicVolume;
				m_audioSettingDesc[0] = Stringf( "%d", (int)(g_theApp->m_musicVolume * 100.f) );
				if (g_theApp->m_appState == AppState::PLAY_MODE) {
					g_theAudio->SetSoundPlaybackVolume( g_theGame->m_backgroundMusicID, 0.6f * g_theApp->m_musicVolume );
				}
				else {
					g_theAudio->SetSoundPlaybackVolume( g_theApp->m_attractModeMusic, 0.6f * g_theApp->m_musicVolume );
				}
			}
			else if (m_settingsPos == 1) {
				g_theApp->m_soundVolume -= 0.5f * deltaTime;
				g_theApp->m_soundVolume = g_theApp->m_soundVolume < 0.f ? 0.f : g_theApp->m_soundVolume;
				m_audioSettingDesc[1] = Stringf( "%d", (int)(g_theApp->m_soundVolume * 100.f) );
			}
		}
		if (g_theInput->IsKeyDown( 'C' )) {
			if (m_settingsPos == 0) {
				g_theApp->m_musicVolume += 0.5f * deltaTime;
				g_theApp->m_musicVolume = g_theApp->m_musicVolume > 1.f ? 1.f : g_theApp->m_musicVolume;
				m_audioSettingDesc[0] = Stringf( "%d", (int)(g_theApp->m_musicVolume * 100.f) );
				if (g_theApp->m_appState == AppState::PLAY_MODE) {
					g_theAudio->SetSoundPlaybackVolume( g_theGame->m_backgroundMusicID, 0.6f * g_theApp->m_musicVolume );
				}
				else {
					g_theAudio->SetSoundPlaybackVolume( g_theApp->m_attractModeMusic, 0.6f * g_theApp->m_musicVolume );
				}
			}
			else if (m_settingsPos == 1) {
				g_theApp->m_soundVolume += 0.5f * deltaTime;
				g_theApp->m_soundVolume = g_theApp->m_soundVolume > 1.f ? 1.f : g_theApp->m_soundVolume;
				m_audioSettingDesc[1] = Stringf( "%d", (int)(g_theApp->m_soundVolume * 100.f) );
			}
		}
	}
}

void SettingsScreen::Render() const
{
	// buttons
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	Vec2 buttonLB[4] = { Vec2( 100.f, 640.f ),  Vec2( 100.f, 540.f ), Vec2( 100.f, 440.f ), Vec2( 100.f, 340.f ) };
	std::string buttonDesc[4] = { "General", "Video", "Audio", "Key Bindings" };
	float buttonWidth = 200.f;
	float buttonHeight = 65.f;

	if (m_state == SettingScreenState::BindingKey) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 900.f, 20.f ), Vec2( 1600.f, 60.f ) ), 40.f, "Press Any Key To Bind", Rgba8::WHITE );
	}
	else if (m_state == SettingScreenState::AudioSettings) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 20.f ), Vec2( 1600.f, 60.f ) ), 35.f, "WASD Navigate   Z/C Modify   Esc Quit", Rgba8::WHITE );
	}
	else {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 20.f ), Vec2( 1600.f, 60.f ) ), 35.f, "WASD Navigate   Space/Enter Modify   Esc Quit", Rgba8::WHITE );
	}

	//g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 630.f ), Vec2( 1600.f, 700.f ) ), 70.f, "Starship Ultimate", Rgba8( 255, 255, 0 ) );
	for (int i = 0; i < m_numOfTopButtons; i++) {
		AddVertsForAABB2D( verts, AABB2( buttonLB[i], buttonLB[i] + Vec2( buttonWidth, buttonHeight ) ), Rgba8( 255, 255, 153 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( buttonLB[i], buttonLB[i] + Vec2( buttonWidth, buttonHeight ) ), 25.f, buttonDesc[i], Rgba8( 0, 0, 0 ) );
		if (m_topSettingsPos == i && !m_intoSettings) {
			Rgba8 color = Rgba8( 51, 51, 255 );
			AddVertsForLineSegment2D( verts, buttonLB[i], buttonLB[i] + Vec2( 0.f, 6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i], buttonLB[i] + Vec2( 6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, buttonHeight ), buttonLB[i] + Vec2( buttonWidth, buttonHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, buttonHeight ), buttonLB[i] + Vec2( buttonWidth, buttonHeight ) + Vec2( -6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, 0.f ), buttonLB[i] + Vec2( buttonWidth, 0.f ) + Vec2( 0.f, 6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, 0.f ), buttonLB[i] + Vec2( buttonWidth, 0.f ) + Vec2( -6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( 0.f, buttonHeight ), buttonLB[i] + Vec2( 0.f, buttonHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( 0.f, buttonHeight ), buttonLB[i] + Vec2( 0.f, buttonHeight ) + Vec2( 6.f, 0.f ), 5.f, color );
		}
	}

	std::vector<std::string> const* settingName = nullptr;
	std::vector<std::string> const* settingDesc = nullptr;

	Vec2 settingStartLBPos = Vec2( 450.f, 640.f );
	float settingDescOffsetX = 500.f;
	float settingDescWidthX = 400.f;
	float marginBetweenNameAndDesc = 50.f;
	float settingTextHeight = 30.f;
	float settingDisplacementY = 40.f;

	if (m_state == SettingScreenState::GeneralSettings) {
		settingName = &m_generalSettingName;
		settingDesc = &m_generalSettingDesc;
	}
	else if (m_state == SettingScreenState::VideoSettings) {
		settingName = &m_videoSettingName;
		settingDesc = &m_videoSettingDesc;
	}
	else if (m_state == SettingScreenState::AudioSettings) {
		settingName = &m_audioSettingName;
		settingDesc = &m_audioSettingDesc;
	}
	else if (m_state == SettingScreenState::KeySettings || m_state == SettingScreenState::BindingKey) {
		settingName = &m_keySettingName;
		settingDesc = &m_keySettingDesc;
	}

	if (settingName) {
		for (int i = 0; i < (int)settingName->size(); i++) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( settingStartLBPos + Vec2( 0.f, -i * settingDisplacementY ), settingStartLBPos + Vec2( settingDescOffsetX, settingTextHeight - i * settingDisplacementY ) ), settingTextHeight, (*settingName)[i], Rgba8::WHITE, 0.618f, Vec2( 0.5f, 0.5f ), TextBoxMode::OVERRUN );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( settingStartLBPos + Vec2( settingDescOffsetX + marginBetweenNameAndDesc, -i * settingDisplacementY ), settingStartLBPos + Vec2( settingDescWidthX + marginBetweenNameAndDesc + settingDescOffsetX, settingTextHeight - i * settingDisplacementY ) ), settingTextHeight, (*settingDesc)[i], Rgba8::WHITE, 0.618f, Vec2( 0.5f, 0.5f ), TextBoxMode::OVERRUN );

			if (m_settingsPos == i && m_intoSettings) {
				Rgba8 color = Rgba8::WHITE;
				Vec2 settingLB = settingStartLBPos + Vec2( settingDescOffsetX + marginBetweenNameAndDesc, -i * settingDisplacementY );
				float settingWidth = settingDescWidthX;
				float settingHeight = settingTextHeight;
				AddVertsForLineSegment2D( verts, settingLB, settingLB + Vec2( 0.f, 6.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB, settingLB + Vec2( 6.f, 0.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB + Vec2( settingWidth, settingHeight ), settingLB + Vec2( settingWidth, settingHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB + Vec2( settingWidth, settingHeight ), settingLB + Vec2( settingWidth, settingHeight ) + Vec2( -6.f, 0.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB + Vec2( settingWidth, 0.f ), settingLB + Vec2( settingWidth, 0.f ) + Vec2( 0.f, 6.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB + Vec2( settingWidth, 0.f ), settingLB + Vec2( settingWidth, 0.f ) + Vec2( -6.f, 0.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB + Vec2( 0.f, settingHeight ), settingLB + Vec2( 0.f, settingHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
				AddVertsForLineSegment2D( verts, settingLB + Vec2( 0.f, settingHeight ), settingLB + Vec2( 0.f, settingHeight ) + Vec2( 6.f, 0.f ), 5.f, color );
			}

		}
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}


bool SettingsScreen::Event_KeyPressed( EventArgs& args )
{
	unsigned char keyCode = (unsigned char)args.GetValue( "KeyCode", -1 );
	SettingsScreen* screen = g_theApp->m_settingsScreen;
	if (screen->m_state == SettingScreenState::BindingKey) {
		if (screen->m_settingsPos == 0) {
			PLAYER_UP_KEYCODE = keyCode;
			screen->m_keySettingDesc[0] = g_starshipKeyStringMapping[PLAYER_UP_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 1) {
			PLAYER_LEFT_KEYCODE = keyCode;
			screen->m_keySettingDesc[1] = g_starshipKeyStringMapping[PLAYER_LEFT_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 2) {
			PLAYER_DOWN_KEYCODE = keyCode;
			screen->m_keySettingDesc[2] = g_starshipKeyStringMapping[PLAYER_DOWN_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 3) {
			PLAYER_RIGHT_KEYCODE = keyCode;
			screen->m_keySettingDesc[3] = g_starshipKeyStringMapping[PLAYER_RIGHT_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 4) {
			PLAYER_MAIN_WEAPON_KEYCODE = keyCode;
			screen->m_keySettingDesc[4] = g_starshipKeyStringMapping[PLAYER_MAIN_WEAPON_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 5) {
			PLAYER_SUB_WEAPON_KEYCODE = keyCode;
			screen->m_keySettingDesc[5] = g_starshipKeyStringMapping[PLAYER_SUB_WEAPON_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 6) {
			PLAYER_DASH_KEYCODE = keyCode;
			screen->m_keySettingDesc[6] = g_starshipKeyStringMapping[PLAYER_DASH_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 7) {
			PLAYER_ULTIMATE_KEYCODE = keyCode;
			screen->m_keySettingDesc[7] = g_starshipKeyStringMapping[PLAYER_ULTIMATE_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 8) {
			PLAYER_INTERACT_KEYCODE = keyCode;
			screen->m_keySettingDesc[8] = g_starshipKeyStringMapping[PLAYER_INTERACT_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 9) {
			PLAYER_ITEM_SCREEN_KEYCODE = keyCode;
			screen->m_keySettingDesc[9] = g_starshipKeyStringMapping[PLAYER_ITEM_SCREEN_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		else if (screen->m_settingsPos == 10) {
			PLAYER_MAP_SCREEN_KEYCODE = keyCode;
			screen->m_keySettingDesc[10] = g_starshipKeyStringMapping[PLAYER_MAP_SCREEN_KEYCODE];
			screen->m_state = SettingScreenState::KeySettings;
		}
		return true;
	}
	else {
		return false;
	}
}