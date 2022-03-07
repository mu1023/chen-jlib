#pragma once
#ifndef _CHEN_IOCPNETMGR_H_
#define _CHEN_IOCPNETMGR_H_

#include <NetMgrDef.h>
#ifdef WINDOWS_FLAG
#include <Acceptor.h>
#include <Connector.h>
#include <functional>
#include <thread>
#include <vector>
#include <list>
#include <map>
#include <atomic>


const UInt32 MAX_THREAD_NUM = 100;

class IocpNetMgr:public NetMgr
{
public:
	IocpNetMgr();
	~IocpNetMgr();

	bool Initialize(NetCallBack* call, Int32 iMaxThread) override;
	void Upadete() override;
	void Finialize() override;

	bool Listen(Int32 iListenPort, Int32 iListenNum);

	bool Connect(const char* ip,UInt16 iPort);

	void WorkerProc();

	bool PostAccept(AcceptOverlapped& rOverlapped);

	void OnAccept(AcceptOverlapped* pkOverlapped);

	void PushLogicData(PushData* data);

	void PushDisconnectConn(std::shared_ptr<ConnSock>& rConnSock);

	//关闭一个链接
	void DisconnectConn(UInt32 allocID);

	//处理发送
	void HandleSend(ConnectIoContext* pContext, UInt32 dwSize);

	void HandleRecv(ConnectIoContext* pContext, UInt32 dwSize);

	bool SendData(std::shared_ptr<ConnSock >& pConnSock, const char* data, UInt16 len);

	PushData* CreatePushData();

	void RecyclePushData(PushData* pData);

	bool CreateSockConn(SocketFd sFd, sockaddr* pAddr, ConnType eConnType);
private:

	HANDLE		 m_IocpHandle;
	std::vector<std::thread*> m_WorkerThread;
	Acceptor		m_Acceptor;

	std::mutex			m_PushMutex;
	std::vector<PushData*> m_PushDatas;

	std::mutex			m_ConnMutex;
	std::map<UInt32,std::shared_ptr<ConnSock>> m_ConnSocks;
	std::atomic<int>		m_allocID;

	LPFN_ACCEPTEX				 m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS	 m_lpfnGetAcceptSockAddrs;

	std::mutex					m_FreePushDataMutex;
	std::vector<PushData*>		m_FreePushDatas;
	//NetBuffer			 m_GlobalBuffer;
	//std::mutex			 m_GolbalBufferMutex;
};
#endif // WINDOWS_FLAG

#endif


