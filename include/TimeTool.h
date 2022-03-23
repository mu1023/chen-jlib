#ifndef __TIME_TOOL_H_
#define __TIME_TOOL_H_
#include<CommonDefine.h>
#include <ctime>
namespace cj {
	class Timestamp {
	public:
		//每秒 1000 ms
		static const UInt64 MILLISECS_OF_SEC = 1000; 
		//每秒1000000 us
		static const UInt64 MICROSECS_OF_SEC = 1000000; 
		//每毫秒微秒数
		static const UInt64 MICROSECS_OF_MILLISEC = 1000;
		//每分秒数
		const static UInt32 SECS_OF_MIN = 60;
		//每小时秒数
		const static UInt32 SECS_OF_HOUR = (60 * SECS_OF_MIN);
		//每日秒数
		const static UInt32 SECS_OF_DAY = (24 * SECS_OF_HOUR);
		//每日毫秒数
		const static UInt32 MILLISECS_OF_DAY = (SECS_OF_DAY * MILLISECS_OF_SEC);
	public:
		Timestamp(UInt64 now = Now());

		UInt64 GetMicroSeconds() { return m_MicroSeconds; }

		UInt64 SetMicroSeconds(UInt64 microSeconds) { m_MicroSeconds = microSeconds; }

		UInt64 GetSeconds() { return m_MicroSeconds / MILLISECS_OF_SEC; }

		bool operator<(const Timestamp& rs)const {
			return m_MicroSeconds < rs.m_MicroSeconds;
		}
		bool operator>=(const Timestamp& rs)const {
			return !this->operator<(rs);
		}
		bool operator>(const Timestamp& rs)const {
			return m_MicroSeconds > rs.m_MicroSeconds;
		}
		bool operator<=(const Timestamp& rs)const {
			return !this->operator>(rs);
		}
		bool operator=(const Timestamp& rs)const {
			return  m_MicroSeconds == rs.m_MicroSeconds;
		}
		bool operator!=(const Timestamp& rs)const {
			return !this->operator=(rs);
		}
		static UInt64 Now();
	private:
		UInt64 m_MicroSeconds;
	};

	class Date {
	public:
		Date(Timestamp ts = Timestamp());
		Date(UInt32 year, UInt32 month, UInt32 day, UInt32 hour, UInt32 minute, UInt32 second, UInt32 millicSecond);
		UInt32 GetYear() { return m_Tm.tm_year + 1900; }
		void SetYear(UInt32 year) { m_Tm.tm_year = year - 1900; }

		UInt32 GetMonth() { return  m_Tm.tm_mon + 1; }
		void SetMonth(UInt32 month) { m_Tm.tm_mon = month - 1; }

		UInt32 GetDay() { return m_Tm.tm_mday; }
		void SetDay(UInt32 day) { m_Tm.tm_mday = day; }

		UInt32 GetHour() { return m_Tm.tm_hour; }
		void SetHour(UInt32 hour) { m_Tm.tm_hour = hour; }

		UInt32 GetMinute() { return m_Tm.tm_min; }
		void SetMinute(UInt32 min) { m_Tm.tm_min = min; }

		UInt32 GetSecond() { return m_Tm.tm_sec; }
		void SetSecond(UInt32 sec) { m_Tm.tm_sec = sec; }

		UInt32 GetMSecond() { return m_Msec; }
		void SetMSecond(UInt32 msec) { m_Msec = msec; }

		Timestamp ToTime();
	private:
		struct tm m_Tm;
		UInt32	m_Msec;
	};
}
#endif