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

void Acceptor::Clear()
{
	if (m_ListenFd != C_INVALID_SOCKET) {
		closesocket(m_ListenFd);
	}
	if (m_SockFd != C_INVALID_SOCKET) {
		closesocket(m_SockFd);
	}
}
