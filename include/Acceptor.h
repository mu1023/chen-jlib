#pragma once


#include<NetDef.h>
#include<NetMgrDef.h>

#include<memory>

class Acceptor;
/*
struct AcceptHandler : IOCPHandler
{
public:
	void OnMessage(ULONG_PTR key, UInt32 size)override;

	void Clear()override;

	Acceptor* m_Acceptor;
};*/

struct Acceptor : IOCPHandler
{
	friend class IocpNetMgr;
public:
	Acceptor();
	~Acceptor();

	bool Init(NetMgr* mgr);

	void OnMessage(CompletionKey key, UInt32 size)override;

	void Clear()override;

	NetMgr* m_NetMgr;
	SocketFd m_ListenFd;

	SOCKADDR_IN		m_addr;
	SocketFd		m_SockFd;
	char			m_Buffer[256];
	TagReqHandle    m_ReqType;
	//OverlappedWrapper<AcceptHandler> m_Overlapped;
};
