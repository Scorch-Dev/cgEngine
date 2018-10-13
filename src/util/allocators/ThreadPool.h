#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <deque>
#include <thread>
#include <vector>

#include "MemoryPool.h"

namespace sentinel
{

typedef std::size_t AsyncJobHandle; //an AsyncJob's id field
extern AsyncJobHandle NULL_JOB_HANDLE;

enum AsyncStatus : int
{
	aborted = -2,
	failed = -1,
	pending = 0,
	success = 1,
};

class ThreadPool
{
public:
	ThreadPool(std::size_t num_threads);
	~ThreadPool();

	AsyncJobHandle asyncDo(std::function<void (void)> func, bool immediate=false);
	bool cancelAsyncJob(AsyncJobHandle handle); //can still fail
	void wait(AsyncJobHandle handle);

private:

	typedef std::thread ThreadT;
	typedef std::mutex MutexT;
	typedef std::condition_variable ConditionT;
	typedef std::unique_lock<MutexT> LockT;

	struct AsyncJob{
		static std::size_t s_next_id;
		std::size_t m_id;
		AsyncStatus m_status;
		std::function<void (void)> m_func;
		MutexT m_mut;

		AsyncJob(std::function<void(void)> func) :
			m_id(s_next_id++),
			m_status(pending),
			m_func(std::move(func))
		{}
	};


	void handleAsyncJob(AsyncJob& job);

	//q to store async jobs
	std::deque<AsyncJob*> m_async_job_q;
	MutexT m_async_job_q_mut;
	ConditionT m_async_job_q_cond;

	//async workers
	std::vector<ThreadT> m_async_workers;
	bool m_handling_async;

	//wait on job without regard for job lifespan
	std::map<AsyncJobHandle, AsyncJob*> m_live_async_jobs;
	MutexT m_live_async_jobs_mut;
	ConditionT m_live_async_jobs_cond;

	MemoryPool m_async_job_pool;
};

}//namespace sentinel

#endif