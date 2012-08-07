//
//  Scheduler.cpp
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Scheduler.h"

#include "Lock.h"
#include "Semaphore.h"
#include "SimpleMutex.h"
#include "Thread.h"

#include "core/ring_buffer.h"

#define ENABLE_SCHEDULER

#define NUM_RESERVED_CPUS 1


namespace smp
{
	namespace scheduler
	{
#if defined(ENABLE_SCHEDULER)
		
		namespace
		{
			
			// forward-declaration
			void RunThread();
			
			////////////////////////////////////////////////////////////////////////////////
			// Task - a Job and its progress
			
			class Task
			{
			public:
				// functions
				Task(Job & job, int num_units, int priority, bool automatic, Semaphore * num_complete)
				: _job(job)
				, _num_units(num_units)
				, _next_unit(0)
				, _completed_units(0)
				, _priority(priority)
				, _automatic(automatic)
				, _num_complete(num_complete)
				{
				}
				
				~Task()
				{
					ASSERT(! list_type::is_contained(* this));
					
					// If task is blocking, deletion is up to
					// the caller of scheduler::Submit
					if (_automatic)
					{
						delete & _job;
					}
					
					if (_num_complete != nullptr)
					{
						_num_complete->Increment();
					}
				}
				
				// Do the given unit of work on the task.
				void ProcessUnit(int unit_index)
				{
					_job(unit_index);
				}
				
				// compare by priority
				friend bool operator<(Task const & lhs, Task const & rhs) 
				{
					return lhs._priority < rhs._priority;
				}
				
				// Return a newly assigned unit index 
				// (or -ve if none are unassigned).
				int SolicitUnit()
				{
					// Get value of _next_unit and increment it.
					int assignment = std::atomic_fetch_add(& _next_unit, 1);
					
					// If it's within range,
					if (assignment < _num_units)
					{
						// return it.
						return assignment;
					}
					
					// discretely decrement it again
					std::atomic_fetch_sub(& _next_unit, 1);
					
					// and return failure value.
					return -1;
				}
				
				// Register the fact that a unit of work is completed.
				void OnUnitComplete()
				{
					if (std::atomic_fetch_add(& _completed_units, 1) >= _num_units)
					{
						// should never exceed _num_units
						ASSERT(false);
					}
				}
				
				int GetNumUnits() const
				{
					return _num_units;
				}
				
				// Returns true iff all the units are completed.
				bool IsCompleted() const
				{
					return _completed_units == _num_units;
				}
				
				// variables
				DEFINE_INTRUSIVE_LIST(Task, list_type);
			private:
				Job & _job;
				int _num_units;
				std::atomic<int> _next_unit;
				std::atomic<int> _completed_units;
				int _priority;
				bool _automatic;
				Semaphore * _num_complete;
			};
			
			
			////////////////////////////////////////////////////////////////////////////////
			// TaskManager
			
			// Maintains a thread-safe list of tasks.
			class TaskManager
			{
				// types
				typedef SimpleMutex Mutex;
				typedef Lock<Mutex> Lock;
				typedef Task::list_type TaskList;
				typedef TaskList::iterator Iterator;
			public:
				
				////////////////////////////////////////////////////////////////////////////////
				// functions
				
				TaskManager()
				: _semaphore(Semaphore::Create(0))
				{
				}
				
				~TaskManager()
				{
					while (true)
					{
						{
							Lock l(_mutex);
							
							if (_tasks.empty())
							{
								return;
							}
						}

						Yield();
					}
					
					delete & _semaphore;
				}
				
				// called from a thread requesting work
				void Submit(Job & job, int num_units, int priority, bool automatic, Semaphore * num_complete)
				{
					// create a task to represent the given job
					// TODO: Anyway to avoid this dynamic allocation?
					// TODO: Perhaps keep a buffer of them; they're all the same size.
					Task & task = ref(new Task(job, num_units, priority, automatic, num_complete));
					
					AddTask(task);
				}
				
				// "Make yourself useful."
				// Called from a thread volunteering to do work.
				bool ExecuteUnit(bool block)
				{
					Task * task;
					int unit_index;
					
					if (block)
					{
						_semaphore.Decrement();
					}
					else
					{
						if (! _semaphore.TryDecrement())
						{
							return false;
						}
					}
					
					// If a unit of work cannot be acquired,
					if (! SolicitUnit(task, unit_index))
					{
						// return failure: no work was done.
						return false;
					}

					// Perform the work.
					task->ProcessUnit(unit_index);
					
					// Wrap up the work.
					if (OnUnitComplete(* task))
					{
						// If the task is completely done, delete it.
						delete task;
					}
					
					// return success: work was done.
					return true;
				}
				
				void Unblock(int num_threads)
				{
					while ((-- num_threads) >= 0)
					{
						_semaphore.Increment();
					}
				}
				
			private:
				void AddTask(Task & task)
				{
					Lock l(_mutex);
					
					// find the correct position in the queue given the priority
					Iterator position = _tasks.begin();
					for (Iterator end = _tasks.end(); position != end; ++ position)
					{
						Task const & lhs = * position;
						if (lhs < task)
						{
							break;
						}
					}
					
					// insert task at that position
					_tasks.insert(position, task);
					
					Unblock(task.GetNumUnits());
				}
				
				// Try and get a unit of work to do.
				bool SolicitUnit(Task * & task, int & unit_index)
				{
					Lock l(_mutex);
					
					// Loop through non-completed tasks in priority order.
					for (Iterator i = _tasks.begin(); i != _tasks.end(); ++ i)
					{
						Task & t = * i;
						
						// Try and get a unit of work assigned from the task.
						unit_index = t.SolicitUnit();
						
						// If a units to available,
						if (unit_index >= 0)
						{
							// return this task/unit.
							task = & t;
							return true;
						}
						
					}
					
					return false;
				}
				
				// Inform this that an assigned unit of work was completed.
				// Returns true iff the task is complete.
				bool OnUnitComplete(Task & task)
				{
					Lock l(_mutex);
				
					// Increment the completed count.
					task.OnUnitComplete();
					
					// If the task is complete,
					if (task.IsCompleted())
					{
						_tasks.remove(task);
						return true;
					}
					
					return false;
				}
				
				// variables
				TaskList _tasks;
				Mutex _mutex;
				Semaphore & _semaphore;
			};
			
			
			////////////////////////////////////////////////////////////////////////////////
			// ThreadBuffer
			
			// A simple array of threads.
			class ThreadBuffer
			{
			public:
				ThreadBuffer(int num_threads)
				{
					// Create a thread for each CPU (except one).
					_threads = new Thread [num_threads];
					
					// Launch them all.
					for (Thread * it = _threads, * end = _threads + num_threads; it != end; ++ it)
					{
						it->Launch(RunThread);
					}
					
					DEBUG_MESSAGE("scheduler using %d threads.", num_threads);
				}
				
				~ThreadBuffer()
				{
					delete [] _threads;
				}
				
			private:
				// locks entire scheduler
				Thread * _threads;
			};
			
			
			////////////////////////////////////////////////////////////////////////////////
			// Singleton declaration
			
			// Holds all the objects associated with the scheduler.
			class Singleton
			{
			public:
				// functions
				Singleton()
				: _num_threads(GetNumCpus() - NUM_RESERVED_CPUS)
				, _workforce(_num_threads)
				{
				}
				
				~Singleton()
				{
					_task_manager.Unblock(_num_threads);
				}
				
				TaskManager & GetTaskManager()
				{
					return _task_manager;
				}
				
			private:
				// variables
				int _num_threads;
				TaskManager _task_manager;
				ThreadBuffer _workforce;
			};
			
			Singleton * singleton = nullptr;
			
			
			////////////////////////////////////////////////////////////////////////////////
			// RunThread - function of a worker thread
			
			void RunThread()
			{
				// Sleep until after the singleton is full constructed.
				while (singleton == nullptr)
				{
					Yield();
				}
				
				// Singleton & task manager don't ever change until shutdown.
				TaskManager & task_manager = singleton->GetTaskManager();
				
				// Keep working until there's no more work.
				while (task_manager.ExecuteUnit(true))
				{
					ASSERT(singleton != nullptr);
				}
			}
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Threaded version of scheduler interface
		
		void Init()
		{
			ASSERT(singleton == nullptr);
			singleton = new Singleton;
			Yield();
		}
		
		void Deinit()
		{
			Singleton & s = ref(singleton);
			
			// the signal for the threads to quit
			singleton = nullptr;
			
			delete & s;
		}
		
		void Complete(Job & job, int num_units, int priority)
		{
			TaskManager & task_manager = singleton->GetTaskManager();

			// contains a count of the jobs completed
			Semaphore & num_complete(Semaphore::Create(0));

			task_manager.Submit(job, num_units, priority, false, & num_complete);
			
			num_complete.Decrement();
			
			ASSERT(num_complete.GetValue() == 0);
			delete & num_complete;
		}
		
		void Complete(Batch & batch, int priority)
		{
			TaskManager & task_manager = singleton->GetTaskManager();
			
			int num_jobs = 0;
			Semaphore & num_complete(Semaphore::Create(0));
			
			for (Batch::const_iterator i = batch.begin(); i != batch.end(); ++ num_jobs, ++ i)
			{
				Job & job = * i;
				task_manager.Submit(job, 1, priority, false, & num_complete);
			}
			
			for (; num_jobs > 0; -- num_jobs)
			{
				num_complete.Decrement();
			}
			
			ASSERT(num_complete.GetValue() == 0);
			delete & num_complete;
		}		
#else
		////////////////////////////////////////////////////////////////////////////////
		// Non-threaded version of scheduler interface
		//
		// Handy for ruling out threading issues when debugging
		
		void Init() { }
		
		void Deinit() { }
		
		void Complete(Job & job, int num_units, int priority)
		{
			for (int unit_index = 0; unit_index < num_units; ++ unit_index)
			{
				job(unit_index);
			}
		}
		
		void Complete(Batch & batch, int priority)
		{
			for (Batch::const_iterator i = batch.begin(); i != batch.end(); ++ i)
			{
				Complete(* i, 1, priority);
			}
		}
#endif
	}
}
