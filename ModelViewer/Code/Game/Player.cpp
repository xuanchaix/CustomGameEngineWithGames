#include "Game/Player.hpp"
#include "Game/Game.hpp"

Player::Player( Game* game )
	:Entity(game)
{

}

Player::~Player()
{

}

void Player::Update()
{
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	m_camera.SetTransform( m_position, m_orientation );
	float speed = 1.f;
	Vec3 iBasis, jBasis, kBasis;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );

	if (g_theInput->IsKeyDown( 0x10/*Shift Key*/)) {
		speed *= 10.f;
	}
	if (g_theInput->IsKeyDown( 'W' )) {
		m_position += iBasis * speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		m_position -= iBasis * speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		m_position += jBasis * speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		m_position -= jBasis * speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'Q' )) {
		m_position += kBasis * speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'E' )) {
		m_position -= kBasis * speed * deltaSeconds;
	}
	if (g_theInput->WasKeyJustPressed( 'H' )) {
		m_orientation.m_pitchDegrees = 0.f;
		m_orientation.m_rollDegrees = 0.f;
		m_orientation.m_yawDegrees = 0.f;
		m_position = Vec3( 0.f, 0.f, 0.f );
	}

	//if (deltaSeconds != 0.f) {
		Vec2 cursorDisp = g_theInput->GetCursorClientDelta();
		m_orientation.m_yawDegrees -= 0.125f * cursorDisp.x * g_window->GetClientDimensions().x;
		m_orientation.m_pitchDegrees -= 0.125f * cursorDisp.y * g_window->GetClientDimensions().y;
	//}

	/*XboxController& controller = g_theInput->GetController(m_playerId);
	if (controller.IsConnected()) {
		if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A )) {
			speed *= 10.f;
		}
		Vec2 normalizedLeftStick = controller.GetLeftStick().GetPosition();
		Vec2 normalizedRightStick = controller.GetRightStick().GetPosition();
		float normalizedLeftTrigger = controller.GetLeftTrigger();
		float normalizedRightTrigger = controller.GetRightTrigger();
		m_position += iBasis * speed * normalizedLeftStick.y * deltaSeconds;
		m_position -= jBasis * speed * normalizedLeftStick.x * deltaSeconds;
		m_orientation.m_rollDegrees -= 90.f * deltaSeconds * normalizedLeftTrigger;
		m_orientation.m_rollDegrees += 90.f * deltaSeconds * normalizedRightTrigger;
		m_orientation.m_yawDegrees -= 90.f * deltaSeconds * normalizedRightStick.x;
		m_orientation.m_pitchDegrees -= 90.f * deltaSeconds * normalizedRightStick.y;
		if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )) {
			m_orientation.m_pitchDegrees = 0.f;
			m_orientation.m_rollDegrees = 0.f;
			m_orientation.m_yawDegrees = 0.f;
			m_position = Vec3( 0.f, 0.f, 0.f );
		}
		if (controller.IsButtonDown( XboxButtonID::XBOX_BUTTON_LEFTSHOULDER )) {
			m_position -= Vec3( 0, 0, 1 ) * speed * deltaSeconds;
		}
		if (controller.IsButtonDown( XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER )) {
			m_position += Vec3( 0, 0, 1 ) * speed * deltaSeconds;
		}
	}*/

	m_orientation.m_pitchDegrees = GetClamped( m_orientation.m_pitchDegrees, -85.f, 85.f );
	m_orientation.m_rollDegrees = GetClamped( m_orientation.m_rollDegrees, -45.f, 45.f );
}

void Player::Render() const
{

}

void Player::Die()
{
	m_isDead = true;
}
