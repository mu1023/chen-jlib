#ifndef _APPENDERBASE_H_
#define _APPENDERBASE_H_

#include <CommonDefine.h>
#include <TimeTool.h>
#include "fmt/format.h"
#include <mutex>
namespace cj
{
	using string_view_t = fmt::basic_string_view<char>;
	using timeBuffer = fmt::basic_memory_buffer<char, 50>;
	using logBuf = fmt::basic_memory_buffer<char, 300>;
	//日志头部信息
	struct SourceLoc
	{
		SourceLoc() = default;
		SourceLoc(const char *filename_in, Int32 line_in, const char *funcname_in,const char *loglevel_in)
			: filename{ filename_in }
			, line{ line_in }
			, funcname{ funcname_in }
			, logLevel{ loglevel_in }
		{}

		bool empty() const
		{
			return line == 0;
		}
		const char				*filename{ nullptr };
		Int32					line{ 0 };
		const char				*funcname{ nullptr };
		const char				*logLevel{ nullptr };
	};
	//一条日志
	struct LogMsg
	{

		LogMsg(string_view_t str):loc(SourceLoc()),info(str),time(Timestamp::Now()){}
		LogMsg(SourceLoc Loc,string_view_t str):loc(Loc),info(str),time(Timestamp::Now()){}
		
		SourceLoc				loc;
		string_view_t		    info;
		UInt64					time;

	};
	inline const char * BaseName(const char * filename)
	{

		const char *rv = std::strrchr(filename, FOLDER_SEP);
		return rv != nullptr ? rv + 1 : filename;

	}

	

	//日志输出的目标
	class AppenderBase
	{
	public:
		virtual void Append(const LogMsg& msg)=0;
	
	private:
	};

	class NullMutex
	{
	public:
		NullMutex(){};
		~NullMutex(){};

		void lock(){};
		void unlock(){};
	};

	template<typename Mutex>
	class Appender:public AppenderBase
	{
	public:
		void Append(const LogMsg& msg)override;
	protected:
		virtual void Put(const LogMsg& msg) = 0;
	private:
		Mutex m_Mutex;
	};
	template<typename Mutex>
	inline void Appender<Mutex>::Append(const LogMsg& msg)
	{
		m_Mutex.lock();
		Put(msg);
		m_Mutex.unlock();
	}
};
#endif