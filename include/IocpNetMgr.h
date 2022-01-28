#pragma once
#ifndef _CHEN_IOCPNETMGR_H_
#define _CHEN_IOCPNETMGR_H_

#include <NetMgrDef.h>
#ifdef WINDOWS_FLAG
#include <Acceptor.h>
#include <functional>
#include <thread>
#include <vector>


const UInt32 MAX_THREAD_NUM = 100;

class IocpNetMgr:public NetMgr
{
	bool Initialize(NetCallBack* call, Int32 iMaxThread, const char* ip, UInt16 port) override;
	void Upadete() override;
	void Finialize() override;

	void WorkerProc();

	void PostAccept();
private:

	SocketFd	 m_ListenSock;
	HANDLE		 m_IocpHandle;
	std::vector<std::thread*> m_WorkerThread;
	OverlappedWrapper<Acceptor>		m_Acceptor;

	LPFN_ACCEPTEX				 m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS	 m_lpfnGetAcceptSockAddrs;
};
#endif // WINDOWS_FLAG

#endif


