#include "Connector.h"

void Connector::Init(SocketFd fd, NetConnectionStatus status, sockaddr* addr)
{
	m_SocketFd = fd;
	m_Status = status;
	memcpy(&m_Adder, addr, sizeof(m_Adder));
}

void Connector::Close()
{
	if (m_Status != NCS_NORMAL)
	{
		return;
	}
	CancelIo((HANDLE)m_SocketFd);
	shutdown(m_SocketFd, SD_BOTH);
	closesocket(m_SocketFd);
	m_SocketFd = INVALID_SOCKET;
}
