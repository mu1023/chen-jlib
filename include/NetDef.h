#pragma once
#ifndef _NET_DEF_H_

#define _NET_DEF_H_

#include <iostream>
#include <mutex>
#include <WinSock2.h>
#include <Windows.h>

#include <CommonDefine.h>
#include <NetBuffer.h>

#ifdef WINDOWS_FLAG
#include <Winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>
#include <mswsock.h> 

#pragma comment(lib,"Ws2_32.lib")   //Socket编程需用的动态链接库
#pragma comment(lib,"Kernel32.lib") //IOCP需要用到的动态链接库

#else
#ifndef socklen_t
#define socklen_t __socklen_t
#endif
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#endif
namespace cj
{

#ifdef WINDOWS_FLAG
	typedef SOCKET SocketFd;
	typedef int	   socklen_t;
#	define C_INVALID_SOCKET		INVALID_SOCKET
#	define C_SOCK_ERROR			SOCKET_ERROR

#	define C_ECONNABORTED		WSAECONNABORTED
#	define C_EWOULDBLOCK		WSAEWOULDBLOCK
#	define C_ECONNRESET			WSAECONNRESET
#	define C_EINTR				WSAEINTR
#	define C_ENOBUFS			WSAENOBUFS
#	define C_EAGAIN				WSAEWOULDBLOCK
#	define C_ETIMEDOUT			WSAETIMEDOUT

	typedef UInt32 Events;
	const UInt32 NoneEvent = 0;

#define SOCKET_STARTUP WSADATA wsaData;\
	WSAStartup(MAKEWORD(2, 2), &wsaData);

#define SOCKET_CLEANUP WSACleanup();
#else

	typedef int SocketFd;
#	define C_INVALID_SOCKET		 -1
#	define C_SOCK_ERROR			 -1

#	define closesocket close

#	define C_ECONNABORTED		ECONNABORTED
#	define C_EWOULDBLOCK		EWOULDBLOCK
#	define C_ECONNRESET			ECONNRESET
#	define C_EINTR				EINTR
#	define C_ENOBUFS			ENOBUFS
#	define C_EAGAIN				EAGAIN
#	define C_ETIMEDOUT			ETIMEDOUT

#define SOCKET_STARTUP

#define SOCKET_CLEANUP
#endif

	inline int sock_error()
	{
#ifdef   WINDOWS_FLAG
		return WSAGetLastError();
#else
		return errno;
#endif
	}


	enum CompletionKey
	{
		CK_NONE = 0,
		CK_ACCPET = 1,
		CK_CONNECT = 2,
		CK_THREAD_CLOSE = 3,
	};

	enum TagReqHandle
	{
		TRQ_NONE = 0,
		TRH_SEND = 1,
		TRH_RECV = 2,
	};

	class ConnSock;
	enum PushDataType
	{
		PDT_NONE = 0,
		PDT_NEW_CONNECT = 1,
		PDT_RECV_DATA = 2,
		PDT_CONN_CLOSE = 3,
	};
	//推送的消息
	struct PushData
	{
		PushData()
		{
			eType = PDT_NONE;
			pConnSock = nullptr;
			iAllocID = 0;
		}
		void Init()
		{
			eType = PDT_NONE;
			pConnSock = nullptr;
			kNetBuff.RetrieveAll();
			iAllocID = 0;
		}
		PushDataType eType;
		std::shared_ptr<ConnSock>  pConnSock;
		NetBuffer	kNetBuff;
		UInt32 iAllocID;
	};
	/*
	struct IOCPHandler
	{
		virtual void OnMessage(bool bSucc,CompletionKey key, UInt32 size) = 0;
		//virtual void OnError(ULONG_PTR key, UInt32 error) {}
		virtual void Clear() {};
	};

	*/
	struct Overlapped : public OVERLAPPED
	{
		TagReqHandle tagReqHandle;
	};

}

#endif // !_NET_DEF_H_
