/*
 *  Scheduler.cpp
 *  crag
 *
 *  Created by John on 8/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Scheduler.h"

#include "App.h"
#include "SimpleMutex.h"
#include "Thread.h"

#include "core/ConfigEntry.h"


int dcc_ncpus(int *ncpus);


namespace
{
	
	CONFIG_DEFINE(scheduler_num_threads, int, 0);
	
	//bool volatile busy = false;
	
	
	// Returns the number of CPUs on this computer that will
	// be used by the Scheduler. On failure, returns 1. 
	int GetNumCpus()
	{
		// Do we want to use the override? 
		if (scheduler_num_threads > 0)
		{
			return scheduler_num_threads;
		}
		
		// Use a function I dug up on the interweb to get the number of CPUs 
		// (because SDL doesn't yet have such a function).
		int ncpus;
		if (dcc_ncpus(& ncpus) == 0)
		{
			return ncpus;
		}
		
		return 1;
	}
	
}


////////////////////////////////////////////////////////////////////////////////
// sys::Scheduler::Task definitions

class sys::Scheduler::Task
{
public:
	Task()
	: f (nullptr)
	{
	}
	
	Task(int init_first, int init_last, Functor * init_f)
	: first (init_first)
	, last (init_last)
	, f (init_f)
	{
	}

	operator bool () const
	{
		return f != nullptr;
	}
	
	bool operator ! () const
	{
		return f == nullptr;
	}
	
	void Clear() 
	{
		f = nullptr;
	}
	
	void Execute() 
	{
		Assert(f != nullptr);
		(* f) (first, last);
		//busy = false;
	}
	
private:
	int volatile first;
	int volatile last;
	Functor * volatile f;
};


////////////////////////////////////////////////////////////////////////////////
// sys::Scheduler::Slot definitions
// 
// This class should roughly equate to one CPU or core. 
// Chunks of work pass through here.

class sys::Scheduler::Slot
{
public:
	Slot()
	: quit(false)
	{
		sys::Scheduler::Slot * data = this;
		thread = new Thread (Callback, data);
	}
	
	~Slot()
	{
		quit = true;
		thread->Join();
		delete thread;
	}
	
	bool Accept(Task const & p)
	{
		Assert (! quit);
		
		bool accepted;

		Lock(0);
		if (! pending_tasks[0])
		{
			pending_tasks[0] = p;
			accepted = true;
			//busy = true;
		}
		else if (! pending_tasks[1])
		{
			pending_tasks[1] = p;
			accepted = true;
			//busy = true;
		}
		else
		{
			accepted = false;
		}
		Unlock(0);
		
		return accepted;
	}
	
	bool HasPendingTasks() const
	{
		Lock(0);
		bool has_pending_tasks = current_task || pending_tasks[0] || pending_tasks[1];
		Unlock(0);

		return has_pending_tasks;
	}
	
private:
	
	static void Callback(Slot * slot)
	{
		slot->Run();
	}
	
	void Run()
	{
		// Loop until it's time to finish up.
		while (! quit)
		{
			Lock(1);
			// Loop for as long as there's something to do...
			while (true)
			{
				// Take a task from the pile (if there is one).
				if (pending_tasks[0])
				{
					current_task = pending_tasks[0];
					pending_tasks[0].Clear();
				}
				else if (pending_tasks[1])
				{
					current_task = pending_tasks[1];
					pending_tasks[1].Clear();
				}
				
				// Did we get one?
				if (current_task)
				{
					// If so, execute it,
					current_task.Execute();
					current_task.Clear();
				}
				else 
				{
					// else, we're done performing tasks.
					break;
				}
			}
			Unlock(1);

			// Once there's nothing to do, sleep to give the other thread time to give this one more to do. 
			app::Sleep();
		}
	}
	
	void Lock(int thread_id) const
	{
		mutex.Lock(thread_id);
	}
	
	void Unlock(int thread_id) const
	{
		mutex.Unlock(thread_id);
	}
	
	Task pending_tasks[2];
	Task current_task;
	
	sys::Thread * thread;
	SimpleMutex mutable mutex;
	//core::Mutex mutable mutex;
	bool volatile quit;
};


////////////////////////////////////////////////////////////////////////////////
// sys::Scheduler definitions

sys::Scheduler::Scheduler()
: num_slots (GetNumCpus())
, slots (new Slot [num_slots])
{
}

sys::Scheduler::~Scheduler()
{
	delete [] slots;
}

void sys::Scheduler::Dispatch(Functor & f, int first, int last, int step)
{
	int sub_first, sub_last = first;
	while (true)
	{
		sub_first = sub_last;
		sub_last += step;
		if (sub_last > last)
		{
			DispatchSub(f, sub_first, last);
			break;
		}
		else 
		{
			DispatchSub(f, sub_first, sub_last);
		}
	}
	
	while (! IsComplete())
	{
		app::Sleep();
	}
}

void sys::Scheduler::DispatchSub(Functor & f, int first, int last)
{
	if (first == last)
	{
		return;
	}

	Task p(first, last, & f);
	
	Slot const * const slots_end = slots + num_slots;
	while (true)
	{
		for (Slot * i = slots; i != slots_end; ++ i)
		{
			if (i->Accept(p))
			{
				return;
			}
		}
		
		app::Sleep();
	}
}

bool sys::Scheduler::IsComplete() const
{
	Slot const * const slots_end = slots + num_slots;
	
	for (Slot * i = slots; i != slots_end; ++ i)
	{
		if (i->HasPendingTasks())
		{
			return false;
		}
	}
	
	//Assert(busy == false);
	return true;
}
