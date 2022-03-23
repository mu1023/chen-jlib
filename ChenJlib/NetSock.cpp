#include "NetSock.h"

namespace cj
{

	Acceptor::Acceptor()
	{
		m_ListenFd = C_INVALID_SOCKET;
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
		if (m_ListenFd != C_INVALID_SOCKET)
		{
			closesocket(m_ListenFd);
		}
		/*if (m_SockFd != C_INVALID_SOCKET) {
			closesocket(m_SockFd);
		}*/
	}


	ConnSock::ConnSock() :m_AllocID(0), m_SocketFd(C_INVALID_SOCKET), m_Status(NetConnectionStatus::NCS_CLOSE), m_RecvBuffer(8192), m_SendBuffer(8192)
	{
		m_SendIoContext = nullptr;
		memset(&m_Adder, 0, sizeof(m_Adder));

	}

	ConnSock::~ConnSock()
	{
		Close();
	}

	void ConnSock::Init(SocketFd fd, NetConnectionStatus status, sockaddr* addr, ConnType connType)
	{
		m_SocketFd = fd;
		m_Status = status;
		m_ConnType = connType;
		memcpy(&m_Adder, addr, sizeof(m_Adder));
	}

	void ConnSock::Close()
	{
		std::lock_guard<std::mutex> lck(m_StatusMutex);
		if (m_Status != NCS_NORMAL)
		{
			return;
		}
		m_Status = NCS_CLOSE;

		CancelIo((HANDLE)m_SocketFd);
		shutdown(m_SocketFd, SD_BOTH);
		closesocket(m_SocketFd);
		m_SocketFd = C_INVALID_SOCKET;
	}

	bool ConnSock::IsClose()
	{
		return m_Status == NCS_CLOSE;
	}
}
