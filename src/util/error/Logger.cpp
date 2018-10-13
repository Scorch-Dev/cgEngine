#include "Logger.h"

namespace sentinel
{

Logger::Logger() :
	m_io_manager(nullptr),
	m_log_file_handle(NULL_FILE_HANDLE)
{
	//do nothing!
}

Logger::~Logger()
{
	/*do nothing!*/
}

/**
 * opens a log file (append mode) and writes a start of run header
 * 
 * @param log_path: the path to the log file from the root
 * @param io_manager: the io_manager ref to use for file writes
 */
void Logger::startUp(const std::string& log_path, IoManager& io_manager)
{
	//set up members and open file
	S_ASSERT(m_io_manager == nullptr, "logger already init'd!");
	S_ASSERT(m_log_file_handle == NULL_FILE_HANDLE, "logger already init'd!");

	m_log_path_sid = INTERN_STR(log_path);
	m_io_manager = &io_manager;

	m_log_file_handle = io_manager.openFile(log_path);
	S_ASSERT(m_log_file_handle != NULL_FILE_HANDLE, "logger already init'd!");

	//dynamic alloc to avoid out of scope issues
	std::string time( std::move(getTimeStr()) );
	std::string* log_file_header = new std::string(
		std::string("-------")
		+ "Log Start: "
		+ time
		+ "------\n");

	AsyncJobHandle header_job = io_manager.asyncWriteUnbuffered(
		m_log_file_handle,log_file_header->c_str(),
		log_file_header->size(),
		[this, log_file_header](FileOpStatus status, std::size_t bytes)
		{
			S_ASSERT(status != IO_FAILED, "Logger failed to start");
			delete(log_file_header);
			m_outstanding_jobs_q.pop();
		});

	//every job emplaces and pops itself
	m_outstanding_jobs_q.push(header_job);
}

/**
 * simply closes the log file and resets internal vars
 */
void Logger::shutDown()
{
	S_ASSERT(m_io_manager != nullptr, "logger not yet init'd!");
	S_ASSERT(m_log_file_handle != NULL_FILE_HANDLE, "logger not yet init'd!");

	//wait for all jobs to return
	while (!m_outstanding_jobs_q.empty())
	{
		m_io_manager->waitAsyncIo(m_outstanding_jobs_q.front());
	}
	
	m_io_manager->closeFile(m_log_file_handle);

	m_log_file_handle = NULL_FILE_HANDLE;
	m_io_manager = nullptr;
}

/**
 * a simple synchronous write to the log file.
 * Timestamp is prepended to the message automatically.
 *
 * @param msg: the msg to write
 * @param level: the log level to use (default LVL_INFO)
 */
void Logger::log(const std::string& msg, LogLevel level)
{
	//first log the time
	FileOpStatus status;
	std::string write_str = std::move(getInfoStr(level))
		+ std::move(getTimeStr()) + msg + '\n';

	m_io_manager->writeFileUnbuffered(
		m_log_file_handle, write_str.c_str(),
		write_str.size(), status);
}

/**
 * log a string asyncrhonously to the file.
 * Timestamp is prepended to the message automatically.
 *
 * @param msg: the message to write
 * @param immediate: true to queue the async job at front
 *        of job queue (default is false)
 * @param level: the log level to use (default LVL_INFO)
 */
void Logger::logAsync(const std::string& msg, bool immediate,
	LogLevel level)
{
	//dynamic to avoid out of scope issues
	FileOpStatus status;
	std::string* write_str = new std::string(
		std::move(getInfoStr(level))
		+ std::move(getTimeStr())+ msg + '\n');

	AsyncJobHandle write_job = m_io_manager->asyncWriteUnbuffered(
		m_log_file_handle, write_str->c_str(),
		write_str->size(),
		[this, write_str](FileOpStatus status, std::size_t bytes)
	{
		S_ASSERT(status != IO_FAILED, "Logger failed to write"); 
		//TODO: the program doesn't have to fail on logger fail...
		delete(write_str);
		m_outstanding_jobs_q.pop();
	});

	m_outstanding_jobs_q.push(write_job); //TODO: this isn't the safest idea on earth
}

} //namespace sentinel