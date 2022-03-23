#include "IocpNetMgr.h"
#ifdef WINDOWS_FLAG
#include <Logging.h>
#include <iostream>
#include <ws2tcpip.h>

namespace cj
{

	IocpNetMgr::IocpNetMgr()
	{
		m_allocID = 0;
		m_IocpHandle = 0;
		m_Acceptor.m_NetMgr = this;;

	}
	IocpNetMgr::~IocpNetMgr()
	{
	}
	bool IocpNetMgr::Initialize(NetCallBack* call, Int32 iMaxThread)
	{
		if (call == nullptr)
		{
			return false;
		}
		m_Call = call;

		if (iMaxThread < 1)
		{
			iMaxThread = 1;
		}
		if (iMaxThread >= MAX_THREAD_NUM)
		{
			iMaxThread = MAX_THREAD_NUM;
		}

		m_IocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (nullptr == m_IocpHandle)
		{
			return false;
		}

		for (Int32 i = 0; i < iMaxThread; ++i)
		{
			std::thread* th = new std::thread(std::bind(&IocpNetMgr::WorkerProc, this));
			m_WorkerThread.push_back(th);
		}


		return true;
	}

	void IocpNetMgr::Upadete()
	{
		std::vector<PushData*> m_Datas;
		{
			std::lock_guard<std::mutex> lck(m_PushMutex);
			m_Datas.swap(m_PushDatas);
		}

		for (auto pData : m_Datas)
		{
			if (pData == nullptr)
			{
				continue;
			}

			switch (pData->eType)
			{

			case PDT_NEW_CONNECT:
			{
				m_Call->OnNetConneted(pData->pConnSock);
			}
			break;
			case PDT_RECV_DATA:
			{
				m_Call->OnNetRecv(pData->iAllocID, pData->kNetBuff);
			}
			break;
			case PDT_CONN_CLOSE:
			{
				m_Call->OnNetDisConnected(pData->iAllocID);
			}
			break;
			case PDT_NONE:
			default:
				break;
			}
			RecyclePushData(pData);
		}
		m_Datas.clear();

	}

	void IocpNetMgr::Finialize()
	{
	}

	bool IocpNetMgr::Listen(Int32 iListenPort, Int32 iListenNum)
	{
		//暂时一个IocpNetMgr只监听一个吧
		if (m_Acceptor.m_ListenFd != C_INVALID_SOCKET)
		{
			return false;
		}

		if (iListenNum < 0)
		{
			iListenNum = 1;
		}
		else if (iListenNum > 128)
		{
			iListenNum = 128;
		}
		m_Acceptor.m_ListenFd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (m_Acceptor.m_ListenFd == C_INVALID_SOCKET)
		{

			LOGGER_ERROR("error = {0}", sock_error());
			return false;
		}

		Int32 reuse = 1;
		if (setsockopt(m_Acceptor.m_ListenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(Int32)) != 0)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			m_Acceptor.Clear();
			return false;
		}

		//非阻塞
		/*unsigned long val = 1;
		if (ioctlsocket(m_Acceptor.m_ListenFd, FIONBIO, &val) != 0)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			return false;
		}*/

		m_Acceptor.m_addr.sin_family = AF_INET;
		m_Acceptor.m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		m_Acceptor.m_addr.sin_port = htons(iListenPort);

		if (bind(m_Acceptor.m_ListenFd, (sockaddr*)&m_Acceptor.m_addr, sizeof(m_Acceptor.m_addr)) != 0)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			m_Acceptor.Clear();
			return false;
		}


		if (CreateIoCompletionPort((HANDLE)m_Acceptor.m_ListenFd, m_IocpHandle, CompletionKey::CK_ACCPET, 0) != m_IocpHandle)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			m_Acceptor.Clear();
			return false;
		}

		if (listen(m_Acceptor.m_ListenFd, 128) == SOCKET_ERROR)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			m_Acceptor.Clear();
			return false;
		}

		// 取得AcceptEx和GetAcceptExSockaddrs函数指针
		DWORD dwbytes = 0;
		GUID kGuidAcceptEx = WSAID_ACCEPTEX;
		GUID kGuidSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

		if (m_lpfnAcceptEx == nullptr)
		{
			DWORD BytesReturned = 0;
			//获取AcceptEx函数指针
			GUID GuidAcceptEx = WSAID_ACCEPTEX;
			if (WSAIoctl(m_Acceptor.m_ListenFd, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &BytesReturned, nullptr, nullptr) == SOCKET_ERROR)
			{
				return false;
			}

			//获取GetAcceptexSockAddrs函数指针
			GUID GuidGetAcceptexSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
			if (WSAIoctl(m_Acceptor.m_ListenFd, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptexSockAddrs, sizeof(GuidGetAcceptexSockAddrs), &m_lpfnGetAcceptSockAddrs, sizeof(m_lpfnGetAcceptSockAddrs), &BytesReturned, nullptr, nullptr) == SOCKET_ERROR)
			{
				return false;
			}
		}
		m_Acceptor.m_Overlapped.resize(iListenNum);
		for (int i = 0; i < iListenNum; ++i)
		{
			m_Acceptor.m_Overlapped[i].m_SockFd = C_INVALID_SOCKET;
			m_Acceptor.m_Overlapped[i].m_Acceptor = &m_Acceptor;
			memset(m_Acceptor.m_Overlapped[i].m_Buffer, 0, sizeof(m_Acceptor.m_Overlapped[i].m_Buffer));
			PostAccept(m_Acceptor.m_Overlapped[i]);
		}
		return true;
	}

	bool IocpNetMgr::Connect(const char* ip, UInt16 iPort)
	{
		SOCKET hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (hSocket == INVALID_SOCKET)
		{
			return false;
		}

		// 设置连接服务器
		sockaddr_in kSockAddr;
		memset(&kSockAddr, 0, sizeof(kSockAddr));
		inet_pton(AF_INET, ip, &kSockAddr.sin_addr);
		kSockAddr.sin_family = AF_INET;
		kSockAddr.sin_port = htons(iPort);

		// 修改该socket为非阻塞模式
		ULONG ulValue = 1;
		int iRet = ioctlsocket(hSocket, FIONBIO, &ulValue);
		if (iRet == SOCKET_ERROR)
		{
			closesocket(hSocket);
			return false;
		}

		// 连接服务器
		if (connect(hSocket, (SOCKADDR*)&kSockAddr, sizeof(kSockAddr)) == -1)
		{
			// 设置连接超时时间
			fd_set kSet;
			FD_ZERO(&kSet);
			FD_SET(hSocket, &kSet);
			timeval kTimeOut;
			kTimeOut.tv_sec = 0;
			kTimeOut.tv_usec = 500;
			iRet = select((int)hSocket + 1, NULL, &kSet, NULL, &kTimeOut);
			if (iRet <= 0)
			{
				closesocket(hSocket);
				return false;
			}
			else
			{
				int iError = -1;
				int iLen = sizeof(int);
				getsockopt(hSocket, SOL_SOCKET, SO_ERROR, (char*)&iError, &iLen);
				if (iError != 0)
				{
					closesocket(hSocket);
					return false;
				}
			}
		}

		// 还原为阻塞模式
		ulValue = 0;
		iRet = ioctlsocket(hSocket, FIONBIO, &ulValue);
		if (iRet == SOCKET_ERROR)
		{
			closesocket(hSocket);
			return false;
		}

		return CreateSockConn(hSocket, (sockaddr*)&kSockAddr, ConnType::CT_ACTIVE);

		return true;
	}

	void IocpNetMgr::WorkerProc()
	{
		while (true)
		{
			DWORD num = 0;
			ULONG_PTR comKey = 0;
			LPOVERLAPPED overLapped = nullptr;
			bool bRet = GetQueuedCompletionStatus(m_IocpHandle, &num, &comKey, &overLapped, INFINITE);

			if (!bRet)
			{
				LOGGER_ERROR("error = {0}", sock_error());
				if (overLapped != nullptr)
				{
					delete overLapped;
				}
				continue;
			}
			CompletionKey cKey = (CompletionKey)comKey;

			if ((CompletionKey)comKey == CompletionKey::CK_THREAD_CLOSE)
			{
				return;
			}
			else if ((CompletionKey)comKey == CompletionKey::CK_ACCPET)
			{
				Acceptor* pAcceptor = ((AcceptOverlapped*)overLapped)->m_Acceptor;
				OnAccept((AcceptOverlapped*)overLapped);
				PostAccept(*(AcceptOverlapped*)overLapped);
			}
			else if ((CompletionKey)comKey == CompletionKey::CK_CONNECT)
			{
				auto* pContext = ((ConnectIoContext*)overLapped);
				auto pConnSock = ((ConnectIoContext*)overLapped)->m_ConnSockPtr;

				if (!pConnSock)
				{
					LOGGER_ERROR("pConnSock is null");
					delete pContext;
					continue;
				}
				//TODO

				switch (((Overlapped*)overLapped)->tagReqHandle)
				{
				case TRH_SEND:
				{
					HandleSend(pContext, num);

				}
				break;
				case TRH_RECV:
				{
					HandleRecv(pContext, num);
				}
				break;
				case TRQ_NONE:
				default:
				{
					LOGGER_ERROR("unknown error!");
					delete pContext;
				}
				break;
				}
			}

		}

		LOGGER_ERROR("work thread close!");
	}

	bool IocpNetMgr::PostAccept(AcceptOverlapped& rOverlapped)
	{
		memset(&rOverlapped, 0, sizeof(OVERLAPPED));
		rOverlapped.m_SockFd = (int)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (rOverlapped.m_SockFd == C_INVALID_SOCKET)
		{

			return false;
		}
		DWORD dwBytes = 0;
		if (!m_lpfnAcceptEx(rOverlapped.m_Acceptor->m_ListenFd, rOverlapped.m_SockFd, rOverlapped.m_Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &rOverlapped))
		{
			if (WSA_IO_PENDING != sock_error())
			{
				LOGGER_ERROR("error = {0}", sock_error());
				return false;
			}
		}
		return true;
	}

	void IocpNetMgr::OnAccept(AcceptOverlapped* pkOverlapped)
	{
		if (pkOverlapped == nullptr)
		{
			return;
		}

		if (setsockopt(pkOverlapped->m_SockFd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(pkOverlapped->m_Acceptor->m_ListenFd), sizeof(pkOverlapped->m_Acceptor->m_ListenFd)) != 0)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			closesocket(pkOverlapped->m_SockFd);
			return;
		}
		int flag = 1;
		if (setsockopt(pkOverlapped->m_SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) != 0)
		{
			LOGGER_ERROR("error = {0}", sock_error());
			closesocket(pkOverlapped->m_SockFd);
			return;
		}


		sockaddr* paddr1 = nullptr;
		sockaddr* paddr2 = nullptr;
		int tmp1 = 0;
		int tmp2 = 0;
		m_lpfnGetAcceptSockAddrs(pkOverlapped->m_Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &paddr1, &tmp1, &paddr2, &tmp2);

		CreateSockConn(pkOverlapped->m_SockFd, paddr2, ConnType::CT_PASSIVE);
	}

	void IocpNetMgr::PushLogicData(PushData* data)
	{
		m_PushMutex.lock();
		m_PushDatas.push_back(data);
		m_PushMutex.unlock();
	}

	void IocpNetMgr::PushDisconnectConn(std::shared_ptr<ConnSock>& rConnSock)
	{
		if (rConnSock->IsClose())
		{
			return;
		}
		rConnSock->Close();
		PushData* data = CreatePushData();
		data->eType = PDT_CONN_CLOSE;
		data->iAllocID = rConnSock->m_AllocID;
		PushLogicData(data);
	}


	void IocpNetMgr::DisconnectConn(UInt32 allocID)
	{
		bool bFind = false;
		std::shared_ptr<ConnSock> connSock;
		m_ConnMutex.lock();
		auto itr = m_ConnSocks.find(allocID);
		if (itr != m_ConnSocks.end())
		{
			bFind = true;
			connSock = itr->second;
			m_ConnSocks.erase(itr);
		}
		m_ConnMutex.unlock();

		if (bFind && connSock != nullptr)
		{
			PushDisconnectConn(connSock);
		}

	}

	void IocpNetMgr::HandleSend(ConnectIoContext* pContext, UInt32 dwSize)
	{
		if (pContext == nullptr)
		{
			return;
		}

		auto pConnSock = pContext->m_ConnSockPtr;

		//这里代表已经关闭了,直接释放内存
		if (!pConnSock || pConnSock->IsClose())
		{
			delete pContext;
			return;
		}

		bool bDel = false;
		std::lock_guard<std::mutex> pLock(pConnSock->m_SendMutex);

		if (dwSize > 0)
		{
			pConnSock->m_SendBuffer.Retrieve(dwSize);
			if (pConnSock->m_SendBuffer.ReadableBytes() == 0 && pConnSock->m_ExtraSendBuffer.ReadableBytes() > 0)
			{
				pConnSock->m_SendBuffer.swap(pConnSock->m_ExtraSendBuffer);
			}
			else if (pConnSock->m_SendBuffer.ReadableBytes() > 0 && pConnSock->m_ExtraSendBuffer.ReadableBytes() > 0)
			{
				pConnSock->m_SendBuffer.Append(pConnSock->m_ExtraSendBuffer.BeginRead(), pConnSock->m_ExtraSendBuffer.ReadableBytes());
			}

			if (pConnSock->m_SendBuffer.ReadableBytes() > 0)
			{
				pContext->buf.buf = pConnSock->m_SendBuffer.BeginRead();
				pContext->buf.len = pConnSock->m_SendBuffer.ReadableBytes();
				DWORD sendBytes = 0;
				memset(pContext, 0, sizeof(OVERLAPPED));

				if (WSASend(pConnSock->GetFd(), &(pContext->buf), 1, &sendBytes, 0, pContext, nullptr) == C_SOCK_ERROR)
				{
					if (sock_error() != ERROR_IO_PENDING)
					{
						LOGGER_ERROR("WSASend() failed. Error:{0}",sock_error());
						bDel = true;
					}
				}
			}
		}
		else
		{
			bDel = true;
		}
		if (bDel)
		{
			delete pContext;
			DisconnectConn(pConnSock->m_AllocID);
		}
	}

	void IocpNetMgr::HandleRecv(ConnectIoContext* pContext, UInt32 dwSize)
	{
		bool bDel = false;

		if (pContext == nullptr)
		{
			return;
		}
		auto pConnSock = pContext->m_ConnSockPtr;

		//这里代表已经关闭了,直接释放内存
		if (!pConnSock || pConnSock->IsClose())
		{
			delete pContext;
			return;
		}

		if (dwSize > 0)
		{
			pConnSock->m_RecvBuffer.HasWritten(dwSize);

			PushData* data = CreatePushData();
			UInt32 iReadableByte = pConnSock->m_RecvBuffer.ReadableBytes();

			data->kNetBuff.Append(pConnSock->m_RecvBuffer.BeginRead(), iReadableByte);
			data->eType = PDT_RECV_DATA;
			data->iAllocID = pConnSock->GetAllocID();
			PushLogicData(data);

			pConnSock->m_RecvBuffer.Retrieve(iReadableByte);

			//TODO
			pContext->buf.buf = pConnSock->m_RecvBuffer.BeginWrite();
			pContext->buf.len = pConnSock->m_RecvBuffer.WritableBytes();

			DWORD dwIOSize = 0, dwFlag = 0;
			memset(pContext, 0, sizeof(OVERLAPPED));

			if (WSARecv(pConnSock->GetFd(), &(pContext->buf), 1, &dwIOSize, &dwFlag, pContext, nullptr) == SOCKET_ERROR)
			{
				int iRecvError = sock_error();
				if (iRecvError != ERROR_IO_PENDING)
				{
					LOGGER_ERROR("WSARecv() failed. Error:{0}", sock_error());
					bDel = true;
				}
			}

		}
		else
		{
			bDel = true;
		}
		if (bDel)
		{
			delete pContext;
			DisconnectConn(pConnSock->GetAllocID());
		}
	}
	//外部逻辑线程调用
	bool IocpNetMgr::SendData(std::shared_ptr<ConnSock>& pConnSock, const char* data, UInt16 len)
	{
		if (pConnSock == nullptr || pConnSock->IsClose())
		{
			return false;
		}
		bool bCreate = false;
		if (pConnSock->m_SendIoContext == nullptr)
		{
			pConnSock->m_SendIoContext = new ConnectIoContext;

			pConnSock->m_SendIoContext->tagReqHandle = TRH_SEND;
			pConnSock->m_SendIoContext->m_ConnSockPtr = pConnSock;
			pConnSock->m_SendBuffer.RetrieveAll();
			pConnSock->m_ExtraSendBuffer.RetrieveAll();
			bCreate = true;
		}
		auto pConntext = pConnSock->m_SendIoContext;
		std::lock_guard<std::mutex> lck(pConnSock->m_SendMutex);

		if (pConnSock->m_SendBuffer.ReadableBytes() > 0)
		{
			pConnSock->m_ExtraSendBuffer.Append(data, len);
		}
		else
		{
			pConnSock->m_SendBuffer.Append(data, len);
			pConntext->buf.buf = pConnSock->m_SendBuffer.BeginRead();
			pConntext->buf.len = pConnSock->m_SendBuffer.ReadableBytes();
			DWORD dwIOSize = 0;
			memset(pConnSock->m_SendIoContext, 0, sizeof(OVERLAPPED));

			if (WSASend(pConnSock->GetFd(), &(pConnSock->m_SendIoContext->buf), 1, &dwIOSize, 0, pConntext, nullptr) == SOCKET_ERROR)
			{
				if (sock_error() != ERROR_IO_PENDING)
				{
					LOGGER_ERROR("WSASend() failed. Error:{0}", sock_error());
					if (bCreate)
					{
						delete pConnSock->m_SendIoContext;
					}
					pConnSock->m_SendIoContext = nullptr;

					DisconnectConn(pConnSock->GetAllocID());
					return false;
				}
			}
		}
		return true;
	}

	PushData* IocpNetMgr::CreatePushData()
	{
		PushData* retData = nullptr;
		if (!m_FreePushDatas.empty())
		{
			std::lock_guard<std::mutex> lck(m_FreePushDataMutex);
			if (!m_FreePushDatas.empty())
			{
				auto* retData = m_FreePushDatas.back();
				m_FreePushDatas.pop_back();
			}
		}

		if (retData != nullptr)
		{
			retData->Init();
		}
		return new PushData();
	}

	void  IocpNetMgr::RecyclePushData(PushData* pData)
	{
		//闲置的pushdata过多就直接delete了。不锁是为了减少竞争。因为这里就算size不准也不影响什么。
		if (m_FreePushDatas.size() > 100)
		{
			delete pData;
			return;
		}

		std::lock_guard<std::mutex> lck(m_FreePushDataMutex);
		m_FreePushDatas.push_back(pData);
		return;
	}

	bool IocpNetMgr::CreateSockConn(SocketFd sFd, sockaddr* pAddr, ConnType eConnType)
	{
		std::shared_ptr<ConnSock> conn = std::make_shared<ConnSock>();

		conn->Init(sFd, NetConnectionStatus::NCS_NORMAL, pAddr, eConnType);
		if (m_IocpHandle != CreateIoCompletionPort((HANDLE)conn->GetFd(), m_IocpHandle, CompletionKey::CK_CONNECT, 0))
		{
			LOGGER_ERROR("error = {0}", sock_error());
			return false;
		}
		auto recvIoContext = new ConnectIoContext;
		memset(recvIoContext, 0, sizeof(OVERLAPPED));

		recvIoContext->tagReqHandle = TRH_RECV;
		recvIoContext->buf.buf = conn->m_RecvBuffer.BeginWrite();
		recvIoContext->buf.len = conn->m_RecvBuffer.WritableBytes();
		recvIoContext->m_ConnSockPtr = conn;



		int allocID = m_allocID.fetch_add(1);
		conn->SetAllocID(allocID);

		m_ConnMutex.lock();
		m_ConnSocks[allocID] = conn;
		m_ConnMutex.unlock();

		DWORD dwIOSize = 0, dwFlag = 0;

		if (WSARecv(conn->GetFd(), &recvIoContext->buf, 1, &dwIOSize, &dwFlag, recvIoContext, nullptr) == SOCKET_ERROR)
		{
			int iRecvError = sock_error();
			if (iRecvError != ERROR_IO_PENDING)
			{
				LOGGER_ERROR("error = {0}", sock_error());
				delete recvIoContext;
				DisconnectConn(conn->GetAllocID());
				return false;
			}
		}

		PushData* data = CreatePushData();
		data->eType = PDT_NEW_CONNECT;
		data->pConnSock = conn;
		PushLogicData(data);
		return true;
	}
}

#endif // WINDOWS_FLAG