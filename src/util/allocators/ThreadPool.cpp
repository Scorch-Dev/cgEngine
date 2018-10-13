#include "ThreadPool.h"

namespace sentinel
{

AsyncJobHandle NULL_JOB_HANDLE = 0; //no jobs have this id

std::size_t ThreadPool::AsyncJob::s_next_id = 1;

/**
 *ctor
 * on construction, spins up N
 * threads and creates a job queue
 * in the thread pool.
 *
 * @param num_threads: threads to spin up
 */
ThreadPool::ThreadPool(std::size_t num_threads) :
	m_async_workers(num_threads),
	m_handling_async(false),
	m_async_job_pool(sizeof(AsyncJob))
{
	m_handling_async = true;

	//each thread waits runs tasks for the queue
	for (int i = 0; i < m_async_workers.size(); i++)
	{
		m_async_workers[i] = std::thread(
			[this]() {

			LockT q_lock(m_async_job_q_mut, std::defer_lock);
			LockT live_jobs_lock(m_live_async_jobs_mut, std::defer_lock);

			while (m_handling_async)
			{
				//wait for a job to be available on the q
				q_lock.lock();
				m_async_job_q_cond.wait(q_lock, [this]() {
					return (m_handling_async == false
						|| m_async_job_q.empty() == false); });

				if (!m_handling_async && m_async_job_q.empty())
					return;

				AsyncJob* job = m_async_job_q.front();
				m_async_job_q.pop_front();
				q_lock.unlock();

				//handle the job and notify any wait calls on it
				handleAsyncJob(*job);

				live_jobs_lock.lock();
				m_live_async_jobs.erase(job->m_id);
				live_jobs_lock.unlock();
				m_live_async_jobs_cond.notify_all();

				job->~AsyncJob();
				m_async_job_pool.freeBlock(job);
			}

		});
	}
}

/**
 * a blocking dtor, which will wait for all jobs
 * in the queue to finish then return.
 * Caution is advised in timing destruction.
 */
ThreadPool::~ThreadPool()
{
	m_handling_async = false; //signal threads to return
	m_async_job_q_cond.notify_all();

	//join all outstanding async workers
	for (int i = 0; i < m_async_workers.size(); i++)
	{
		if (m_async_workers[i].joinable())
			m_async_workers[i].join();
	}
}

/**
 * queues up an async operation
 * of type std::function<void(void)>. This
 * works incredibly well with lambdas.
 *
 * @param func: the function to enqueue
 */
AsyncJobHandle ThreadPool::asyncDo(std::function<void(void)> func, bool immediate)
{
	LockT q_lock(m_async_job_q_mut, std::defer_lock);
	LockT live_jobs_lock(m_live_async_jobs_mut, std::defer_lock);
	std::lock(q_lock, live_jobs_lock);

	void* mem_ptr = m_async_job_pool.alloc();
	AsyncJob* async_job;

	async_job = new(mem_ptr) AsyncJob(func);

	//immediate jobs expedited to front
	if(!immediate)
		m_async_job_q.push_back(async_job);
	else
		m_async_job_q.push_front(async_job);


	m_live_async_jobs.emplace(async_job->m_id, async_job);

	live_jobs_lock.unlock();
	q_lock.unlock();

	//let the waiting threads know_job is available
	m_async_job_q_cond.notify_one();
	return (async_job->m_id);
}

/**
 * attempts to cancel an asynchronous job
 * by handle. There is no guarentee that this
 * function succeeds (obviously in the case
 * that the function already fired). If it fails,
 * then the return indicates this.
 *
 * @param handle: the handle of the job to cancel
 * @returns true if succeeded cancel, false otherwise
 */
bool ThreadPool::cancelAsyncJob(AsyncJobHandle handle)
{
	//check if it's even alive
	LockT live_jobs_lock(m_live_async_jobs_mut);
	if (!m_live_async_jobs.count(handle))
		return false;

	//set status to aborted
	AsyncJob* job = m_live_async_jobs.at(handle);
	LockT job_lock(job->m_mut);

	bool success = true;

	if (job->m_status == pending)
		job->m_status = aborted;
	else
		success = false;

	job_lock.unlock();
	live_jobs_lock.unlock();

	return success;
}

/**
* blocks until an async job specified finishes.
*
* @param handle: the async job handle
*/
void ThreadPool::wait(AsyncJobHandle handle)
{
	LockT live_jobs_lock(m_live_async_jobs_mut);

	if (!m_live_async_jobs.count(handle))
		return; //nothing to do here :)

				//wait for the job
	m_live_async_jobs_cond.wait(live_jobs_lock,
		[this, &handle]() {return(m_live_async_jobs.count(handle) == 0); });
}

/**
 * A private callback to process jobs from the queue.
 *
 * @param job: the AsyncJob to process
 */
void ThreadPool::handleAsyncJob(AsyncJob& job)
{
	//grab mutex to avoid race on job & use func
	LockT job_lock(job.m_mut);

	if (job.m_status == aborted)
		return;

	job.m_func();
}

}//namespace sentinel