#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

enum class XboxButtonID 
{
	XBOX_BUTTON_A, XBOX_BUTTON_B, XBOX_BUTTON_X, XBOX_BUTTON_Y, 
	XBOX_BUTTON_UP, XBOX_BUTTON_DOWN, XBOX_BUTTON_LEFT, XBOX_BUTTON_RIGHT,
	XBOX_BUTTON_LEFTSHOULDER, XBOX_BUTTON_RIGHTSHOULDER, 
	XBOX_BUTTON_LEFTJOYSTICKPRESS, XBOX_BUTTON_RIGHTJOYSTICKPRESS,
	XBOX_BUTTON_START, XBOX_BUTTON_BACK, NUM
};

class XboxController {
	friend class InputSystem;
public:
	XboxController();
	XboxController( int id );
	~XboxController();

	bool IsConnected() const;
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton( XboxButtonID buttonID ) const;
	bool IsButtonDown( XboxButtonID buttonID ) const;
	bool WasButtonJustPressed( XboxButtonID buttonID ) const;
	bool WasButtonJustReleased( XboxButtonID buttonID ) const;

	void SetVibration( unsigned short leftSpeed, unsigned short rightSpeed );
	void SetId( int id );
private:
	void Update();
	void EndFrame();
	void Reset();
	void UpdateJoystick( AnalogJoystick& out_joystick, short rawX, short rawY );
	void UpdateTrigger( float& out_triggerValue, unsigned char rawValue );
	void UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag );

private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int)XboxButtonID::NUM];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;
};