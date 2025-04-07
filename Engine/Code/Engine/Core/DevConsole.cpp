#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/NetSystem/NetSystem.hpp"


DevConsole* g_devConsole = nullptr;

Rgba8 const DevConsole::INFO_ERROR = Rgba8( 200, 51, 51 );
Rgba8 const DevConsole::INFO_WARNING = Rgba8( 255, 128, 0 );
Rgba8 const DevConsole::INFO_MAJOR = Rgba8( 51, 51, 255 );
Rgba8 const DevConsole::INFO_MINOR = Rgba8( 255, 255, 255 );
Rgba8 const DevConsole::INFO_COMMAND_ECHO = Rgba8( 102, 255, 102 );
Rgba8 const DevConsole::INFO_COMMAND_REMOTE_ECHO = Rgba8( 0, 255, 255 );
Rgba8 const DevConsole::INFO_INPUT_TEXT = Rgba8( 255, 0, 255 );
Rgba8 const DevConsole::INFO_INSERTION_POINT = Rgba8( 255, 255, 255 );

DevConsole::DevConsole( DevConsoleConfig const& config )
	:m_config(config)
{

}

DevConsole::~DevConsole()
{

}

void DevConsole::Startup()
{
	m_lines.reserve( 10000 );
	m_insertionPointBlinkTimer = new Timer( 0.5f );
	m_insertionPointBlinkTimer->Start();
	g_theEventSystem->SubscribeEventCallbackFunction( "KeyPressed", Event_KeyPressed );
	g_theEventSystem->SubscribeEventCallbackFunction( "CharInput", Event_CharInput );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_clear", Command_Clear );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_echo", Command_Echo );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_help", Command_Help );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_remoteCommand", Command_RemoteCommand );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_BurstTest", Command_BurstTest );
	g_theEventSystem->SubscribeEventCallbackFunction( "clear", Command_Clear );
	g_theEventSystem->SubscribeEventCallbackFunction( "echo", Command_Echo );
	g_theEventSystem->SubscribeEventCallbackFunction( "help", Command_Help );
	g_theEventSystem->SubscribeEventCallbackFunction( "remoteCommand", Command_RemoteCommand );
	g_theEventSystem->SubscribeEventCallbackFunction( "BurstTest", Command_BurstTest );
}

void DevConsole::Shutdown()
{

}

void DevConsole::BeginFrame()
{
	if (m_insertionPointBlinkTimer->DecrementPeriodIfElapsed()) {
		m_insertionPointVisible = !m_insertionPointVisible;
	}
}

void DevConsole::EndFrame()
{
	++m_frameNumber;
}

void DevConsole::Execute( std::string const& consoleCommandText, bool isRemote/*false*/)
{
	m_mutex.lock();
	m_commandHistory.push_back( consoleCommandText );
	if (g_devConsole->m_inputText == g_devConsole->m_commandHistory[g_devConsole->m_historyIndex]) {
		g_devConsole->m_historyIndex++;
	}
	else {
		g_devConsole->m_historyIndex = (int)g_devConsole->m_commandHistory.size();
	}
	if (isRemote) {
		m_lines.push_back( DevConsoleLine( consoleCommandText, DevConsole::INFO_COMMAND_REMOTE_ECHO, m_frameNumber, true ) );
	}
	else {
		m_lines.push_back( DevConsoleLine( consoleCommandText, DevConsole::INFO_INPUT_TEXT, m_frameNumber, true ) );
	}
	m_mutex.unlock();
	Strings commandLines;
	SplitStringWithQuotes( commandLines, consoleCommandText, '\n' );
	for (std::string const& line : commandLines) {
		Strings splitedLine;
		int numOfRes = SplitStringWithQuotes( splitedLine, line, ' ' );
		if (numOfRes == 0) {
			continue;
		}
		else if (numOfRes == 1) {
			FireEvent( "Command_" + splitedLine[0] );
			continue;
		}
		EventArgs args;
		for (int i = 1; i < numOfRes; i++) {
			Strings keyValuePair;
			int lengthOfPair = SplitStringWithQuotes( keyValuePair, splitedLine[i], '=' );
			if (lengthOfPair == 2) {
				args.SetValue( keyValuePair[0], keyValuePair[1] );
			}
			else {
				m_mutex.lock();
				m_lines.push_back( DevConsoleLine( Stringf( "command arguments should be key=value! Now: %s", splitedLine[i].c_str() ), DevConsole::INFO_ERROR, m_frameNumber, false ) );
				m_mutex.unlock();
			}
		}
		FireEvent( "Command_" + splitedLine[0], args );
	}
}

void DevConsole::AddLine( Rgba8 const& color, std::string const& text )
{
	m_mutex.lock();
	m_lines.push_back( DevConsoleLine( text, color, m_frameNumber, false ) );
	m_mutex.unlock();
}

void DevConsole::Render( AABB2 const& bounds, Renderer* rendererOverride /*= nullptr */ ) const
{
	m_mutex.lock();
	if (rendererOverride) {
		rendererOverride->SetModelConstants();
		rendererOverride->BeginCamera( *m_config.m_camera );
		Render_OpenFull( bounds, *rendererOverride, *m_config.m_defaultFont, m_config.m_fontAspect );
		rendererOverride->EndCamera( *m_config.m_camera );
	}
	else {
		m_config.m_defaultRenderer->SetModelConstants();
		m_config.m_defaultRenderer->BeginCamera( *m_config.m_camera );
		Render_OpenFull( bounds, *m_config.m_defaultRenderer, *m_config.m_defaultFont, m_config.m_fontAspect );
		m_config.m_defaultRenderer->EndCamera( *m_config.m_camera );
	}
	m_mutex.unlock();
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode( DevConsoleMode mode )
{
	m_mutex.lock();
	m_mode = mode;
	m_mutex.unlock();
}

void DevConsole::ToggleMode( DevConsoleMode mode )
{
	m_mutex.lock();
	if (m_mode == mode) {
		m_mode = DevConsoleMode::HIDDEN;
	}
	else {
		m_mode = mode;
	}
	m_mutex.unlock();
}

void DevConsole::ExecuteXmlCommandScriptNode( XmlElement const& commandScriptXmlElement )
{
	XmlElement const* iter = commandScriptXmlElement.FirstChildElement();
	while (iter != nullptr) {
		std::string commandStr = iter->Name();
		XmlAttribute const* attrIter = iter->FirstAttribute();
		while (attrIter != nullptr) {
			commandStr += " " + std::string(attrIter->Name()) + "=" + std::string(attrIter->Value());
			attrIter = attrIter->Next();
		}
		Execute( commandStr );
		iter = iter->NextSiblingElement();
	}
}

void DevConsole::ExecuteXmlCommandScriptFile( std::string const& commandScriptXmlFilePathName )
{
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( commandScriptXmlFilePathName.c_str() );
	if (errorCode != tinyxml2::XMLError::XML_SUCCESS) {
		ERROR_RECOVERABLE( Stringf( "Cannot read XML file %s", commandScriptXmlFilePathName.c_str() ) );
		return;
	}
	ExecuteXmlCommandScriptNode( *xmlDocument.RootElement() );
}

void DevConsole::Render_OpenFull( AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect /*= 1.f */ ) const
{
	float boundsBottomY = bounds.m_mins.y;
	float stepYPerLine = (bounds.m_maxs.y - bounds.m_mins.y) / m_config.m_maxRenderLines;
	Vec2 littleDisplacement = (bounds.m_maxs - bounds.m_mins) / 1200.f;
	std::vector<Vertex_PCU> verts;
	verts.reserve( 10000 );
	AddVertsForAABB2D( verts, bounds, Rgba8( 0, 0, 0, 120 ), AABB2() );
	renderer.BindTexture( nullptr );
	renderer.BindShader( nullptr );
	renderer.SetBlendMode( BlendMode::ALPHA );
	renderer.SetSamplerMode( SamplerMode::POINT_CLAMP );
	renderer.SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	renderer.SetDepthMode( DepthMode::ENABLED );
	renderer.SetModelConstants();
	renderer.DrawVertexArray( verts );
	verts.clear();
	
	// add inputting line
	font.AddVertsForTextInBox2D( verts, AABB2( Vec2( bounds.m_mins.x, boundsBottomY ) + littleDisplacement, Vec2( bounds.m_maxs.x, boundsBottomY + stepYPerLine ) + littleDisplacement ), stepYPerLine, m_inputText, Rgba8( 0, 0, 0 ), fontAspect, Vec2( 0.f, 0.f ), TextBoxMode::OVERRUN );
	font.AddVertsForTextInBox2D( verts, AABB2( Vec2( bounds.m_mins.x, boundsBottomY ), Vec2( bounds.m_maxs.x, boundsBottomY + stepYPerLine )), stepYPerLine, m_inputText, INFO_INPUT_TEXT, fontAspect, Vec2( 0.f, 0.f ), TextBoxMode::OVERRUN );
	boundsBottomY += stepYPerLine;
	int numOfLines = RoundDownToInt( m_config.m_maxRenderLines ) > (int)m_lines.size() ? (int)m_lines.size() : RoundDownToInt( m_config.m_maxRenderLines );

	for (int i = 0; i < numOfLines; i++) {
		if (m_lines[m_lines.size() - 1 - i].m_isInstruction) {
			font.AddVertsForTextInBox2D( verts,
				AABB2( Vec2( bounds.m_mins.x, boundsBottomY + stepYPerLine * i ) + littleDisplacement, Vec2( bounds.m_maxs.x, boundsBottomY + stepYPerLine * (i + 1) ) + littleDisplacement ),
				stepYPerLine, m_lines[m_lines.size() - 1 - i].m_text,
				Rgba8( 0, 0, 0 ), fontAspect, Vec2( 0.f, 0.f ), TextBoxMode::OVERRUN );
			font.AddVertsForTextInBox2D( verts,
				AABB2( Vec2( bounds.m_mins.x, boundsBottomY + stepYPerLine * i ), Vec2( bounds.m_maxs.x, boundsBottomY + stepYPerLine * (i + 1) ) ),
				stepYPerLine, m_lines[m_lines.size() - 1 - i].m_text,
				m_lines[m_lines.size() - 1 - i].m_color, fontAspect, Vec2( 0.f, 0.f ), TextBoxMode::OVERRUN );
		}
		else {
			font.AddVertsForTextInBox2D( verts,
				AABB2( Vec2( bounds.m_mins.x, boundsBottomY + stepYPerLine * i ) + littleDisplacement, Vec2( bounds.m_maxs.x, boundsBottomY + stepYPerLine * (i + 1) ) + littleDisplacement ),
				stepYPerLine, Stringf( "Frame:%i Time:%.2f Info:%s", m_lines[m_lines.size() - 1 - i].m_frameNum, m_lines[m_lines.size() - 1 - i].m_secondsFromBegin, m_lines[m_lines.size() - 1 - i].m_text.c_str() ),
				Rgba8( 0, 0, 0 ), fontAspect, Vec2( 0.f, 0.f ), TextBoxMode::OVERRUN );
			font.AddVertsForTextInBox2D( verts,
				AABB2( Vec2( bounds.m_mins.x, boundsBottomY + stepYPerLine * i ), Vec2( bounds.m_maxs.x, boundsBottomY + stepYPerLine * (i + 1) ) ),
				stepYPerLine, Stringf( "Frame:%i Time:%.2f Info:%s", m_lines[m_lines.size() - 1 - i].m_frameNum, m_lines[m_lines.size() - 1 - i].m_secondsFromBegin, m_lines[m_lines.size() - 1 - i].m_text.c_str() ),
				m_lines[m_lines.size() - 1 - i].m_color, fontAspect, Vec2( 0.f, 0.f ), TextBoxMode::OVERRUN );
		}
	}

	renderer.BindTexture( &m_config.m_defaultFont->GetTexture() );
	renderer.BindShader( nullptr );
	renderer.SetBlendMode( BlendMode::ALPHA );
	renderer.SetSamplerMode( SamplerMode::POINT_CLAMP );
	renderer.SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	renderer.SetDepthMode( DepthMode::ENABLED );
	renderer.SetModelConstants();
	renderer.DrawVertexArray( verts );

	std::vector<Vertex_PCU> insertionPointverts;
	if (m_insertionPointVisible) {
		AddVertsForLineSegment2D( insertionPointverts, Vec2( stepYPerLine * m_config.m_fontAspect * (float)m_insertionPointPosition + stepYPerLine * 0.1f, bounds.m_mins.y ), Vec2( stepYPerLine * m_config.m_fontAspect * (float)m_insertionPointPosition + stepYPerLine * 0.1f, bounds.m_mins.y + stepYPerLine ), (bounds.m_maxs.x - bounds.m_mins.x) / 400.f, INFO_INSERTION_POINT );
	}
	renderer.BindTexture( nullptr );
	renderer.BindShader( nullptr );
	renderer.SetBlendMode( BlendMode::ALPHA );
	renderer.SetSamplerMode( SamplerMode::POINT_CLAMP );
	renderer.SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	renderer.SetDepthMode( DepthMode::ENABLED );
	renderer.SetModelConstants();
	renderer.DrawVertexArray( insertionPointverts );
}

bool DevConsole::Event_KeyPressed( EventArgs& args )
{
	unsigned char keyCode = (unsigned char)args.GetValue( "KeyCode", (unsigned char)255 );
	if (g_devConsole->m_mode == DevConsoleMode::EMERGE && keyCode != KEYCODE_TILDE) {
		if (keyCode == KEYCODE_LEFTARROW) {
			if (g_devConsole->m_insertionPointPosition > 0) {
				g_devConsole->m_insertionPointPosition--;
			}
		}
		else if (keyCode == KEYCODE_RIGHTARROW) {
			if (g_devConsole->m_insertionPointPosition < (int)g_devConsole->m_inputText.size()) {
				g_devConsole->m_insertionPointPosition++;
			}
		}
		else if (keyCode == KEYCODE_HOME) {
			g_devConsole->m_insertionPointPosition = 0;
		}
		else if (keyCode == KEYCODE_END) {
			g_devConsole->m_insertionPointPosition = (int)g_devConsole->m_inputText.size();
		}
		else if (keyCode == KEYCODE_DELETE && g_devConsole->m_insertionPointPosition < (int)g_devConsole->m_inputText.size()) {
			g_devConsole->m_inputText.erase( g_devConsole->m_inputText.begin() + g_devConsole->m_insertionPointPosition );
		}
		else if (keyCode == KEYCODE_UPARROW && g_devConsole->m_historyIndex >= 1) {
			g_devConsole->m_historyIndex--;
			g_devConsole->m_inputText = g_devConsole->m_commandHistory[g_devConsole->m_historyIndex];
			g_devConsole->m_insertionPointPosition = (int)g_devConsole->m_inputText.size();
		}
		else if (keyCode == KEYCODE_DOWNARROW && g_devConsole->m_historyIndex < (int)g_devConsole->m_commandHistory.size() - 1) {
			g_devConsole->m_historyIndex++;
			g_devConsole->m_inputText = g_devConsole->m_commandHistory[g_devConsole->m_historyIndex];
			g_devConsole->m_insertionPointPosition = (int)g_devConsole->m_inputText.size();
		}
		return true;
	}
	else if (keyCode == KEYCODE_TILDE) {
		g_devConsole->ToggleMode( DevConsoleMode::EMERGE );
		return true;
	}
	else {
		return false;
	}
}

bool DevConsole::Event_CharInput( EventArgs& args )
{
	if (g_devConsole->m_mode == DevConsoleMode::EMERGE) {
		g_devConsole->m_insertionPointBlinkTimer->Start();
		g_devConsole->m_insertionPointVisible = true;
		unsigned char keyCode = (unsigned char)args.GetValue( "KeyCode", (unsigned char)-1 );
		if (keyCode != -1 && keyCode != 96 && keyCode >= 32 && keyCode < 126) {
			g_devConsole->m_inputText.insert( g_devConsole->m_inputText.begin() + g_devConsole->m_insertionPointPosition, keyCode );
			g_devConsole->m_insertionPointPosition++;
		}
		else if (keyCode == KEYCODE_ENTER) {
			if (g_devConsole->m_inputText.size() == 0) {
				g_devConsole->m_mode = DevConsoleMode::HIDDEN;
			}
			else {
				g_devConsole->Execute( g_devConsole->m_inputText );
				g_devConsole->m_inputText.clear();
				g_devConsole->m_insertionPointPosition = 0;
			}
		}
		else if (keyCode == KEYCODE_BACKSPACE && g_devConsole->m_insertionPointPosition >= 1) {
			g_devConsole->m_inputText.erase( g_devConsole->m_inputText.begin() + g_devConsole->m_insertionPointPosition - 1 );
			g_devConsole->m_insertionPointPosition--;
		}
		else if (keyCode == KEYCODE_ESC) {
			if (g_devConsole->m_inputText.size() > 0) {
				g_devConsole->m_inputText.clear();
				g_devConsole->m_insertionPointPosition = 0;
			}
			else {
				g_devConsole->m_mode = DevConsoleMode::HIDDEN;
			}
		}
		return true;
	}
	else {
		return false;
	}
}

bool DevConsole::Command_Clear( EventArgs& args )
{
	UNUSED( args );
	g_devConsole->m_lines.clear();
	return true;
}

bool DevConsole::Command_Echo( EventArgs& args )
{
	g_devConsole->m_lines.push_back( DevConsoleLine( args.GetValue( "Message", "Echo Nothing" ), DevConsole::INFO_COMMAND_ECHO, g_devConsole->m_frameNumber, true ) );
	return true;
}

bool DevConsole::Command_Help( EventArgs& args )
{
	UNUSED( args );
	Strings commands;
	g_theEventSystem->GetNamesOfAllRegisteredCommands( commands );
	g_devConsole->m_lines.push_back( DevConsoleLine( "Available Commands:", DevConsole::INFO_MAJOR, g_devConsole->m_frameNumber, true ) );

	for (auto& str : commands) {
		g_devConsole->m_lines.push_back( DevConsoleLine( str, DevConsole::INFO_MAJOR, g_devConsole->m_frameNumber, true ) );
	}
	return true;
}

bool DevConsole::Command_RemoteCommand( EventArgs& args )
{
#ifdef ENGINE_ENABLE_NET_SYSTEM
	if (g_theNetSystem) {
		std::string message = args.GetValue( "Command", "" );
		TrimString( message, '"' );
		g_theNetSystem->Send( message );
		return true;
	}
	return false;
#else
	UNUSED( args );
	return true;
#endif
}

bool DevConsole::Command_BurstTest( EventArgs& args )
{
	UNUSED( args );
#ifdef ENGINE_ENABLE_NET_SYSTEM
	if (g_theNetSystem) {
		std::string message = "Echo Message=\"This is a buffer length 2048.";
		for (int i = 0; i < 2004; i++) {
			message.push_back( 't' );
		}
		message.push_back( '\"' );

		g_theNetSystem->Send( message );
		message = "Echo Message=1";
		g_theNetSystem->Send( message );
		message = "Echo Message=2";
		g_theNetSystem->Send( message );
		message = "Echo Message=3";
		g_theNetSystem->Send( message );
		message = "Echo Message=4";
		g_theNetSystem->Send( message );
		message = "Echo Message=5";
		g_theNetSystem->Send( message );
		message = "Echo Message=6";
		g_theNetSystem->Send( message );
		message = "Echo Message=7";
		g_theNetSystem->Send( message );
		message = "Echo Message=8";
		g_theNetSystem->Send( message );
		message = "Echo Message=9";
		g_theNetSystem->Send( message );
		message = "Echo Message=10";
		g_theNetSystem->Send( message );
		message = "Echo Message=\"This is a buffer length 8192.";
		for (int i = 0; i < 8148; i++) {
			message.push_back( 't' );
		}
		message.push_back( '\"' );
		g_theNetSystem->Send( message );
		message = "Echo Message=11";
		g_theNetSystem->Send( message );
		message = "Echo Message=12";
		g_theNetSystem->Send( message );
		message = "Echo Message=13";
		g_theNetSystem->Send( message );
		message = "Echo Message=14";
		g_theNetSystem->Send( message );
		message = "Echo Message=15";
		g_theNetSystem->Send( message );
		message = "Echo Message=16";
		g_theNetSystem->Send( message );
		message = "Echo Message=17";
		g_theNetSystem->Send( message );
		message = "Echo Message=18";
		g_theNetSystem->Send( message );
		message = "Echo Message=19";
		g_theNetSystem->Send( message );
		message = "Echo Message=20";
		g_theNetSystem->Send( message );

		message = "Echo Message=\"This is a buffer length 2048.";
		for (int i = 0; i < 2004; i++) {
			message.push_back( 't' );
		}
		message.push_back( '\"' );
		g_theNetSystem->Send( message );

		return true;
	}
	return false;
#else
	return true;
#endif
}

DevConsole::DevConsoleLine::DevConsoleLine( std::string const& text, Rgba8 const& color, int frameNum, bool isInstruction )
	:m_color(color)
	,m_text(text)
	,m_frameNum(frameNum)
	,m_secondsFromBegin(GetCurrentTimeSeconds())
	,m_isInstruction(isInstruction)
{
}

