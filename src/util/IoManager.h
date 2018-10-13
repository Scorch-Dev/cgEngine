#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <map>
#include <stdio.h>
#include <thread>

#include "MemoryPool.h"
#include "SentinelConfig.h"
#include "ThreadPool.h"

namespace sentinel
{

typedef void* FileHandle; //points to an AsyncFile
extern const FileHandle NULL_FILE_HANDLE;

enum FileOpStatus : int
{
	IO_BUFFER_OVERFLOW = -2,
    IO_FAILED = -1,
    IO_SUCCESS = 1,
};

class IoManager
{
public:
	IoManager();

	//spin up/down async threads
    void startUp();
    void shutDown();

    //file open close
    FileHandle openFile(const std::string& file_path, bool clear=false);
    void closeFile(FileHandle handle);

    //synchronous read/writes ( simple :) )
    std::size_t readFile(FileHandle handle, char* buffer, 
            std::size_t buffer_size, FileOpStatus& status);
    std::size_t writeFile(FileHandle handle, const char* buffer, 
            std::size_t buffer_size, FileOpStatus& status);
    std::size_t writeFileUnbuffered(FileHandle handle, const char* buffer, 
            std::size_t buffer_size, FileOpStatus& status);

    //async operations
    bool cancelAsyncIo(AsyncJobHandle handle); //can still fail
    void waitAsyncIo(AsyncJobHandle handle);

    AsyncJobHandle asyncRead(FileHandle handle, char* buffer, std::size_t buffer_size, 
            const std::function<void (FileOpStatus, std::size_t)>& cb, bool immedate=false);
    AsyncJobHandle asyncWrite(FileHandle handle, const char* buffer, std::size_t buffer_size, 
            const std::function<void (FileOpStatus, std::size_t)>& cb, bool immediate=false);
    AsyncJobHandle asyncWriteUnbuffered(FileHandle handle, 
            const char* buffer, std::size_t buffer_size, 
            const std::function<void (FileOpStatus, std::size_t)>& cb, bool immediate=false);

private:

    typedef std::mutex MutexT;
    typedef std::unique_lock<MutexT> LockT;

    friend FileHandle;

    //a file struct & supports multiple access
    struct AsyncFile
    {
        FILE* file;
        MutexT mut;
        bool deleted;

        AsyncFile(FILE* f) : file(f), deleted(false) {}
    };

	ThreadPool* m_thread_pool; //* so we can realloc
    MemoryPool m_async_file_pool;

};

} //namespace s_util

#endif
