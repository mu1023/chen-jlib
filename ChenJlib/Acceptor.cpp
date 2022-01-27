#include "Acceptor.h"
#include <assert.h>

Acceptor::Acceptor()
{
	m_ListenFd = C_INVALID_SOCKET;
	m_SockFd = C_INVALID_SOCKET;
}

Acceptor::~Acceptor()
{
}

bool Acceptor::Init(NetMgr* mgr)
{
	if (!mgr) 
	{
		return false;
	}
	m_NetMgr = mgr;
	return true;
}

void Acceptor::OnMessage(CompletionKey key, UInt32 size)
{
	if (setsockopt(m_SockFd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_ListenFd, sizeof(m_ListenFd)) != 0)
	{
		return;
	}
	int bTrue = 1;
	if (setsockopt(m_SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&bTrue, sizeof(bTrue)) != 0)
	{
		return;
	}
	//m_NetMgr->
}

void Acceptor::Clear()
{
	if (m_ListenFd != C_INVALID_SOCKET) {
		closesocket(m_ListenFd);
	}
	if (m_SockFd != C_INVALID_SOCKET) {
		closesocket(m_SockFd);
	}
}
