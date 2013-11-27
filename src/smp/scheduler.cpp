//
//  Scheduler.cpp
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "scheduler.h"

#include "Semaphore.h"
#include "SimpleMutex.h"
#include "Thread.h"

#include "core/ring_buffer.h"

//#define ENABLE_SCHEDULER

namespace smp
{
	namespace scheduler
	{
		namespace
		{
			
			// forward-declaration
			void RunThread();
			
			////////////////////////////////////////////////////////////////////////////////
			// helper function
			
#if defined(ENABLE_SCHEDULER)
			size_t CalculateNumThreads(size_t num_reserved_cpus)
			{
				size_t num_cpus = GetNumCpus();
				if (num_cpus > num_reserved_cpus)
				{
					// The remainder of the CPUs after some have been reserved.
					return num_cpus - num_reserved_cpus;
				}
				else
				{
					// There are not enough CPUs left over to justify a scheduler.
					return 0;
				}
			}
#else
			size_t CalculateNumThreads(size_t)
			{
				// No threads.
				return 0;
			}
#endif

			////////////////////////////////////////////////////////////////////////////////
			// Task - a Job and its progress
			
			class Task
			{
			public:
				// functions
				Task(Job & job, size_t num_units, int priority, bool automatic, Semaphore * num_complete)
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
				void ProcessUnit(size_t unit_index)
				{
					_job(unit_index);
				}
				
				// compare by priority
				friend bool operator<(Task const & lhs, Task const & rhs) 
				{
					return lhs._priority < rhs._priority;
				}
				
				// Get the index of the next unit to be processed
				// or return false.
				bool SolicitUnit(size_t & unit_index)
				{
					// Get value of _next_unit and increment it.
					unit_index = _next_unit.fetch_add(1, std::memory_order_relaxed);
					
					// If it's within range,
					if (unit_index < _num_units)
					{
						// return success.
						return true;
					}
					
					// Otherwise, discretely decrement it again
					_next_unit.fetch_sub(1, std::memory_order_relaxed);
					
					// and return failure.
					return false;
				}
				
				// Register the fact that a unit of work is completed.
				void OnUnitComplete()
				{
					if (_completed_units.fetch_add(1, std::memory_order_relaxed) >= _num_units)
					{
						// should never exceed _num_units
						ASSERT(false);
					}
				}
				
				size_t GetNumUnits() const
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
				size_t _num_units;
				std::atomic<size_t> _next_unit;
				std::atomic<size_t> _completed_units;
				int _priority;
				bool _automatic;
				Semaphore * _num_complete;
			};
			
			
			////////////////////////////////////////////////////////////////////////////////
			// TaskManager
			
			// store for tasks for scheduler to perform
			class TaskManager
			{
			public:
				virtual ~TaskManager()
				{
				}

				// adds a job to be scheduled
				virtual void Submit(Job & job, size_t num_units, int priority, bool automatic, Semaphore * num_complete) = 0;
				virtual bool ExecuteUnit(bool block) = 0;
				virtual void Unblock(size_t /*num_threads*/) { }
			};
			
			// Maintains a thread-safe list of tasks.
			class SimpleTaskManager : public TaskManager
			{
			public:
				
				////////////////////////////////////////////////////////////////////////////////
				// functions
				
				// called from a thread requesting work
				virtual void Submit(Job & job, size_t num_units, int /*priority*/, bool /*automatic*/, Semaphore * num_complete) override
				{
					for (size_t unit = 0; unit < num_units; ++ unit)
					{
						job(unit);
					}
					
					if (num_complete)
					{
						num_complete->Increment();
					}
				}
				
				// "Make yourself useful."
				// Called from a thread volunteering to do work.
				virtual bool ExecuteUnit(bool /*block*/) override
				{
					DEBUG_BREAK("There's never work to do with SimpleTaskManager");
					return false;
				}
			};
			
			// Maintains a thread-safe list of tasks.
			class MultithreadedTaskManager : public TaskManager
			{
				// types
				typedef std::mutex Mutex;
				typedef std::lock_guard<Mutex> Lock;
				typedef Task::list_type TaskList;
				typedef TaskList::iterator Iterator;
			public:
				
				////////////////////////////////////////////////////////////////////////////////
				// functions
				
				MultithreadedTaskManager()
				: _semaphore(Semaphore::Create(0))
				{
				}
				
				~MultithreadedTaskManager()
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
				virtual void Submit(Job & job, size_t num_units, int priority, bool automatic, Semaphore * num_complete) override
				{
					// create a task to represent the given job
					// TODO: Anyway to avoid this dynamic allocation?
					// TODO: Perhaps keep a buffer of them; they're all the same size.
					Task & task = ref(new Task(job, num_units, priority, automatic, num_complete));
					
					AddTask(task);
				}
				
				// "Make yourself useful."
				// Called from a thread volunteering to do work.
				virtual bool ExecuteUnit(bool block) override
				{
					Task * task;
					size_t unit_index;
					
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
				
				virtual void Unblock(size_t num_threads) override
				{
					ASSERT(num_threads > 0);
					do
					{
						-- num_threads;
						_semaphore.Increment();
					}	while (num_threads != 0);
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
				bool SolicitUnit(Task * & task, size_t & unit_index)
				{
					Lock l(_mutex);
					
					// Loop through non-completed tasks in priority order.
					for (Iterator i = _tasks.begin(); i != _tasks.end(); ++ i)
					{
						Task & t = * i;
						
						// If a unit is available,
						if (t.SolicitUnit(unit_index))
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
				OBJECT_NO_COPY(ThreadBuffer);

				// types
				typedef std::vector<Thread> vector;
			public:
				ThreadBuffer(std::size_t num_threads)
				: _threads(new Thread [num_threads])
				, _threads_end(_threads + num_threads)
				{
					// Launch them all.
					for (Thread * it = _threads; it != _threads_end; ++ it)
					{
						it->Launch(RunThread, "scheduler");
					}
				}
				
				vector::size_type size() const
				{
					return _threads_end - _threads;
				}
				
				~ThreadBuffer()
				{
					for (Thread * it = _threads; it != _threads_end; ++ it)
					{
						it->Join();
					}
					
					delete [] _threads;
				}	
				
			private:
				// locks entire scheduler
				Thread * const _threads;
				Thread const * const _threads_end;
			};
			
			
			////////////////////////////////////////////////////////////////////////////////
			// Singleton declaration
			
			// Holds all the objects associated with the scheduler.
			class Singleton
			{
			public:
				// functions
				Singleton(size_t num_threads)
				: _task_manager(ref(num_threads
									? static_cast<TaskManager*>(new MultithreadedTaskManager)
									: static_cast<TaskManager*>(new SimpleTaskManager)))
				, _thread_buffer(num_threads)
				{
					if (num_threads > 0)
					{
						Yield();
					}
				}
				
				~Singleton()
				{
					_task_manager.Unblock(_thread_buffer.size());

					delete & _task_manager;
				}
				
				TaskManager & GetTaskManager()
				{
					return _task_manager;
				}
				
			private:
				// variables
				TaskManager & _task_manager;
				ThreadBuffer _thread_buffer;
			};
			
			Singleton * singleton = nullptr;
			
			
			////////////////////////////////////////////////////////////////////////////////
			// RunThread - function of a worker thread
			
			void RunThread()
			{
				core::DebugSetThreadName("schd");

				// Sleep until after the singleton is fully constructed.
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
		
		void Init(size_t num_reserved_cpus)
		{
			ASSERT(singleton == nullptr);

			size_t num_threads = CalculateNumThreads(num_reserved_cpus);
			DEBUG_MESSAGE("scheduler using " SIZE_T_FORMAT_SPEC " threads.", num_threads);

			singleton = new Singleton(num_threads);
		}
		
		void Deinit()
		{
			Singleton & s = ref(singleton);
			
			// the signal for the threads to quit
			singleton = nullptr;
			
			delete & s;
		}
		
		void Complete(Job & job, size_t num_units, int priority)
		{
			TaskManager & task_manager = singleton->GetTaskManager();

			// contains a count of the jobs completed
			Semaphore & num_complete(Semaphore::Create(0));

			task_manager.Submit(job, num_units, priority, false, & num_complete);
			
			while (! num_complete.TryDecrement())
			{
				task_manager.ExecuteUnit(false);
			}
			
			ASSERT(num_complete.GetValue() == 0);
			delete & num_complete;
		}
		
		void Complete(Batch & batch, int priority)
		{
			CRAG_VERIFY(batch);
			
			TaskManager & task_manager = singleton->GetTaskManager();
			
			size_t num_jobs = 0;
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
	}
}
