#pragma once


#include<NetDef.h>
#include<NetMgrDef.h>

#include<memory>

class Acceptor;

struct AcceptOverlapped :public Overlapped
{
	Acceptor* m_Acceptor;
};

struct Acceptor 
{
	friend class IocpNetMgr;
public:
	Acceptor();
	~Acceptor();

	bool Init(NetMgr* mgr);

	void OnMessage(bool bSucc,CompletionKey key, UInt32 size);

	void Clear();

	NetMgr* m_NetMgr;
	SocketFd m_ListenFd;

	SOCKADDR_IN		m_addr;
	SocketFd		m_SockFd;
	char			m_Buffer[256];
	AcceptOverlapped  m_Overlapped;
	//OverlappedWrapper<AcceptHandler> m_Overlapped;
};
