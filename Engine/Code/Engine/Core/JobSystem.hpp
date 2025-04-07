#pragma once

#include <deque>
#include <vector>
#include <mutex>
#include <thread>

class JobSystem;

enum class JobStatus {
	NoRecord, Queued, Executing, Completed, Retrieved, 
};

typedef uint32_t JobType;

typedef JobType WorkerThreadType;

constexpr JobType NoType = 0x0;
constexpr JobType CommonJob = 0x1;

constexpr WorkerThreadType NoWorkerType = 0x0;
constexpr WorkerThreadType CommonWorker = 0x1;

class Job {
public:
	Job(JobType type = CommonJob ) : m_type(type) {};
	virtual ~Job(){};
	virtual void Execute() = 0;
	std::atomic<JobStatus> m_status = JobStatus::NoRecord;
	std::atomic<JobType> m_type = CommonJob;
};

class JobWorkerThread {
	friend class JobSystem;
protected:
	JobWorkerThread( JobSystem* jobSystem, unsigned int UID );
	virtual ~JobWorkerThread();
	void ThreadMain();
	Job* ClaimAQueuedJob();

	std::thread* m_thread = nullptr;
	unsigned int m_UID = (unsigned int)-1;
	JobSystem* m_jobSystem = nullptr;
	Job* m_currentJob = nullptr;

	std::atomic<WorkerThreadType> m_workerType = CommonJob;
};

struct JobSystemConfig {
	int m_numOfWorkers = -1; // -1: depends on how may physical cores the computer have
};

class JobSystem {
	friend class JobWorkerThread;
public:
	JobSystem( JobSystemConfig const& config );

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void AddJob( Job* jobToAdd );
	bool RetrieveJob( Job* jobToRetrieve );
	Job* RetriveOldestCompletedJob();
	void CancelJob( Job* jobToCancel );
	bool SetWorkerThreadType( int workerID, WorkerThreadType type );
	WorkerThreadType GetWorkerThreadType( int workerID ) const;
	int GetWorkersCount() const;

protected:
	std::atomic<bool> m_isQuiting = false;
	Job* WorkerClaimAQueuedJob( JobWorkerThread* worker );
	void WorkerCompleteAJob( JobWorkerThread* worker, Job* job );

	JobSystemConfig m_config;

	std::vector<JobWorkerThread*> m_workers;

	std::deque<Job*> m_completedJobs;
	std::mutex m_completedJobsMutex;
	std::deque<Job*> m_queuedJobs;
	std::mutex m_queuedJobsMutex;
	std::deque<Job*> m_executingJobs;
	std::mutex m_executingJobsMutex;
};

