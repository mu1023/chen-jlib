#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <Noncopyable.h>
#include <Formatter.h>
#include <AppenderBase.h>
#include <FileAppender.h>
#include <ConsoleAppender.h>
#include <UdpAppender.h>
#include <Singleton.h>
#include <fmt/format.h>
#include <cstring>
#include <string>
#include <sstream>
#include <functional>
#include <ctime>
#include <stdarg.h>
#include <vector>

namespace cj
{
	enum class LogLevel
	{
		DEBUG = 1,
		INFO = 2,
		WARRING = 3,
		ERRORR = 4,
		LOG_MAX,
	};


	//日志，非异步
	class Logger :public Singleton<Logger>
	{
	public:
		typedef std::shared_ptr<AppenderBase>	    AppenderBase_Ptr;
		typedef std::initializer_list<AppenderBase_Ptr> Appender_Init_List;

		explicit Logger() :m_Appenders(), m_LogLevel(LogLevel::DEBUG)
		{
		};

		template<typename It>
		Logger(It begin, It end) :m_Appenders(begin, end), m_LogLevel(LogLevel::DEBUG)
		{
		}

		Logger(Appender_Init_List lt) :Logger(lt.begin(), lt.end())
		{
		};

		Logger(AppenderBase_Ptr ptr) :Logger({ ptr })
		{
		};

		void InsertAppenderPtr(AppenderBase_Ptr ptr){m_Appenders.push_back(ptr); }
		template<typename ...Args>
		void Log(LogLevel lvl, SourceLoc loc, const char* formatStr, Args... args)
		{
			if (lvl < m_LogLevel)
			{
				return;
			}
			logBuf buf;
			try
			{
				fmt::format_to(buf, formatStr, args...);

			}
			catch (...)
			{
				return;
			}
			LogMsg lg(loc, string_view_t(buf.data(), buf.size()));

			for (auto it : m_Appenders)
			{
				it->Append(lg);
			}
		}
		void Log(LogMsg* lg);


		inline void SetLogLevel(LogLevel level)	{	m_LogLevel = level; 	}

		inline LogLevel GetLogLevel(){	return m_LogLevel;	}

	private:
		//	fmt::memory_buffer			m_Buffer;

		std::vector<AppenderBase_Ptr> m_Appenders;
		LogLevel m_LogLevel;

	};
	
	 const char* GetLogLevelString(LogLevel level);

	//static Logger  m_GlobalLogger;



}

#define LOGGER_DEBUG( formatStr , ...	)  cj::Logger::Instance()->Log( cj::LogLevel::DEBUG,cj::SourceLoc(__FILE__,__LINE__,__func__,cj::GetLogLevelString(cj::LogLevel::DEBUG) ),formatStr,__VA_ARGS__);
#define LOGGER_INFO( formatStr , ...	)  cj::Logger::Instance()->Log( cj::LogLevel::INFO,cj::SourceLoc(__FILE__,__LINE__,__func__,cj::GetLogLevelString(cj::LogLevel::INFO)),formatStr,__VA_ARGS__);
#define LOGGER_WARRING( formatStr , ...	)  cj::Logger::Instance()->Log( cj::LogLevel::WARRING,cj::SourceLoc(__FILE__,__LINE__,__func__,cj::GetLogLevelString(cj::LogLevel::WARRING)),formatStr,__VA_ARGS__);
#define LOGGER_ERROR( formatStr , ...	)  cj::Logger::Instance()->Log( cj::LogLevel::ERRORR,cj::SourceLoc(__FILE__,__LINE__,__func__,cj::GetLogLevelString(cj::LogLevel::ERRORR)),formatStr,__VA_ARGS__);
#endif // _LOGGING_H__
