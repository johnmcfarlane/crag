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

#include "atomic.h"
#include "Lock.h"
#include "SimpleMutex.h"
#include "Thread.h"

#include "core/intrusive_list.h"

#define ENABLE_SCHEDULER


namespace smp
{
	namespace scheduler
	{
#if defined(ENABLE_SCHEDULER)

		namespace
		{
			
			// forward-declaration
			int RunThread(void *);
			
			////////////////////////////////////////////////////////////////////////////////
			// Task - a Job and its progress
			
			class Task
			{
				// types
				typedef core::intrusive::hook<Task> hook_type;
				
			public:
				// functions
				Task(Job & job, int num_units, int priority, bool block)
				: _job(job)
				, _num_units(num_units)
				, _next_unit(0)
				, _completed_units(0)
				, _priority(priority)
				, _block(block)
				{
				}
				
				~Task()
				{
					Assert(_hook.is_detached());
					
					// If task is blocking, deletion is up to
					// the caller of scheduler::Submit
					if (! IsBlocking())
					{
						delete & _job;
					}
				}
				
				// Is this a task which is blocking the calling thread?
				bool IsBlocking() const
				{
					return _block;
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
					// get value of _next_unit and increment it
					int assignment = AtomicFetchAndAdd(_next_unit, 1);
					
					// if we are out of units,
					if (assignment >= _num_units)
					{
						// discretely decrement it again
						AtomicFetchAndAdd(_next_unit, -1);
						
						// and return failure value.
						return -1;
					}
					
					return assignment;
				}
				
				// Register the fact that a unit of work is completed.
				void OnUnitComplete()
				{
					if (AtomicFetchAndAdd(_completed_units, 1) > _num_units)
					{
						// should never exceed _num_units
						Assert(false);
					}
				}
				
				// Returns true iff all the units are completed.
				bool IsCompleted() const
				{
					return _completed_units == _num_units;
				}
				
			private:
				// variables
				hook_type _hook;
				Job & _job;
				int _num_units;
				int _next_unit;
				int _completed_units;
				int _priority;
				bool _block;
				
			public:
				// type of the list in which these are stored
				typedef core::intrusive::list<Task, & Task::_hook> list_type;
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
				
				~TaskManager()
				{
					while (! _tasks.empty())
					{
						Sleep(0);
					}
				}

				// called from a thread requesting work
				void Submit(Job & job, int num_units, int priority, bool block)
				{
					// create a task to represent the given job
					Task & task = ref(new Task(job, num_units, priority, block));

					AddTask(task);
					
					Finish(task);
				}
				
				// called from a thread volunteering to do work
				bool ExecuteUnit()
				{
					Task * task;
					int unit_index;
					
					if (SolicitUnit(task, unit_index))
					{
						task->ProcessUnit(unit_index);
						
						OnUnitComplete(* task);
						return true;
					}
					else
					{
						return false;
					}
				}
				
			private:
				void AddTask(Task & task)
				{
					Lock l(_mutex);
					
					// true only while scheduler is used blockingly in one thread.
					Assert(_tasks.empty());
					
					// find the correct position in the queue given the priority
					Iterator position = _tasks.begin();
					for (Iterator end = _tasks.end(); position != end; ++ position)
					{
						Task const & lhs = * position;
						if (! (lhs < task))
						{
							break;
						}
					}
					
					// insert task at that position
					_tasks.insert(position, task);
				}
				
				void Finish(Task & task)
				{
					// If blocking, 
					if (task.IsBlocking())
					{
						// wait for the task to be completed,
						while (! task.IsCompleted())
						{
							ExecuteUnit();
						}
						
						// and delete it.
						RemoveAndDelete(task);
					}
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
				void OnUnitComplete(Task & task)
				{
					// Increment the completed count.
					task.OnUnitComplete();
					
					// If the task is complete,
					if (task.IsCompleted())
					{
						// If scheduler is responsible for freeing it,
						if (! task.IsBlocking())
						{
							// remove/delete the task.
							RemoveAndDelete(task);
						}
					}
				}
				
				// Removes the task from the list and deletes it.
				// (Doesn't necessarily delete the job.)
				void RemoveAndDelete(Task & task)
				{
					Lock l(_mutex);
					
					_tasks.remove(task);
				}

				// variables
				TaskList _tasks;
				Mutex _mutex;
			};
			
			
			////////////////////////////////////////////////////////////////////////////////
			// ThreadBuffer
			
			// A simple array of threads.
			class ThreadBuffer
			{
			public:
				ThreadBuffer()
				{
					// Create a thread for each CPU.
					int num_threads = GetNumCpus();
					_threads = new Thread [num_threads];
					
					// Launch them all.
					for (Thread * it = _threads, * end = _threads + num_threads; it != end; ++ it)
					{
						it->Launch(RunThread, nullptr);
					}
					
					std::cout << "scheduler using " << num_threads << "threads." << std::endl;
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
				// function
				TaskManager & GetTaskManager()
				{
					return _task_manager;
				}
				
			private:
				// variables
				TaskManager _task_manager;
				ThreadBuffer _workforce;
			};
			
			Singleton * singleton = nullptr;
			
			
			////////////////////////////////////////////////////////////////////////////////
			// RunThread - function of a worker thread
			
			int RunThread(void *)
			{
				SetThreadName("worker");
				
				// Sleep until after the singleton is full constructed.
				while (singleton == nullptr)
				{
					Sleep(0);
				}
				
				// Singleton & task manager don't ever change until shutdown.
				TaskManager & task_manager = singleton->GetTaskManager();

				// Until we're all done,
				while (singleton != nullptr)
				{
					// do work
					if (! task_manager.ExecuteUnit())
					{
						// or sleep.
						// TODO: Start using semaphores again.
						Sleep(0.01);
					}
				}
				
				return 0;
			}
		}


		////////////////////////////////////////////////////////////////////////////////
		// Threaded version of scheduler interface

		void Init()
		{
			Assert(singleton == nullptr);
			singleton = new Singleton;
		}

		void Deinit()
		{
			Singleton & s = ref(singleton);
			
			// the signal for the threads to quit
			singleton = nullptr;
			
			delete & s;
		}

		void Submit(Job & job, int num_units, int priority, bool block)
		{
			switch (num_units)
			{
				case 0:
					break;
					
				case 1:
					if (block)
					{
						// We're blocking the thread anyway, so only bother to parallelize 
						// if there is more than one unit of work to perform.
						job(0);
						break;
					}
					
				default:
					singleton->GetTaskManager().Submit(job, num_units, priority, block);
					break;
			}
		}

#else
		////////////////////////////////////////////////////////////////////////////////
		// Non-threaded version of scheduler interface
		//
		// Handy for ruling out threading issues when debugging

		void Init() { }

		void Deinit() { }

		void Submit(Job & job, int num_units, int priority, bool block)
		{
			for (size_t unit_index = 0; unit_index < num_units; ++ unit_index)
			{
				job(unit_index);
			}
		}
#endif
	}
}
