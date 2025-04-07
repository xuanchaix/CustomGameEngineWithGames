#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

JobSystem* g_theJobSystem = nullptr;

JobSystem::JobSystem( JobSystemConfig const& config )
	:m_config(config)
{

}

void JobSystem::StartUp()
{
	if (m_config.m_numOfWorkers == -1) {
		int numOfWorkers = std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1;
		for (int i = 0; i < numOfWorkers; i++) {
			m_workers.push_back( new JobWorkerThread( this, (unsigned int)i ) );
		}
	}
	else {
		for (int i = 0; i < m_config.m_numOfWorkers; i++) {
			m_workers.push_back( new JobWorkerThread( this, (unsigned int)i ) );
		}
	}
}

void JobSystem::BeginFrame()
{

}

void JobSystem::EndFrame()
{

}

void JobSystem::ShutDown()
{
	m_isQuiting = true;
	for (int i = 0; i < (int)m_workers.size(); i++) {
		delete m_workers[i];
	}
}

void JobSystem::AddJob( Job* jobToAdd )
{
	jobToAdd->m_status = JobStatus::Queued;
	m_queuedJobsMutex.lock();
	m_queuedJobs.push_back( jobToAdd );
	m_queuedJobsMutex.unlock();
}

bool JobSystem::RetrieveJob( Job* jobToRetrieve )
{
	m_completedJobsMutex.lock();
	for (auto iter = m_completedJobs.begin(); iter != m_completedJobs.end(); ++iter) {
		if (*iter == jobToRetrieve) {
			m_completedJobs.erase( iter );
			m_completedJobsMutex.unlock();
			jobToRetrieve->m_status = JobStatus::Retrieved;
			return true;
		}
	}
	m_completedJobsMutex.unlock();
	return false;
}

Job* JobSystem::RetriveOldestCompletedJob()
{
	m_completedJobsMutex.lock();
	if ((int)m_completedJobs.size() > 0) {
		Job* jobToReturn = m_completedJobs.front();
		m_completedJobs.pop_front();
		m_completedJobsMutex.unlock();
		jobToReturn->m_status = JobStatus::Retrieved;
		return jobToReturn;
	}
	else {
		m_completedJobsMutex.unlock();
		return nullptr;
	}
}

void JobSystem::CancelJob( Job* jobToCancel )
{
	m_queuedJobsMutex.lock();
	for (auto iter = m_queuedJobs.begin(); iter != m_queuedJobs.end(); ++iter) {
		if (*iter == jobToCancel) {
			m_queuedJobs.erase( iter );
			m_queuedJobsMutex.unlock();
			jobToCancel->m_status = JobStatus::NoRecord;
			return;
		}
	}
	m_queuedJobsMutex.unlock();

}

bool JobSystem::SetWorkerThreadType( int workerID, WorkerThreadType type )
{
	if (workerID >= 0 && workerID < (int)m_workers.size()) {
		m_workers[workerID]->m_workerType = type;
		return true;
	}
	return false;
}

WorkerThreadType JobSystem::GetWorkerThreadType( int workerID ) const
{
	if (workerID >= 0 && workerID < (int)m_workers.size()) {
		return m_workers[workerID]->m_workerType;
	}
	return NoWorkerType;
}

int JobSystem::GetWorkersCount() const
{
	return (int)m_workers.size();
}

Job* JobSystem::WorkerClaimAQueuedJob( JobWorkerThread* worker )
{
	m_queuedJobsMutex.lock();
	if (!m_queuedJobs.empty()) {
		Job* jobToClaim = nullptr;
		auto iter = m_queuedJobs.begin();
		for (; iter != m_queuedJobs.end(); ++iter) {
			if (worker->m_workerType == (*iter)->m_type) {
				jobToClaim = *iter;
				break;
			}
		}
		if (jobToClaim == nullptr) {
			m_queuedJobsMutex.unlock();
			return nullptr;
		}
		m_queuedJobs.erase( iter );
		m_queuedJobsMutex.unlock();
		m_executingJobsMutex.lock();
		m_executingJobs.push_back( jobToClaim );
		jobToClaim->m_status = JobStatus::Executing;
		m_executingJobsMutex.unlock();
		return jobToClaim;
	}
	else {
		m_queuedJobsMutex.unlock();
		return nullptr;
	}
}

void JobSystem::WorkerCompleteAJob( JobWorkerThread* worker, Job* job )
{
	UNUSED( worker );
	m_executingJobsMutex.lock();
	for (auto iter = m_executingJobs.begin(); iter != m_executingJobs.end(); ++iter) {
		if (*iter == job) {
			m_executingJobs.erase( iter );
			m_executingJobsMutex.unlock();
			m_completedJobsMutex.lock();
			m_completedJobs.push_back( job );
			job->m_status = JobStatus::Completed;
			m_completedJobsMutex.unlock();
			return;
		}
	}
	ERROR_RECOVERABLE( "Cannot find a job in executing list to remove!" );
	m_executingJobsMutex.unlock();
	return;
}

JobWorkerThread::JobWorkerThread( JobSystem* jobSystem, unsigned int UID )
	:m_jobSystem(jobSystem)
	,m_UID(UID)
{
	m_thread = new std::thread( &JobWorkerThread::ThreadMain, this );
}

JobWorkerThread::~JobWorkerThread()
{
	m_thread->join();
	delete m_thread;
}

void JobWorkerThread::ThreadMain()
{
	while (!m_jobSystem->m_isQuiting) {
		m_currentJob = ClaimAQueuedJob();
		if (m_currentJob) {
			m_currentJob->Execute();
			m_jobSystem->WorkerCompleteAJob( this, m_currentJob );
		}
		else {
			std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );
		}
	}
}

Job* JobWorkerThread::ClaimAQueuedJob()
{
	return m_jobSystem->WorkerClaimAQueuedJob( this );
}
