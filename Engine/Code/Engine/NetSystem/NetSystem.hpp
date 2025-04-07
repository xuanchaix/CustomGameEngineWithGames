#pragma once

#include "Engine/Core/EngineCommon.hpp"

#ifdef ENGINE_ENABLE_NET_SYSTEM

#include <string>
#include <deque>

struct NetSystemConfig {
	std::string m_modeString = "None";
	std::string m_hostAddressString = "127.0.0.1:23456";
	int m_sendBufferSize = 2048;
	int m_recvBufferSize = 2048;
};

enum class NetMode {
	NONE = 0,
	CLIENT,
	SERVER,
};

enum class ClientState {
	Disconnected,
	Connected,
	Disabled,
};

class NetSystem {
public:
	NetSystem( NetSystemConfig const& nConfig );
	~NetSystem();
	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	bool IsEnabled() const;
	bool IsServer() const;
	bool IsClient() const;
	bool IsConnected() const;
	void Send( std::string const& data );
	NetMode GetNetMode() const;

protected:
	void ExecuteRecvMessage( std::string const& message );
	bool DealWithError();
	void CreateClientSocket();
	bool ProcessMessage();
protected:
	NetMode m_mode = NetMode::NONE;
	NetSystemConfig m_config;
	ClientState m_clientState = ClientState::Disconnected;
	ClientState m_lastFrameClientState = ClientState::Disconnected;
	uintptr_t m_clientSocket = (uintptr_t)~0ull;
	uintptr_t m_listenSocket = (uintptr_t)~0ull;
	unsigned long m_hostAddress = 0;
	unsigned short m_hostPort = 0;
	char* m_sendBuffer = nullptr;
	char* m_recvBuffer = nullptr;
	std::deque<std::string> m_sendQueue;
	std::string m_recvQueue;
};

#endif //  ENGINE_ENABLE_NET_SYSTEM