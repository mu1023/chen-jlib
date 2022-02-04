#include "IocpNetMgr.h"
#ifdef WINDOWS_FLAG

#include <iostream>
#include <ws2tcpip.h>
IocpNetMgr::IocpNetMgr()
{
	m_allocID = 0;
	m_ListenSock = C_INVALID_SOCKET;
	m_IocpHandle = 0;

}
IocpNetMgr::~IocpNetMgr()
{
}
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
	unsigned long val = 1;
	if (!ioctlsocket(m_Acceptor.m_ListenFd, FIONBIO, &val))
	{
		return false;
	}

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

	PostAccept(&m_Acceptor);
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
		else if ((CompletionKey)comKey == CompletionKey::CK_ACCPET)
		{
			Acceptor* pAcceptor= ((AcceptOverlapped*)overLapped)->m_Acceptor;
			OnAccept(pAcceptor);
			PostAccept(pAcceptor);
		}
		else if ((CompletionKey)comKey == CompletionKey::CK_CONNECT) 
		{

			switch (((Overlapped*)overLapped)->tagReqHandle)
			{
			case TRH_SEND:
			{
				Connector* pConnector = ((ConnectorOverlapped*)overLapped)->m_Connector;
				if (pConnector)
				{
					pConnector->m_SendBuffer.Retrieve(num);
					if (pConnector->m_SendBuffer.ReadableBytes() > 0)
					{
						pConnector->m_SendOverlapped.buf.buf = pConnector->m_SendBuffer.BeginRead();
						pConnector->m_SendOverlapped.buf.len = pConnector->m_SendBuffer.ReadableBytes();
						DWORD sendBytes = 0;
						if (WSASend(pConnector->m_SocketFd, &pConnector->m_SendOverlapped.buf, 1, &sendBytes, 0, &(pConnector->m_SendOverlapped), nullptr) == C_SOCK_ERROR)
						{
							if (WSAGetLastError() != ERROR_IO_PENDING)
							{
								std::cout << "WSASend() failed. Error:" << GetLastError() << std::endl;
								pConnector->Close();
								PushCloseConn(pConnector->m_AllocID); 
							}
						}
					}
				}
				
			}
			break;
			case TRH_RECV:
			{

			}
			break;
			case TRQ_NONE:
			default:
				break;
			}
		}

	}
}

void IocpNetMgr::PostAccept(Acceptor* acceptor)
{
	if (acceptor == nullptr) {
		return;
	}
	/*if (acceptor->m_SockFd != C_INVALID_SOCKET) {
		//PushConnect(acceptor->m_SockFd);
	}*/
	int t = 10;
	while (t--)
	{
		acceptor->m_SockFd = C_INVALID_SOCKET;
		memset(acceptor->m_Buffer, 0, sizeof(acceptor->m_Buffer));

		acceptor->m_SockFd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (acceptor->m_SockFd == C_INVALID_SOCKET)
		{
			Sleep(1);
			break;
		}
		DWORD bytes = 0;
		if (!m_lpfnAcceptEx(acceptor->m_ListenFd, acceptor->m_SockFd, acceptor->m_Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, &acceptor->m_Overlapped))
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				closesocket(acceptor->m_SockFd);
				acceptor->m_SockFd = C_INVALID_SOCKET;
				Sleep(1);
				continue;
			}
		}
		return;
	}
	//error
	return;
}

void IocpNetMgr::OnAccept(Acceptor* acceptor)
{
	if (acceptor->m_SockFd == C_INVALID_SOCKET) {
		return;
	}

	if (setsockopt(acceptor->m_SockFd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&acceptor->m_ListenFd, sizeof(acceptor->m_ListenFd)) != 0)
	{
		closesocket(acceptor->m_SockFd);
		return;
	}
	int flag = 1;
	if (setsockopt(acceptor->m_SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) != 0)
	{
		closesocket(acceptor->m_SockFd);
		return;
	}


	sockaddr* paddr1 = NULL;
	sockaddr* paddr2 = NULL;
	int tmp1 = 0;
	int tmp2 = 0;
	m_lpfnGetAcceptSockAddrs(acceptor->m_Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &paddr1, &tmp1, &paddr2, &tmp2);

		

	PushData* data = new PushData();
	data->pConnector = std::make_shared<Connector>();
	data->pConnector->Init(acceptor->m_SockFd, NetConnectionStatus::NCS_NORMAL,paddr2);
	data->pData = nullptr;

	int allocID = m_allocID.fetch_add(1);
	data->pConnector->SetAllocID(allocID);
	m_ConnMutex.lock();
	m_Connectors[allocID] = data->pConnector;
	m_ConnMutex.unlock();

	m_PushMutex.lock();
	m_PushDatas.push_back(data);
	m_PushMutex.unlock();
}

void IocpNetMgr::PushCloseConn(UInt32 allocID)
{
}

#endif // WINDOWS_FLAG