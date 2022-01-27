#include "IocpNetMgr.h"
#ifdef WINDOWS_FLAG

#include <iostream>
#include <ws2tcpip.h>
bool IocpNetMgr::Initialize(NetCallBack* call, Int32 iMaxThread)
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
	//��ȡAcceptEx����ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	if (WSAIoctl(m_ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &BytesReturned, NULL, NULL) == SOCKET_ERROR)
	{
		return false;
	}

	//��ȡGetAcceptexSockAddrs����ָ��
	GUID GuidGetAcceptexSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (WSAIoctl(m_ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptexSockAddrs, sizeof(GuidGetAcceptexSockAddrs), &m_lpfnGetAcceptSockAddrs, sizeof(m_lpfnGetAcceptSockAddrs), &BytesReturned, NULL, NULL) == SOCKET_ERROR )
	{
		return false;
	}

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	for (UInt32 i = 0; i < info.dwNumberOfProcessors * 2; ++i)
	{
		std::thread* th = new std::thread(std::bind(&IocpNetMgr::WorkerProc, this));
		m_WorkerThread.push_back(th);
	}

	//���²�����Ϊiocp��epoll�Ĳ����ԡ��������ó���д��NetMgr��
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

	m_Acceptor.m_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, "0.0.0.0", &m_Acceptor.m_addr.sin_addr.s_addr) < 0)
	{
		std::cout << "inet pton err : errcode = " << sock_error() << std::endl;
		return false;
	}
	m_Acceptor.m_addr.sin_port = htons(1234);

	if (bind(m_Acceptor.m_ListenFd, (sockaddr*)&m_Acceptor.m_addr, sizeof(m_Acceptor.m_addr)) != 0)
	{
		m_Acceptor.Clear();
		return false;
	}

	if (listen(m_Acceptor.m_ListenFd, SOMAXCONN) != 0)
	{
		m_Acceptor.Clear();
		return false;
	}

	if (CreateIoCompletionPort((HANDLE)m_Acceptor.m_ListenFd, m_IocpHandle, CompletionKey::CK_ACCPET, 0) == NULL)
	{
		m_Acceptor.Clear();
		return false;
	}

	m_Acceptor.m_ReqType = TagReqHandle::TRH_ACCEPT;
	m_Acceptor.m_SockFd = C_INVALID_SOCKET;
	memset(m_Acceptor.m_Buffer, 0, sizeof(m_Acceptor.m_Buffer));

	m_Acceptor.m_SockFd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Acceptor.m_SockFd == C_INVALID_SOCKET)
	{
		return false;
	}
	DWORD xx = 0;
	if (!m_lpfnAcceptEx(m_Acceptor.m_ListenFd, m_Acceptor.m_SockFd, m_Acceptor.m_Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &xx, m_Acceptor))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			return false;
		}
	}
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
}

void IocpNetMgr::PostAccept()
{
	//PostQueuedCompletionStatus(m_IocpHandle, 0, CK_ACCPET, m_Acceptor);
}


#endif // WINDOWS_FLAG