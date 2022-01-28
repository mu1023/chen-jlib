#include "IocpNetMgr.h"
#ifdef WINDOWS_FLAG

#include <iostream>
#include <ws2tcpip.h>
bool IocpNetMgr::Initialize(NetCallBack* call, Int32 iMaxThread, const char* ip, UInt16 port)
{
	if (call == nullptr) {
		return false;
	}
	m_Call = call;

	if (iMaxThread < 1) {
		iMaxThread = 1;
	}
	if (iMaxThread >= MAX_THREAD_NUM) {
		iMaxThread = MAX_THREAD_NUM;
	}

	m_ListenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_IocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (nullptr == m_IocpHandle)
	{
		return false;
	}
	
	DWORD BytesReturned = 0;
	//获取AcceptEx函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	if (WSAIoctl(m_ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &BytesReturned, NULL, NULL) == SOCKET_ERROR)
	{
		return false;
	}

	//获取GetAcceptexSockAddrs函数指针
	GUID GuidGetAcceptexSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (WSAIoctl(m_ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptexSockAddrs, sizeof(GuidGetAcceptexSockAddrs), &m_lpfnGetAcceptSockAddrs, sizeof(m_lpfnGetAcceptSockAddrs), &BytesReturned, NULL, NULL) == SOCKET_ERROR )
	{
		return false;
	}

	
	for (UInt32 i = 0; i < iMaxThread; ++i)
	{
		std::thread* th = new std::thread(std::bind(&IocpNetMgr::WorkerProc, this));
		m_WorkerThread.push_back(th);
	}

	//以下部分因为iocp与epoll的差异性。。所以拿出来写在NetMgr吧
	m_Acceptor.m_ListenFd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Acceptor.m_ListenFd == C_INVALID_SOCKET)
	{
		return false;
	}

	Int32 reuse = 1;
	if (setsockopt(m_Acceptor.m_ListenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(Int32)) != 0)
	{
		m_Acceptor.Clear();
		return false;
	}

	//非阻塞
	u_long val = 1;
	ioctlsocket(m_Acceptor.m_ListenFd, FIONBIO, &val);

	m_Acceptor.m_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip, &m_Acceptor.m_addr.sin_addr.s_addr) < 0)
	{
		std::cout << "inet pton err : errcode = " << sock_error() << std::endl;
		return false;
	}
	m_Acceptor.m_addr.sin_port = htons(port);

	if (bind(m_Acceptor.m_ListenFd, (sockaddr*)&m_Acceptor.m_addr, sizeof(m_Acceptor.m_addr)) != 0)
	{
		m_Acceptor.Clear();
		return false;
	}

	unsigned long val = 1;
	int nb = ioctlsocket(m_Acceptor.m_ListenFd, FIONBIO, &val);
	if (nb != NO_ERROR)
	{
		return false;
	}

	if (listen(m_Acceptor.m_ListenFd, 128) != 0)
	{
		m_Acceptor.Clear();
		return false;
	}

	if (CreateIoCompletionPort((HANDLE)m_Acceptor.m_ListenFd, m_IocpHandle, CompletionKey::CK_ACCPET, 0) == NULL)
	{
		m_Acceptor.Clear();
		return false;
	}

	PostAccept();
	return true;
}

void IocpNetMgr::Upadete()
{
	
}

void IocpNetMgr::Finialize()
{
}

void IocpNetMgr::WorkerProc()
{
	while (true)
	{
		DWORD num = 0;
		ULONG_PTR comKey = 0;
		LPOVERLAPPED overLapped = nullptr;
		bool bRet = GetQueuedCompletionStatus(m_IocpHandle, &num, &comKey, &overLapped, 500);

		if (!bRet) {
			continue;
		}
		CompletionKey cKey = (CompletionKey)comKey;

		if ((CompletionKey)comKey == CompletionKey::CK_THREAD_CLOSE)
		{
			return;
		}

		if (overLapped != nullptr)
		{
			((Overlapped*)overLapped)->handler->OnMessage(bRet, (CompletionKey)comKey, num);
		}
		if ((CompletionKey)comKey == CompletionKey::CK_ACCPET)
		{
			PostAccept();
			continue;
		}
		switch (((Overlapped*)overLapped)->tagReqHandle)
		{
		case TRQ_NONE:
			break;
		case TRH_SEND:
			break;
		case TRH_RECV:
			break;
		default:
			break;
		}

	}
}

void IocpNetMgr::PostAccept()
{
	int t = 10;
	while (t--)
	{
		m_Acceptor.m_SockFd = C_INVALID_SOCKET;
		memset(m_Acceptor.m_Buffer, 0, sizeof(m_Acceptor.m_Buffer));

		m_Acceptor.m_SockFd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_Acceptor.m_SockFd == C_INVALID_SOCKET)
		{
			Sleep(1);
			break;
		}
		DWORD bytes = 0;
		if (!m_lpfnAcceptEx(m_Acceptor.m_ListenFd, m_Acceptor.m_SockFd, m_Acceptor.m_Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, m_Acceptor))
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				closesocket(m_Acceptor.m_SockFd);
				m_Acceptor.m_SockFd = C_INVALID_SOCKET;
				Sleep(1);
				continue;
			}
		}
		return;
	}
	//error
	return;
}

#endif // WINDOWS_FLAG