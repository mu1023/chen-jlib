#pragma once
#include <NetDef.h>

class NetCallBack
{
public:
	virtual bool OnNetConneted() = 0;
	virtual bool OnNetAcceot() = 0;
	virtual bool OnNetRecv() = 0;
	virtual bool OnNetDisConnented() = 0;
};

class NetMgr
{
public:
	virtual bool Initialize(NetCallBack* call, Int32 iMaxThread, const char* ip, UInt16 port) = 0;
	virtual void Upadete() = 0;
	virtual void Finialize() = 0;

	NetCallBack* GetCall() { return m_Call; }
protected:
	NetCallBack		*m_Call;
};