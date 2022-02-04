#pragma once
#include<NetDef.h>
#include<NetMgrDef.h>

struct Connector;

enum NetConnectionStatus
{
	NCS_CLOSE = 0,
	NCS_NORMAL = 1,
};

struct ConnectorOverlapped :public Overlapped
{
	WSABUF	   buf;
	Connector* m_Connector ;
};

struct Connector : public std::enable_shared_from_this<Connector>
{
public:
	const UInt32 WRITE_BUFFER_SIZE = 10000;
	const UInt32 READ_BUFFER_SIZE = 10000;


	Connector() :m_AllocID(0),m_SocketFd(C_INVALID_SOCKET),m_Status(NetConnectionStatus::NCS_CLOSE) {};
	~Connector() {};

	void Init(SocketFd fd, NetConnectionStatus status, sockaddr* addr);

	SocketFd GetFd() { return m_SocketFd; };

	//UInt32 Write(const char* msg, UInt32 len);
	void SetAllocID(UInt32 allocID){ m_AllocID = allocID;}

	void Close();

	UInt32						m_AllocID;
	SocketFd					m_SocketFd;

	std::mutex				    m_RecvMutex;
	NetBuffer					m_RecvBuffer;
	ConnectorOverlapped			m_RecvOverlapped;

	std::mutex				    m_SendMutex;
	NetBuffer					m_SendBuffer;
	ConnectorOverlapped			m_SendOverlapped;

	sockaddr_in m_Adder;
	std::mutex				    m_Mutex;
	NetConnectionStatus			m_Status;
};