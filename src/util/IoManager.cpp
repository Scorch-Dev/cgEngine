#include "IoManager.h"

namespace sentinel
{

const FileHandle NULL_FILE_HANDLE = nullptr;


/*ctor*/
IoManager::IoManager() :
	m_thread_pool(nullptr),
	m_async_file_pool(sizeof(AsyncFile))
{
}

/**
 * Spins up worker threads. Workers each wait for items
 * on the job q and process them as they come. After
 * the job is processed, the job handle is removed
 * from the set of active jobs.
 */
void IoManager::startUp()
{
	//spin up for real
	m_thread_pool = new ThreadPool(4);
}

/**
 * signals threads to return once job q has been emptied.
 * NOTE: jobs can still be q'd up during spindown,
 * so be mindful of this. Will hang until all threads are joined
 */
void IoManager::shutDown()
{
	delete(m_thread_pool);
    
    //I'm not your mother, you can make sure your own damn files are closed!
}

/**
 * Opens a file and returns a file handle to the user.
 * Files are opened in binary mode for versatility.
 *
 * @param file_path: teh file path to open
 * @param clear: true if you want to clear the file on opening (default: false)
 * @returns a handle for the file
 */
FileHandle IoManager::openFile(const std::string& file_path, bool clear)
{
    FILE* f;

    if(clear)
        f = fopen(file_path.c_str(), "w+b");
    else
        f = fopen(file_path.c_str(), "a+b");

    if(!f)
        return NULL_FILE_HANDLE;

    void* mem_ptr = m_async_file_pool.alloc();
    AsyncFile* async_file = new(mem_ptr) AsyncFile(f);

    return reinterpret_cast<FileHandle>(async_file);
}

/**
 * Closes a file. It is the programmers responsibility
 * to close files. Closing a file while there are outstanding
 * asynchronous jobs on the file leads to undefined
 * behavior.
 *
 * @param handle the file handle to close
 */
void IoManager::closeFile(FileHandle handle)
{
    S_ASSERT(handle != NULL_FILE_HANDLE);

    AsyncFile* async_file = reinterpret_cast<AsyncFile*>(handle);
    
	LockT lock(async_file->mut);
    async_file->deleted = true;//avoid race in next line
    fclose(async_file->file);
	lock.unlock();

    async_file->~AsyncFile();
    m_async_file_pool.freeBlock(async_file);
}

/**
 * a simple synchronous file read. Will attempt
 * to read the entire file.
 *
 * @param handle: the FileHandle to read from
 * @param buffer: the char* to put raw bytres into
 * @param buffer_size: the total size of your buffer
 * @param status: a reference to a FileOpStatus for error checking
 * @returns a size_t of bytes read
 */
std::size_t IoManager::readFile(FileHandle handle, char* buffer,
            std::size_t buffer_size, FileOpStatus& status)
{
    //check file still exists
    AsyncFile* async_file = reinterpret_cast<AsyncFile*>(handle);
    LockT lock(async_file->mut);

    if(async_file->deleted)
    {
        status = IO_FAILED;
        return 0;
    }
    FILE* f = async_file->file;


	//try to read all bytes
	fseek(f, 0, SEEK_END);
	long f_size = ftell(f);
    rewind(f);

	std::size_t bytes_read = fread(buffer, sizeof(char), f_size, f);

	if (bytes_read < f_size)
		status = IO_BUFFER_OVERFLOW;
	else if (!ferror(f))
		status = IO_SUCCESS;
	else
		status = IO_FAILED;

    clearerr(f);
    return bytes_read;
}

/**
 * a simple synchronous file write which appends
 * to the file.
 * 
 * @param handle: the FileHandle to read from
 * @param buffer: the const char* to write from
 * @param buffer_size: the size of your buffer
 * @param status: a reference to a FileOpStatus for error checking
 * @returns a size_t of bytes written
 */
std::size_t IoManager::writeFile(FileHandle handle, const char* buffer, 
            std::size_t buffer_size, FileOpStatus& status)
{
    AsyncFile* async_file = reinterpret_cast<AsyncFile*>(handle);
    S_ASSERT(async_file);

    LockT lock(async_file->mut);
    if(async_file->deleted)
    {
        status = IO_FAILED;
        return 0;
    }

    //file write and return status
    FILE* f = async_file->file;
    std::size_t bytes_written = fwrite(buffer, sizeof(char), buffer_size, f);

	if (!ferror(f))
		status = IO_SUCCESS;
	else
		status = IO_FAILED;

    clearerr(f);
    fflush(f);
    return bytes_written;
}

/**
 * a simple unbuffered file appending write. This is
 * slower than the writeFile above, but ensures
 * if an error occurs, that some or most of
 * your bytes may be written to the file anyway.
 *
 * NOTE: this is not truly "unbuffered", but rather
 * "chunked" into 4 byte sections. To do so
 * would require using fileno, which is POSIX std,
 * but not C std. Better to take the safe route
 * 
 * @param handle: the FileHandle to read from
 * @param buffer: the const char* to write from
 * @param buffer_size: the size of your buffer
 * @param status: a reference to a FileOpStatus for error checking
 * @returns a size_t of bytes written
 */
std::size_t IoManager::writeFileUnbuffered(FileHandle handle, 
        const char* buffer, std::size_t buffer_size, FileOpStatus& status)
{
    AsyncFile* async_file = reinterpret_cast<AsyncFile*>(handle);

    LockT lock(async_file->mut);
    if(async_file->deleted)
    {
        status = IO_FAILED;
        return 0;
    }

    //write chunks of 4 chars (pretty slow)
    FILE* f = async_file->file;
	std::size_t bytes_written = 0;
    for(unsigned int i = 0; i < buffer_size; i++)
    {
		fputc(buffer[i], f);

		if ((bytes_written % 4) == 0 || i == buffer_size - 1)
			fflush(f);

		bytes_written++;
    }

	//check for err/return bytes
	if (!ferror(f))
		status = IO_SUCCESS;
	else
		status = IO_FAILED;

	clearerr(f);
	return bytes_written;
}

/**
 * Attempts to cancel an asynchronous job.
 * This might fail if the job is already handled
 * or is currently being handled.
 *
 * @param handle: the AsyncJobHandle of the job
 * @returns true if cancel was sucesful, false otherwise
 */
bool IoManager::cancelAsyncIo(AsyncJobHandle handle) 
{
	return m_thread_pool->cancelAsyncJob(handle);
}

/**
 * blocks until an async job specified finishes.
 * 
 * @param handle: the async job handle
 */
void IoManager::waitAsyncIo(AsyncJobHandle handle) 
{
	m_thread_pool->wait(handle);
}

/**
 * queues up an asynchronous read. The read
 * works the exact same as readFile above, except
 * this funciton returns immediately, and the
 * read occurs at some unspecified time in the future.
 *
 * @param handle: the FileHandle to read from
 * @param buffer: the char* to read into
 * @param buffer_size: size of the buffer
 * @param cb: a std::function<void(FileOpStatus, size_t)> to
 *        call when the read completes.
 * @returns an AsyncJobHandle for waiting or cancelling.
 */
AsyncJobHandle IoManager::asyncRead(FileHandle handle, 
	char* buffer, std::size_t buffer_size,
	const std::function<void(FileOpStatus, std::size_t)>& cb, bool immediate)
{
	return m_thread_pool->asyncDo([this, handle, buffer, buffer_size, cb]()
	{
		FileOpStatus status;
		std::size_t bytes_read = readFile(handle, buffer, buffer_size, status);
		cb(status, bytes_read);
	}, immediate);
}

/**
 * queues up an asynchronous buffered write. The write
 * works the exact same as writeFile above, except
 * this funciton returns immediately, and the
 * write occurs at some unspecified time in the future.
 *
 * @param handle: the FileHandle to write to
 * @param buffer: the const char* to write from
 * @param buffer_size: size of the buffer
 * @param cb: a std::function<void(FileOpStatus, size_t)> to
 *        call when the read completes.
 * @returns an AsyncJobHandle for waiting or cancelling.
 */
AsyncJobHandle IoManager::asyncWrite(FileHandle handle, 
        const char* buffer, std::size_t buffer_size, 
        const std::function<void (FileOpStatus, std::size_t)>& cb, bool immediate)
{
	return m_thread_pool->asyncDo([this, handle, buffer, buffer_size, cb]()
	{
		FileOpStatus status;
		std::size_t bytes_read = writeFile(handle, buffer, buffer_size, status);
		cb(status, bytes_read);
	}, immediate);
}

/**
 * queues up an asynchronous unbuffered write. The write
 * works the exact same as writeFileUnbuffered above, except
 * this funciton returns immediately, and the
 * write occurs at some unspecified time in the future.
 *
 * @param handle: the FileHandle to write to
 * @param buffer: the const char* to write from
 * @param buffer_size: size of the buffer
 * @param cb: a std::function<void(FileOpStatus, size_t)> to
 *        call when the read completes.
 * @returns an AsyncJobHandle for waiting or cancelling.
 */
AsyncJobHandle IoManager::asyncWriteUnbuffered(FileHandle handle,
        const char* buffer, std::size_t buffer_size, 
        const std::function<void (FileOpStatus, std::size_t)>& cb, bool immediate)
{
	return m_thread_pool->asyncDo([this, handle, buffer, buffer_size, cb]()
	{
		FileOpStatus status;
		std::size_t bytes_read = writeFileUnbuffered(handle, buffer, buffer_size, status);
		cb(status, bytes_read);
	}, immediate);
}

}//namespace s_util
