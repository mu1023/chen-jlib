#ifndef _UDPAPPENDER_H_
#define _UDPAPPENDER_H_
#include<AppenderBase.h>
#include<NetDef.h>
#include<CommonDefine.h>
#include<Formatter.h>
#include<fmt/format.h>
#include<iostream>
#include<tuple>
#include<functional>
#include<vector>
#include<string>
#include<mutex>
#include<assert.h> 

namespace cj
{
	template<typename Mutex,typename Formatter = NoCacheTimestampFormatter >
	class UdpAppender :public Appender<Mutex>
	{
	public:
		UdpAppender(const char* ip, UInt32 port);
		~UdpAppender();
	protected:
		void Put(const LogMsg& msg)override;
	private:

		const char*					m_Ip;
		UInt32						m_Port;
		SocketFd					m_Sock;

		Formatter					m_Formatter;
	};

	



	template<typename Mutex, typename Formatter>
	inline UdpAppender<Mutex, Formatter>::UdpAppender(const char * ip, UInt32 port) :m_Ip(ip), m_Port(port)
	{
		if (ip == NULL )
		{
			return;
		}
		m_Sock = socket(AF_INET, SOCK_DGRAM , 0);

		if (m_Sock == C_INVALID_SOCKET) 
		{
			std::cout << "create socket err : errcode = " << sock_error() << std::endl;

			return;
		}

		sockaddr_in addr;

		memset(&addr, 0, sizeof addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip, &addr.sin_addr.s_addr) < 0) 
		{
			std::cout << "inet pton err : errcode = " << sock_error() << std::endl;
			return;
		}

		::connect(m_Sock, (struct sockaddr*)&addr, sizeof addr);

	}

	template<typename Mutex, typename Formatter>
	inline UdpAppender<Mutex, Formatter>::~UdpAppender()
	{
		::closesocket(m_Sock);
	}

	template<typename Mutex, typename Formatter>
	inline void UdpAppender<Mutex, Formatter>::Put(const LogMsg & msg)
	{
		logBuf dest;
		m_Formatter.format(msg, dest);

		if (::send(m_Sock, dest.data(), (Int32)dest.size(), 0) < 0) {
			std::cout <<m_Sock<< "Put err : errcode = " << sock_error() << std::endl;
		}
	}

}
#endif