#pragma once
#ifndef _NET_MGR_DEF_H_
#define _NET_MGR_DEF_H_

#endif // !_NET_MGR_DEF_H_

#include <NetDef.h>

class NetCallBack
{
public:
	virtual bool OnNetConneted(std::shared_ptr<ConnSock> pNewConnSock) = 0;
	virtual bool OnNetRecv(UInt32 iAllodID, const NetBuffer& rNetBuffer) = 0;
	virtual bool OnNetDisConnected(UInt32 allocID) = 0;
};

class NetMgr
{
public:
	NetMgr():m_Call(nullptr) {};
	virtual ~NetMgr() {};

	virtual bool Initialize(NetCallBack* call, Int32 iMaxThread) = 0;
	virtual void Upadete() = 0;
	virtual void Finialize() = 0;

	NetCallBack* GetCall() { return m_Call; }
protected:
	NetCallBack		*m_Call;
};