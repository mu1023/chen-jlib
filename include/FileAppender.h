#ifndef _FILEAPPENDER_H_
#define _FILEAPPENDER_H_
#include <AppenderBase.h>
#include <CommonDefine.h>
#include <Formatter.h>
#include <TimeTool.h>
#include <fmt/format.h>
#include <iostream>
#include <tuple>
#include <functional>
#include <vector>
#include <string>
#include <mutex>
#include <assert.h> 

namespace cj
{
	template<typename Mutex, typename Formatter = CacheTimestampFormatter>
	class FileAppender :public Appender<Mutex>
	{
	public:

		FileAppender(const std::string baseName, time_t nowTimestamp);
		~FileAppender();
	protected:
		void Put(const LogMsg& msg)override;
	private:
		void RollFile(time_t nowTimestamp);
		static std::tuple<std::string, std::string> SplitFileName(const std::string& fname);

		FILE* m_Fp;
		std::string					 m_BaseName;
		UInt64					     NextRollDate;

		UInt64						m_CacheSec;
		timeBuffer					m_CachedDatetime;
		Formatter					m_Formatter;
	};
	template<typename Mutex, typename Formatter>
	FileAppender<Mutex, Formatter>::FileAppender(const std::string baseName, time_t nowTimestamp) :m_Fp(0), NextRollDate(0), m_CacheSec(0)
	{
		m_BaseName = baseName;

		RollFile(nowTimestamp);
	}
	template<typename Mutex, typename Formatter>
	inline FileAppender<Mutex, Formatter>::~FileAppender()
	{
		::fclose(m_Fp);
	}
	template<typename Mutex, typename Formatter>
	inline void FileAppender<Mutex, Formatter>::Put(const LogMsg& msg)
	{
		if (msg.time > NextRollDate)
		{
			RollFile(msg.time);
		}
		logBuf dest;
		m_Formatter.format(msg, dest);

		fwriteUnlock(dest.data(), 1, dest.size(), m_Fp);
	}
	template<typename Mutex, typename Formatter>
	void FileAppender<Mutex, Formatter>::RollFile(time_t nowTimestamp)
	{
		Timestamp     tt(nowTimestamp);
		Date	date(tt);
		std::string FilePrefix;
		std::string FileSuffix;
		std::tie(FilePrefix, FileSuffix) = SplitFileName(m_BaseName);

		std::string fileName = fmt::format("{}{:04d}-{:02d}-{:02d}{}", FilePrefix, date.GetYear(), date.GetMonth(), date.GetDay(), FileSuffix);

		date.SetHour(0);
		date.SetMinute(0);
		date.SetSecond(0);
		date.SetMSecond(0);
		NextRollDate = date.ToTime().GetMicroSeconds() + Timestamp::MILLISECS_OF_DAY;
		if (m_Fp != NULL)
		{
			::fclose(m_Fp);
		}
		fopen_s(&m_Fp, fileName.c_str(), "a");
		assert(m_Fp != NULL);
	}

	template<typename Mutex, typename Formatter>
	std::tuple<std::string, std::string> FileAppender<Mutex, Formatter>::SplitFileName(const std::string& fname)
	{
		auto ext_index = fname.rfind('.');


		if (ext_index == std::string::npos || ext_index == 0 || ext_index == fname.size() - 1)
		{
			return std::make_tuple(fname, std::string());
		}

		auto folder_index = fname.rfind(FOLDER_SEP);

		if (folder_index != std::string::npos && folder_index >= ext_index - 1)
		{
			return std::make_tuple(fname, std::string());
		}

		return std::make_tuple(fname.substr(0, ext_index), fname.substr(ext_index));
	}

}

#endif // _FILEAPPENDER_H_
