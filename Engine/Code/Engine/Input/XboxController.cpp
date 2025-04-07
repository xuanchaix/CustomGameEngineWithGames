#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <windows.h>
#include <Xinput.h>

#pragma comment( lib, "xinput9_1_0" )

constexpr float XBOX_JOYSTICK_INNER_DEAD_ZONE = 0.35f;
constexpr float XBOX_JOYSTICK_OUTER_DEAD_ZONE = 0.95f;

XboxController::XboxController(int id): m_id(id)
{
	m_leftStick.SetDeadZoneThresholds( XBOX_JOYSTICK_INNER_DEAD_ZONE, XBOX_JOYSTICK_OUTER_DEAD_ZONE );
	m_rightStick.SetDeadZoneThresholds( XBOX_JOYSTICK_INNER_DEAD_ZONE, XBOX_JOYSTICK_OUTER_DEAD_ZONE );
}

XboxController::XboxController()
{
	m_leftStick.SetDeadZoneThresholds( XBOX_JOYSTICK_INNER_DEAD_ZONE, XBOX_JOYSTICK_OUTER_DEAD_ZONE );
	m_rightStick.SetDeadZoneThresholds( XBOX_JOYSTICK_INNER_DEAD_ZONE, XBOX_JOYSTICK_OUTER_DEAD_ZONE );
}

XboxController::~XboxController()
{
}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton( XboxButtonID buttonID ) const
{
	return m_buttons[(int)buttonID];
}

bool XboxController::IsButtonDown( XboxButtonID buttonID ) const
{
	return m_buttons[(int)buttonID].m_currentState;
}

bool XboxController::WasButtonJustPressed( XboxButtonID buttonID ) const
{
	return m_buttons[(int)buttonID].m_currentState && !(m_buttons[(int)buttonID].m_previousStste);
}

bool XboxController::WasButtonJustReleased( XboxButtonID buttonID ) const
{
	return !(m_buttons[(int)buttonID].m_currentState) && m_buttons[(int)buttonID].m_previousStste;
}

void XboxController::SetVibration( unsigned short leftSpeed, unsigned short rightSpeed )
{
	XINPUT_VIBRATION xVibration;
	xVibration.wLeftMotorSpeed = leftSpeed;
	xVibration.wRightMotorSpeed = rightSpeed;
	XInputSetState( m_id, &xVibration );
}

void XboxController::SetId( int id )
{
	m_id = id;
}

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState( m_id, &xboxControllerState );
	if (errorStatus == ERROR_SUCCESS) {
		m_isConnected = true;
		UpdateJoystick( m_leftStick, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY );
		UpdateJoystick( m_rightStick, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY );
		UpdateTrigger( m_leftTrigger, xboxControllerState.Gamepad.bLeftTrigger );
		UpdateTrigger( m_rightTrigger, xboxControllerState.Gamepad.bRightTrigger );
		UpdateButton( XboxButtonID::XBOX_BUTTON_A, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_A );
		UpdateButton( XboxButtonID::XBOX_BUTTON_B, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_B );
		UpdateButton( XboxButtonID::XBOX_BUTTON_X, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_X );
		UpdateButton( XboxButtonID::XBOX_BUTTON_Y, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_Y );
		UpdateButton( XboxButtonID::XBOX_BUTTON_DOWN, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN );
		UpdateButton( XboxButtonID::XBOX_BUTTON_LEFT, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT );
		UpdateButton( XboxButtonID::XBOX_BUTTON_RIGHT, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT );
		UpdateButton( XboxButtonID::XBOX_BUTTON_UP, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP );
		UpdateButton( XboxButtonID::XBOX_BUTTON_BACK, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_BACK );
		UpdateButton( XboxButtonID::XBOX_BUTTON_START, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_START );
		UpdateButton( XboxButtonID::XBOX_BUTTON_LEFTJOYSTICKPRESS, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB );
		UpdateButton( XboxButtonID::XBOX_BUTTON_RIGHTJOYSTICKPRESS, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB );
		UpdateButton( XboxButtonID::XBOX_BUTTON_LEFTSHOULDER, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
		UpdateButton( XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
	}
	else {
		if (m_isConnected) {
			m_isConnected = false;
			Reset();
		}
		return;
	}
}

void XboxController::EndFrame()
{
	for (int i = 0; i < (int)XboxButtonID::NUM; i++) {
		m_buttons[i].m_previousStste = m_buttons[i].m_currentState;
	}
}

void XboxController::Reset()
{
	m_leftTrigger = 0.f;
	m_rightTrigger = 0.f;
	for (int i = 0; i < (int)XboxButtonID::NUM; i++) {
		m_buttons[i].Reset();
	}
	m_leftStick.Reset();
	m_rightStick.Reset();
}

void XboxController::UpdateJoystick( AnalogJoystick& out_joystick, short rawX, short rawY )
{
	out_joystick.UpdatePosition( RangeMapClamped( (float)rawX, -32768.f, 32767.f, -1.f, 1.f ), 
								 RangeMapClamped( (float)rawY, -32768.f, 32767.f, -1.f, 1.f ) );
}

void XboxController::UpdateTrigger( float& out_triggerValue, unsigned char rawValue )
{
	out_triggerValue = (float)rawValue / 255.f;
}

void XboxController::UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag )
{
	if ((buttonFlags & buttonFlag) == buttonFlag) {
		m_buttons[(int)buttonID].m_currentState = true;
	}
	else {
		m_buttons[(int)buttonID].m_currentState = false;
	}
}

