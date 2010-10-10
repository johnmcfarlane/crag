/*
 *  Scheduler.cpp
 *  crag
 *
 *  Created by John on 8/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "ForEach.h"

#include "Mutex.h"
#include "Semaphore.h"
#include "SimpleMutex.h"
#include "Thread.h"
#include "ThreadCondition.h"
#include "time.h"

#include "sys/App.h"

#include "core/ConfigEntry.h"
#include "core/for_each.h"


namespace 
{
	
	// Overrides the value passed to scheduler.
	CONFIG_DEFINE(scheduler_num_threads, int, 0);

	
	////////////////////////////////////////////////////////////////////////////////
	// Task definition
	
	// Stores the details unique to a particular task or sub-task.
	// A task is what is basically what is passed into smp::for_each.
	// A sub-task is the same task with a no-greater range of indices.
	struct Task
	{
		bool ExtractSubTask(Task & sub_task)
		{
			if (functor != nullptr)
			{
				Assert(first < last);
				
				// functor
				sub_task.functor = functor;
				
				// first
				sub_task.first = first;
				first += step_size;
				if (first >= last)
				{
					first = last;
					functor = nullptr;
				}

				// last
				sub_task.last = first;

				// step_size
				sub_task.step_size = 1;

				return true;
			}
			
			return false;
		}
		
		smp::Functor * functor;
		int first;
		int last;
		size_t step_size;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Foreman definition
	
	class Foreman
	{
		OBJECT_NO_COPY(Foreman);
		
	public:
		Foreman()
		{
			task.functor = nullptr;
		}
		
		~Foreman()
		{
			Assert(! IsWorking());
		}
		
		bool IsWorking() const
		{
			return task.functor != nullptr; 
		}
		
		void Start(Task const & t)
		{
			Assert(t.first < t.last);
			Assert(t.functor != nullptr);
			
			Assert(! IsWorking());
			task_mutex.Lock();
			task = t;
			Assert(IsWorking());
			task_mutex.Unlock();
		}
		
		// This really only waits for all the work 
		// to be snapped up by the worker threads.
		void WaitForCompletion()
		{
			completion_condition.Wait(completion_mutex);
		}
		
		// Called from worker threads.
		bool GetSubTask(Task & sub_task)
		{
			task_mutex.Lock();
			bool result = task.ExtractSubTask(sub_task);
			task_mutex.Unlock();
			
			if (! result)
			{
				completion_condition.Restart();
			}
			
			return result;
		}

	private:
		smp::Mutex task_mutex; 
		Task task;

		smp::Mutex completion_mutex;
		smp::ThreadCondition completion_condition;
	};
	
	// the foreman
	Foreman * foreman = nullptr;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Worker definitions
	// 
	// This class should roughly equate to one CPU or core. 
	// Chunks of work pass through here.
	
	// counts the worker threads in and out
	smp::Semaphore * worker_semaphore = nullptr;
	
	class Worker
	{
		OBJECT_NO_COPY(Worker);

	public:
		Worker()
		{
			thread.Launch(* this);
		}
		
		~Worker()
		{
			Assert(foreman == nullptr);
			
			thread.Join();
		}
		
	private:
		
		void Run()
		{
			// Loop until it's time to finish up.
			while (true)
			{
				worker_semaphore->Decrement();
				
				if (foreman == nullptr)
				{
					break;
				}
				
				Task t;
				if (foreman->GetSubTask(t))
				{
					(* t.functor) (t.first, t.last);					
				}

				worker_semaphore->Increment();
			}
		}
				
		typedef smp::Thread<Worker, & Worker::Run> Thread;
		Thread thread;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Scheduler variable definitions
	
	typedef std::vector<Worker *> WorkerVector;
	WorkerVector workers;
	
	void ActivateWorkers()
	{
		for (int n = workers.size(); n; -- n)
		{
			worker_semaphore->Increment();
		}
	}
	
	void DeactivateWorkers()
	{
		for (int n = workers.size(); n; -- n)
		{
			worker_semaphore->Decrement();
		}
	}

	
	////////////////////////////////////////////////////////////////////////////////
	// local function definitions
	
	int CalculateNumWorkers(int num_cpus, int num_reserved_cpus)
	{
		if (scheduler_num_threads != 0)
		{
			return scheduler_num_threads;
		}
		
		int num_worker_threads = num_cpus - num_reserved_cpus;
		if (num_worker_threads < 2)
		{
			return 0;
		}
		
		return num_worker_threads;
	}

	void SerialForEach(Task & task)
	{
		Task sub_task;
		while (task.ExtractSubTask(sub_task))
		{
			(* sub_task.functor) (sub_task.first, sub_task.last);
		}
	}
	
	void ParallelForEach(Task & task)
	{
		foreman->Start(task);
		Assert(worker_semaphore->GetValue() == 0);
		
		ActivateWorkers();

		foreman->WaitForCompletion();
		// Last sub_task was dispatched but it may not have finished yet.
		
		DeactivateWorkers();
		
		Assert(! foreman->IsWorking());
	}
	
}


////////////////////////////////////////////////////////////////////////////////
// smp::scheduler definitions

void smp::Init(int num_reserved_cpus)
{
	Assert(foreman == nullptr);
	Assert(worker_semaphore == nullptr);
	Assert(workers.size() == 0);
	
	int num_cpus = sys::GetNumCpus();
	int num_worker_threads = CalculateNumWorkers(num_cpus, num_reserved_cpus);

	if (num_worker_threads > 0)
	{
		foreman = new Foreman;
		worker_semaphore = new Semaphore (0);

		workers.resize(num_worker_threads);
		for (WorkerVector::iterator it = workers.begin(); it != workers.end(); ++ it)
		{
			* it = new Worker();
		}
	}

	std::cout << "scheduler: num CPUs = " << num_cpus << '\n';
	std::cout << "scheduler: num slots = " << num_worker_threads << '\n';
}

void smp::Deinit()
{
	// nullifying the foreman lets the slots know they should quit
	if (foreman != nullptr)
	{
		Assert(! foreman->IsWorking());
		delete foreman;
		foreman = nullptr;
	}

	// but they're still asleep so wake them up
	ActivateWorkers();
	
	// now go away!
	for (WorkerVector::iterator it = workers.begin(); it != workers.end(); ++ it)
	{
		delete * it;
	}
	workers.clear();

	delete worker_semaphore;
	worker_semaphore = nullptr;
}

void smp::ForEach(int f, int l, int ss, Functor & fn)
{
	// Input parameter sanity checks.
	Assert(l >= f);
	if (l <= f)
	{
		// No time wasters please!
		Assert(false);
		return;
	}
	
	// Assign the task to the foreman.
	Task t = { & fn, f, l, ss };
	
	if (worker_semaphore == nullptr)
	{
		SerialForEach(t);
	}
	else
	{
		ParallelForEach(t);
	}
}