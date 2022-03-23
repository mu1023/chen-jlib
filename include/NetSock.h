#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include <NetDef.h>
#include <NetMgrDef.h>
#include <memory>

namespace cj
{
	class Acceptor;

	struct AcceptOverlapped :public Overlapped
	{
		Acceptor* m_Acceptor;
		SocketFd		m_SockFd;
		char			m_Buffer[256];
	};

	struct Acceptor
	{
		friend class IocpNetMgr;
	public:
		Acceptor();
		~Acceptor();

		bool Init(NetMgr* mgr);

		void Clear();

		NetMgr* m_NetMgr;
		SocketFd m_ListenFd;

		SOCKADDR_IN		m_addr;

		std::vector<AcceptOverlapped>  m_Overlapped;
		//OverlappedWrapper<AcceptHandler> m_Overlapped;
	};

	struct ConnSock;

	enum NetConnectionStatus
	{
		NCS_CLOSE = 0,
		NCS_NORMAL = 1,
	};

	struct ConnectIoContext :public Overlapped
	{
		WSABUF	   buf;
		std::shared_ptr<ConnSock> m_ConnSockPtr;
	};

	enum ConnType
	{
		CT_ACTIVE = 0,	//主动连接
		CT_PASSIVE = 1,//被动链接
	};

	struct ConnSock : public std::enable_shared_from_this<ConnSock>
	{

		friend class IocpNetMgr;
	public:
		const UInt32 WRITE_BUFFER_SIZE = 10000;
		const UInt32 READ_BUFFER_SIZE = 10000;


		ConnSock();
		~ConnSock();


		void Init(SocketFd fd, NetConnectionStatus status, sockaddr* addr, ConnType connType);

		SocketFd GetFd()
		{
			return m_SocketFd;
		};

		UInt32 GetAllocID()
		{
			return m_AllocID;
		}

		//UInt32 Write(const char* msg, UInt32 len);
		void SetAllocID(UInt32 allocID)
		{
			m_AllocID = allocID;
		}

		void Close();

		bool IsClose();

	private:
		UInt32						m_AllocID;
		volatile SocketFd			m_SocketFd;

		NetBuffer					m_RecvBuffer;

		std::mutex				    m_SendMutex;
		NetBuffer					m_SendBuffer;
		NetBuffer					m_ExtraSendBuffer;
		ConnectIoContext* m_SendIoContext;

		sockaddr_in m_Adder;
		std::mutex				    m_StatusMutex;
		volatile NetConnectionStatus			m_Status;

		ConnType					m_ConnType;
	};
}

#endif // !_CONNECTOR_H_
