#pragma once
#include<NetDef.h>
#include<NetMgrDef.h>

struct Connector;

struct Connector : public std::enable_shared_from_this<Connector>
{
public:
	const UInt32 WRITE_BUFFER_SIZE = 10000;
	const UInt32 READ_BUFFER_SIZE = 10000;


	Connector(SocketFd fd) :m_SocketFd(fd) {};
	~Connector() {};

	SocketFd GetFd() { return m_SocketFd; };

	//UInt32 Write(const char* msg, UInt32 len);

	//virtual void Close();

	SocketFd					m_SocketFd;

	NetBuffer					m_RecvBuffer;
	//OverlappedWrapper<ConnectorHandler>			m_RecvHandler;

	NetBuffer					m_SendBuffer;
	//OverlappedWrapper<ConnectorHandler>			m_SendHandler;


	std::mutex				    m_Mutex;
	//NetConnectionStatus			m_Status;
};