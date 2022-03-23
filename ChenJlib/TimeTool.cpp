#include<TimeTool.h>
#ifdef WINDOWS_FLAG
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#endif
namespace cj {
	Timestamp::Timestamp(UInt64 now):m_MicroSeconds(now)
	{
	}
	UInt64 Timestamp::Now()
	{

#ifdef WINDOWS_FLAG
		// 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
		FILETIME fileTime;
		ULARGE_INTEGER li;
		const static UInt64 TIME_DIFF = 116444736000000000;
		;
		::GetSystemTimeAsFileTime(&fileTime);
		li.LowPart = fileTime.dwLowDateTime;
		li.HighPart = fileTime.dwHighDateTime;
		return ((UInt64)li.QuadPart - TIME_DIFF) / 10000;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int64_t seconds = tv.tv_sec;
		return Timestampstamp(seconds * MILLISECS_OF_SEC + tv.tv_usec * MICROSECS_OF_MILLISEC);
#endif // _WIN32
	}
	Date::Date(Timestamp ts)
	{
		time_t sec = ts.GetSeconds();
		Localtime(&m_Tm,&sec);
		m_Msec = ts.GetMicroSeconds() % Timestamp::MICROSECS_OF_SEC;
	}
	Date::Date(UInt32 year, UInt32 month, UInt32 day, UInt32 hour, UInt32 minute, UInt32 second, UInt32 millicSecond)
	{

		m_Tm.tm_year = year - 1900;
		m_Tm.tm_mon = month - 1;
		m_Tm.tm_mday = day;
		m_Tm.tm_hour = hour;
		m_Tm.tm_min = minute;
		m_Tm.tm_sec = second;
		m_Msec = millicSecond;	
	}
	Timestamp Date::ToTime()
	{
		return (UInt64)mktime((struct tm*)&m_Tm) * 1000 + m_Msec;
	}
}