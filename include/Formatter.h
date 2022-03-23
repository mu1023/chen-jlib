#ifndef _FORMATTER_H_
#define _FORMATTER_H_
#include <AppenderBase.h>
namespace cj
{
	class FormatterBase
	{
	public:
		virtual void format(const LogMsg& msg,logBuf& dest) = 0;
	};

	//ͬһ���ڻ���ʱ��,�̲߳���ȫ��
	class CacheTimestampFormatter :public FormatterBase
	{
	public:
		void format(const LogMsg& msg, logBuf& dest)override;
	private:
		UInt64						m_CacheSec;
		timeBuffer					m_CachedDatetime;
	};

	//������
	class NoCacheTimestampFormatter :public FormatterBase
	{
	public:
		void format(const LogMsg& msg, logBuf& dest)override;
	private:
		
	};

	class DataPointFormatter :public FormatterBase
	{
	public:
		void format(const LogMsg& msg, logBuf& dest)override;
	private:

	};
}
#endif // _FORMATTERBASE_H_
