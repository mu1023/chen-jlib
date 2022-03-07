#pragma once
#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include<NetDef.h>
#include<NetMgrDef.h>

#include<memory>

class Acceptor;

struct AcceptOverlapped :public Overlapped
{
	Acceptor*		m_Acceptor;
	SocketFd		m_SockFd;
	char			m_Buffer[256];
};

struct Acceptor 
{
	friend class IocpNetMgr;
public:
	Acceptor();
	~Acceptor();

	bool Init(NetMgr* mgr);

	void Clear();

	NetMgr* m_NetMgr;
	SocketFd m_ListenFd;

	SOCKADDR_IN		m_addr;
	
	std::vector<AcceptOverlapped>  m_Overlapped;
	//OverlappedWrapper<AcceptHandler> m_Overlapped;
};
#endif _ACCEPTOR_H_
