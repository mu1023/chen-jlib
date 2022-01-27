#pragma once
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


#ifdef WINDOWS_FLAG

enum CompletionKey
{
	CK_NONE = 0,
	CK_ACCPET = 1,
	CL_RECV
};

enum TagReqHandle
{
	TRQ_NONE = 0,
	TRH_ACCEPT = 1,
};
struct IOCPHandler
{
	virtual void OnMessage(ULONG_PTR key, UInt32 size) = 0;
	//virtual void OnError(ULONG_PTR key, UInt32 error) {}
	virtual void Clear() {};
};


struct Overlapped : public OVERLAPPED
{
	IOCPHandler* handler;
};

template<class T>
struct OverlappedWrapper : T
{
	Overlapped overlap;

	OverlappedWrapper() {
		ZeroMemory(&overlap, sizeof(overlap));
		overlap.handler = this;
	}

	operator OVERLAPPED* () { return &overlap; }
};
#endif // WINDOWS_FLAG

/*

enum NetConnectionType
{
	NCT_ACTIVE,	//主动连接
	NCT_PASSIVE,	//被动连接
};


enum NetConnectionStatus
{
	NCS_CLOSED,			//初始状态，还没连接
	NCS_VERIFY,			//验证阶段
	NCS_NORMAL			//正常通信状态
};
class Connector : public std::enable_shared_from_this<Connector>
{
public:
	const UInt32 WRITE_BUFFER_SIZE = 10000;
	const UInt32 READ_BUFFER_SIZE = 10000;


	Connector(SocketFd fd) :m_Fd(fd) {};
	~Connector() {};

	virtual void HandleRead() = 0;

	virtual void HandleWrite() = 0;

	virtual void HandleError() = 0;

	SocketFd GetFd();

	UInt32 Write(const char* msg, UInt32 len);

	virtual void Close() = 0;

protected:

	SocketFd					m_Fd;

	NetBuffer					m_ReadBuffer;
	NetBuffer					m_WriteBuffer;
	std::mutex				    m_Mutex;
	NetConnectionStatus			m_Status;
};*/