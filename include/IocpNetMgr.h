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

	IocpNetMgr();
	~IocpNetMgr();

	bool Initialize(NetCallBack* call, Int32 iMaxThread, const char* ip, UInt16 port) override;
	void Upadete() override;
	void Finialize() override;

	void WorkerProc();

	void PostAccept(Acceptor* acceptor);

	void OnAccept(Acceptor* acceptor);

	void PushCloseConn(UInt32 allocID);
private:

	SocketFd	 m_ListenSock;
	HANDLE		 m_IocpHandle;
	std::vector<std::thread*> m_WorkerThread;
	Acceptor		m_Acceptor;

	std::mutex			m_PushMutex;
	std::vector<PushData*> m_PushDatas;

	std::mutex			m_ConnMutex;
	std::map<UInt32,std::shared_ptr<Connector>> m_Connectors;
	std::atomic<int>		m_allocID;

	LPFN_ACCEPTEX				 m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS	 m_lpfnGetAcceptSockAddrs;
};
#endif // WINDOWS_FLAG

#endif


