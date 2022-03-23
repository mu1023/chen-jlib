#include <Logging.h>
namespace cj
{
	const char* GetLogLevelString(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::INFO:return "INFO";
			break;
		case LogLevel::DEBUG:return "DEBUG";
			break;
		case LogLevel::ERRORR:return "ERRORR";
			break;
		case LogLevel::WARRING:return "WARRING";
			break;
		default:
			break;
		}
		return "";
	}

	

	void Logger::Log(LogMsg * lg)
	{
		for (auto it : m_Appenders)
		{
			it->Append(*lg);
		}
	}

}