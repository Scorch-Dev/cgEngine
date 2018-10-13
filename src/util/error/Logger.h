/**
 * A simple logger class
 * (singleton is over-rated, 
 * nothing wrong with multiple loggers).
 *
 * @author: Alan Armero
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <string>
#include <thread>
#include <queue>

#include "IoManager.h"
#include "SentinelAssert.h"
#include "StringId.h"

namespace sentinel
{

enum LogLevel : int
{
	LVL_INFO = 0,
	LVL_DEBUG = 1,
	LVL_WARN = 2,
	LVL_ERROR = 3,
	LVL_FATAL = 4,
};

class Logger
{
public:
	Logger();
	~Logger();

	void startUp(const std::string& log_path, IoManager& io_manager);
	void shutDown();

	void log(const std::string& msg, LogLevel level=LVL_INFO);
	void logAsync(const std::string& msg, bool immediate=false,
		LogLevel level = LVL_INFO);

	inline StrId getLogPathStrId() 
	{ 
		return m_log_path_sid;
	}


private:

	inline std::string getInfoStr(LogLevel level)
	{
		std::string lvl_str;

		switch (level)
		{
		case LVL_INFO:
			lvl_str = "[INFO]";
			break;
		case LVL_DEBUG:
			lvl_str = "[DEBUG]";
			break;
		case LVL_WARN:
			lvl_str = "[WARNING]";
			break;
		case LVL_ERROR:
			lvl_str = "[ERROR]";
			break;
		case LVL_FATAL:
			lvl_str = "[FATAL]";
			break;
		default:
			break;
		}

		return lvl_str;
	}

    /*sorry I know this is not efficient to return string*/
	inline std::string getTimeStr()
	{
		//write the start of the file with some marker
		std::time_t now = std::time(NULL);
		std::tm * ptm = std::localtime(&now);
		char now_buffer[64];
		// Format: Mo, 15.06.2009 20:20:00
		std::strftime(now_buffer, 64, "%a, %d.%m.%Y %H:%M:%S", ptm);
		now_buffer[63] = '\0';

		return ('[' + std::string(now_buffer) + ']');
	}

	StrId m_log_path_sid;
	IoManager* m_io_manager;
	FileHandle m_log_file_handle;

	//keep track of outstanding async to know when to close file
	std::queue<AsyncJobHandle> m_outstanding_jobs_q;
};
}

#endif
