#include "Connector.h"

void Connector::Init(SocketFd fd, NetConnectionStatus status, sockaddr* addr)
{
	m_SocketFd = fd;
	m_Status = status;
	memcpy(&m_Adder, addr, sizeof(m_Adder));
}
