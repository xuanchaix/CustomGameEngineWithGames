#include "Engine/NetSystem/NetSystem.hpp"

#ifdef ENGINE_ENABLE_NET_SYSTEM

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"

#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


NetSystem::NetSystem( NetSystemConfig const& nConfig )
	:m_config( nConfig )
{
}

NetSystem::~NetSystem()
{
	//free( m_recvBuffer );
}

void NetSystem::StartUp()
{
	m_recvBuffer = (char*)malloc( m_config.m_recvBufferSize );
	//m_sendBuffer = (char*)malloc( m_config.m_sendBufferSize );
	if (m_config.m_modeString == "Client" || m_config.m_modeString == "client") {
		m_mode = NetMode::CLIENT;
		g_devConsole->AddLine( Rgba8( 255, 255, 255 ), "Client" );
	}
	else if (m_config.m_modeString == "Server" || m_config.m_modeString == "server") {
		m_mode = NetMode::SERVER;
		g_devConsole->AddLine( Rgba8( 255, 255, 255 ), "Server" );
	}
	if (m_mode == NetMode::CLIENT) {
		// Startup Windows sockets
		WSADATA data;
		WSAStartup( MAKEWORD( 2, 2 ), &data );

		// Create client socket
		CreateClientSocket();
	}
	else if (m_mode == NetMode::SERVER) {
		// Startup Windows sockets.
		WSADATA data;
		int result = WSAStartup( MAKEWORD( 2, 2 ), &data );

		// Create listen socket.
		m_listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if (m_listenSocket == INVALID_SOCKET) {
			WSACleanup();
			ERROR_AND_DIE( Stringf( "Error at socket(): %ld\n", WSAGetLastError() ) );
		}

		// Set blocking mode.
		unsigned long blockingMode = 1;
		result = ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );

		// Get host port from string.
		Strings IPAndPort;
		SplitStringOnDelimiter( IPAndPort, m_config.m_hostAddressString, ':' );
		m_hostAddress = INADDR_ANY;
		m_hostPort = (unsigned short)(atoi( IPAndPort[1].c_str() ));

		// Bind the listen socket to a port.
		sockaddr_in addr = { };
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl( m_hostAddress );
		addr.sin_port = htons( m_hostPort );
		result = bind( m_listenSocket, (sockaddr*)&addr, (int)sizeof( addr ) );
		if (result == SOCKET_ERROR) {
			WSACleanup();
			ERROR_AND_DIE( Stringf( "bind failed with error: %d\n", WSAGetLastError() ) );
		}

		// Listen for connections to accept.
		result = listen( m_listenSocket, SOMAXCONN );
	}
}

void NetSystem::BeginFrame()
{
	if (m_mode == NetMode::SERVER) {
		/*RemoteEcho Message="Command_echo Message=hello"*/
		//m_sendQueue.push_back( std::string( "Command_echo EchoArg=hello" ) );
	}
	if (m_mode == NetMode::CLIENT) {
		// Check if our connection attempt has completed.
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl( m_hostAddress );
		addr.sin_port = htons( m_hostPort );
		int result = connect( m_clientSocket, (sockaddr*)(&addr), (int)sizeof( addr ) );

		fd_set writeSockets;
		fd_set exceptSockets;
		FD_ZERO( &writeSockets );
		FD_ZERO( &exceptSockets );
		FD_SET( m_clientSocket, &writeSockets );
		FD_SET( m_clientSocket, &exceptSockets );
		timeval waitTime = { };
		result = select( 0, NULL, &writeSockets, &exceptSockets, &waitTime );

		// We are connected if the following is true.
		if (result >= 0) {
			if(FD_ISSET( m_clientSocket, &writeSockets ) ) {
				// Send and receive if we are connected.
				m_clientState = ClientState::Connected;
				if (!ProcessMessage()) {
					goto PrintState;
				}
			}
			else if (FD_ISSET( m_clientSocket, &exceptSockets )) {
				// Attempt to connect if we haven't already.
				addr.sin_family = AF_INET;
				addr.sin_addr.S_un.S_addr = htonl( m_hostAddress );
				addr.sin_port = htons( m_hostPort );
				result = connect( m_clientSocket, (sockaddr*)(&addr), (int)sizeof( addr ) );
				if (result <= 0) {
					int error = WSAGetLastError();
					if (error == WSAEWOULDBLOCK || error == WSAEALREADY) {
						goto PrintState;
					}
				}

				// Check if our connection attempt failed.
				FD_ZERO( &exceptSockets );
				FD_SET( m_clientSocket, &exceptSockets );
				waitTime = { };
				result = select( 0, NULL, &writeSockets, &exceptSockets, &waitTime );

				// The connection failed if the following is true, in which case we need to connect again.
				if (result > 0 && FD_ISSET( m_clientSocket, &exceptSockets ) && FD_ISSET( m_clientSocket, &writeSockets )) {
					m_clientState = ClientState::Connected;
				}
			}
			else {
				if (DealWithError()) {
					goto PrintState;
				}
			}
		}
		else if (result < 0) {
			if (DealWithError()) {
				goto PrintState;
			}
		}

PrintState:
		if (m_lastFrameClientState == ClientState::Disconnected && m_clientState == ClientState::Connected) {
			g_devConsole->AddLine( Rgba8( 255, 255, 255 ), Stringf( "Connected to Server %s! Socket: %lld", m_config.m_hostAddressString.c_str(), m_clientSocket ) );
		}
		else if (m_lastFrameClientState == ClientState::Connected && m_clientState == ClientState::Disconnected) {
			g_devConsole->AddLine( Rgba8( 255, 255, 255 ), Stringf( "DisConnected from Server %s! Socket: %lld", m_config.m_hostAddressString.c_str(), m_clientSocket ) );
		}
		m_lastFrameClientState = m_clientState;
	}
	else if (m_mode == NetMode::SERVER) {
		// If we do not have a connection, check for connections to accept.
		if (m_clientSocket == INVALID_SOCKET) {
			m_clientSocket = accept( m_listenSocket, NULL, NULL );
			if (m_clientSocket != INVALID_SOCKET) {
				// If a connection is accepted set blocking mode.
				unsigned long blockingMode = 1;
				ioctlsocket( m_clientSocket, FIONBIO, &blockingMode );
				g_devConsole->AddLine( Rgba8( 255, 255, 255 ), Stringf( "Connected to Client! Socket: %lld", m_clientSocket ) );
			}
		}
		else{
			//g_devConsole->AddLine( Rgba8( 255, 255, 255 ), "Server Connected to Client!" );
			if (!ProcessMessage()) {
				return;
			}
		}

	}
}

void NetSystem::EndFrame()
{

}

void NetSystem::ShutDown()
{
	BeginFrame();
	if (m_mode == NetMode::CLIENT) {
		// Close all open sockets.
		shutdown( m_clientSocket, SD_BOTH );
		closesocket( m_clientSocket );
		// Shutdown Windows sockets.
		WSACleanup();
	}
	else if (m_mode == NetMode::SERVER) {
		// Close all open sockets.
		shutdown( m_clientSocket, SD_BOTH );
		shutdown( m_listenSocket, SD_BOTH );
		closesocket( m_clientSocket );
		closesocket( m_listenSocket );

		// Shutdown Windows sockets.
		WSACleanup();
	}
	free( m_recvBuffer );
	//free( m_sendBuffer );
}

bool NetSystem::IsEnabled() const
{
	return m_clientState != ClientState::Disabled;
}

bool NetSystem::IsServer() const
{
	return m_mode == NetMode::SERVER;
}

bool NetSystem::IsClient() const
{
	return m_mode == NetMode::CLIENT;
}

bool NetSystem::IsConnected() const
{
	return m_clientState == ClientState::Connected;
}

void NetSystem::Send( std::string const& data )
{
	m_sendQueue.push_back( data );
}

NetMode NetSystem::GetNetMode() const
{
	return m_mode;
}

void NetSystem::ExecuteRecvMessage( std::string const& message )
{
	g_devConsole->Execute( message, true );
	/*Strings commandLineSplit;
	int numOfStrs = SplitStringOnDelimiter( commandLineSplit, message, ' ' );
	if (numOfStrs == 2) {
		NamedStrings args;
		Strings keyValuePair;
		int lengthOfPair = SplitStringOnDelimiter( keyValuePair, commandLineSplit[1], '=' );
		if (lengthOfPair == 2) {
			args.SetValue( keyValuePair[0], keyValuePair[1] );
		}
		FireEvent( commandLineSplit[0], args );
	}*/
}

bool NetSystem::DealWithError()
{
	int error = WSAGetLastError();
	if (error == WSAECONNABORTED || error == WSAECONNRESET || error == 0) {
		if (m_mode == NetMode::SERVER) {
			g_devConsole->AddLine( Rgba8( 255, 255, 255 ), Stringf( "Disconnected from Client! Socket: %lld", m_clientSocket ) );
			shutdown( m_clientSocket, SD_BOTH );
			closesocket( m_clientSocket );
			m_clientSocket = INVALID_SOCKET;
		}
		else if (m_mode == NetMode::CLIENT) {
			m_clientState = ClientState::Disconnected;
			shutdown( m_clientSocket, SD_BOTH );
			closesocket( m_clientSocket );
			// remake a socket
			CreateClientSocket();
		}
	}
	else if (error == WSAEWOULDBLOCK || error == WSAEALREADY) {
		return true;
	}
	else {
		g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "Error code: %d", error ) );
	}
	return false;
	//ERROR_AND_DIE( "Error in Receiving messages" );
}

void NetSystem::CreateClientSocket()
{
	m_clientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (m_clientSocket == INVALID_SOCKET) {
		WSACleanup();
		ERROR_AND_DIE( Stringf( "Error at socket(): %ld\n", WSAGetLastError() ) );
	}

	// Set blocking mode
	unsigned long blockingMode = 1;
	int result = ioctlsocket( m_clientSocket, FIONBIO, &blockingMode );

	// Get host address from string.
	Strings IPAndPort;
	SplitStringOnDelimiter( IPAndPort, m_config.m_hostAddressString, ':' );
	IN_ADDR addr = { };
	result = inet_pton( AF_INET, IPAndPort[0].c_str(), &addr );
	//if (result != 0) {
	//	WSACleanup();
	//	ERROR_AND_DIE( Stringf( "Error IP Address: %ld\n", WSAGetLastError() ) );
	//}
	m_hostAddress = ntohl( addr.S_un.S_addr );

	// Get host port from string
	m_hostPort = (unsigned short)(atoi( IPAndPort[1].c_str() ));
}

bool NetSystem::ProcessMessage()
{
	// Send and receive if we are connected.
	int result;
	while (m_sendQueue.size() > 0) {
		m_sendBuffer = const_cast<char*>(m_sendQueue[0].c_str());
		result = send( m_clientSocket, m_sendBuffer, (int)strlen(m_sendBuffer) + 1, 0);
		if (result > 0) {
			m_sendQueue.pop_front();
		}
		else {
			if (DealWithError()) {
				return false;
			}
		}
		/*size_t lengthOfBuffer = (size_t)m_config.m_sendBufferSize;
		if (m_sendQueue[0].size() > lengthOfBuffer) {
			memcpy( m_sendBuffer, m_sendQueue[0].c_str(), lengthOfBuffer );
			result = send( m_clientSocket, m_sendBuffer, (int)lengthOfBuffer, 0 );
			if (result > 0) {
				m_sendQueue[0] = m_sendQueue[0].substr( lengthOfBuffer );
			}
			else {
				if (DealWithError()) {
					return false;
				}
			}
		}
		else {
			memcpy( m_sendBuffer, m_sendQueue[0].c_str(), m_sendQueue[0].size() );
			result = send( m_clientSocket, m_sendBuffer, (int)m_sendQueue[0].size() + 1, 0 );
			if (result > 0) {
				m_sendQueue.pop_front();
			}
			else {
				if (DealWithError()) {
					return false;
				}
			}
		}*/
	}
	//memset( m_recvBuffer, '\0', m_config.m_recvBufferSize);
	result = recv( m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0 );
	if (result > 0) {
		Strings lines;
		bool hasStringInIt = false;
		int lastStrEnd = 0;
		for (int i = 0; i < result; i++) {
			if (m_recvBuffer[i] == '\0') {
				hasStringInIt = true;
				lines.push_back( std::string( &m_recvBuffer[lastStrEnd] ) );
				lastStrEnd = i + 1;
			}
		}
		if (!hasStringInIt) {
			m_recvQueue += std::string( m_recvBuffer ).substr( 0, m_config.m_recvBufferSize );
		}
		else if(lastStrEnd < result) {
			lines.push_back( std::string( &m_recvBuffer[lastStrEnd] ).substr( 0, m_config.m_recvBufferSize - lastStrEnd ) );
		}

		for (int i = 0; i < (int)lines.size(); i++) {
			if (!m_recvQueue.empty() && i == 0) {
				m_recvQueue += lines[i];
				// execute order
				ExecuteRecvMessage( m_recvQueue );
				m_recvQueue.clear();
			}
			else if (i == (int)(lines.size()) - 1 && lastStrEnd < result) {
				m_recvQueue += lines[i];
			}
			else {
				// execute order
				ExecuteRecvMessage( lines[i] );
			}
		}
		//}
	}
	else {
		if (DealWithError()) {
			return false;
		}
	}
	return true;
}

#endif //  ENGINE_ENABLE_NET_SYSTEM
