#include<Formatter.h>
namespace cj
{
	void CacheTimestampFormatter::format(const LogMsg & msg, logBuf & dest)
	{
		UInt64 secs = msg.time / Timestamp::MILLISECS_OF_SEC;
		if (secs != m_CacheSec)
		{
			Timestamp tm(msg.time);
			Date date(tm);
			m_CachedDatetime.clear();


			fmt::format_to(m_CachedDatetime, "[{}-{}-{} {:02}:{:02}:{:02}] ", date.GetYear(), date.GetMonth(), date.GetDay(), date.GetHour(), date.GetMinute(), date.GetSecond());
			m_CacheSec = secs;
		}
		dest.append(m_CachedDatetime.begin(), m_CachedDatetime.end());
		if (!msg.loc.empty())
		{
			dest.push_back('[');
			fmt::string_view logstr = msg.loc.logLevel;
			dest.append(logstr.begin(), logstr.end());
			dest.push_back(']');
			dest.push_back(' ');

			dest.push_back('[');
			fmt::string_view filestr = BaseName(msg.loc.filename);
			dest.append(filestr.begin(), filestr.end());
			dest.push_back(']');
			dest.push_back(' ');


			dest.push_back('[');
			fmt::string_view funcstr = msg.loc.funcname;
			dest.append(funcstr.begin(), funcstr.end());
			dest.push_back(':');
			fmt::format_int i(msg.loc.line);
			dest.append(i.data(), i.data() + i.size());
			dest.push_back(']');
			dest.push_back(' ');
		}
		dest.append(msg.info.begin(), msg.info.end());

		//dest.push_back('\r');
		dest.push_back('\n');
	}
	void NoCacheTimestampFormatter::format(const LogMsg & msg, logBuf & dest)
	{

		Timestamp tm(msg.time);
		Date date(tm);

		fmt::format_to(dest, "[{}-{}-{} {:02}:{:02}:{:02}] ", date.GetYear(), date.GetMonth(), date.GetDay(), date.GetHour(), date.GetMinute(), date.GetSecond());

		if (!msg.loc.empty())
		{
			dest.push_back('[');
			fmt::string_view logstr = msg.loc.logLevel;
			dest.append(logstr.begin(), logstr.end());
			dest.push_back(']');
			dest.push_back(' ');

			dest.push_back('[');
			fmt::string_view filestr = BaseName(msg.loc.filename);
			dest.append(filestr.begin(), filestr.end());
			dest.push_back(']');
			dest.push_back(' ');


			dest.push_back('[');
			fmt::string_view funcstr = msg.loc.funcname;
			dest.append(funcstr.begin(), funcstr.end());
			dest.push_back(':');
			fmt::format_int i(msg.loc.line);
			dest.append(i.data(), i.data() + i.size());
			dest.push_back(']');
			dest.push_back(' ');
		}
		dest.append(msg.info.begin(), msg.info.end());

		//dest.push_back('\r');
		dest.push_back('\n');
	}

}