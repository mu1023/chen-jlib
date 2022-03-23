#ifndef _CONSOLEAPPENDER_H_
#define _CONSOLEAPPENDER_H_
#include <AppenderBase.h>
#include <CommonDefine.h>
#include <Formatter.h>
#include <fmt/format.h>
#include <iostream>
#include <tuple>
#include <functional>
#include <TimeTool.h>
#include <vector>
#include <string>
#include <mutex>
#include <assert.h> 
namespace cj
{
	template<typename Mutex, typename Formatter = CacheTimestampFormatter>
	class ConsoleAppender :public Appender<Mutex>
	{
	public:
		ConsoleAppender();
	protected:
		void Put(const LogMsg& msg)override;
		
	private:
		UInt64						m_CacheSec;
		timeBuffer					m_CachedDatetime;
		Formatter					m_Formatter;
	};

	template<typename Mutex,typename Formatter>
	inline ConsoleAppender<Mutex, Formatter>::ConsoleAppender():m_CacheSec(0)
	{
	}

	template<typename Mutex,typename Formatter>
	inline void ConsoleAppender<Mutex, Formatter>::Put(const LogMsg & msg)
	{
		logBuf dest;
		m_Formatter.format(msg, dest);

		fwrite(dest.data(), 1, dest.size(), stdout);
	}
}
#endif