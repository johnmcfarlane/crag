//
//  Daemon.h
//  crag
//
//  Created by John McFarlane on 4/26/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "MessageQueue_Impl.h"
#include "smp/Thread.h"

#include "core/ring_buffer.h"

#include "ipc/ListenerInterface.h"

#include "core/app.h"

namespace ipc
{
	////////////////////////////////////////////////////////////////////////////////
	// thread-safe wrapper for a service/sub-system (engine)
	
	template <typename ENGINE>
	class Daemon
	{
		OBJECT_NO_COPY(Daemon);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		enum class State
		{
			initialized,
			running,
			acknowledge_flush_begin,
			request_flush_end,
			acknowledge_flush_end,
			request_destroy
		};

		typedef smp::SimpleMutex Mutex;
	public:
		typedef ENGINE Engine;
		typedef ::ipc::MessageQueue<Engine> MessageQueue;
		
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Daemon(size_t ring_buffer_size)
		: _engine(nullptr)
		, _messages(ring_buffer_size)
		, _state(State::initialized)
		, _state_change_time(-1)
#if defined(CRAG_DEBUG)
		, _name(nullptr)
#endif
		{
			ASSERT(singleton == nullptr);
			singleton = this;
		}
		
		~Daemon() 
		{ 
			CRAG_VERIFY_EQUAL_ENUM(_state, State::acknowledge_flush_end);
			SetState(State::request_destroy);
			
			ASSERT(singleton == this);
			singleton = nullptr;
			
			_thread.Join();

			// Never called from within the thread.
			ASSERT(! _thread.IsCurrent());
			
			// Must call Stop from the outside thread first.
			ASSERT(! _thread.IsLaunched());
			
			ASSERT(_messages.IsEmpty());
			
			ASSERT(_engine == nullptr);
		}
		
		// specifically, the thread is still in the engine's Run function
		bool IsRunning() const
		{
			return singleton->_state <= State::running;
		}
		
		// true iff called from the Daemon's own thread
		static bool IsCurrentThread()
		{
			return singleton->_thread.IsCurrent();
		}
		
		// the amount of time the Daemon will allow the thread to run 
		// on program shutdown before forcefully quitting
		static float ShutdownTimeout()
		{
			return 2.f;
		}
		
		size_t GetQueueCapacity() const
		{
			return _messages.capacity();
		}
		
		void Start(char const * name)
		{
			ASSERT(! singleton->_thread.IsCurrent());

#if defined(CRAG_DEBUG)
			ASSERT(_name == nullptr);
			_name = name;
#endif

			_thread.Launch([this, name] () {
				core::DebugSetThreadName(name);
				this->Run();
			}, name);
			
			while (_engine == nullptr)
			{
				// block until the the engine is constructed and assigned
				smp::Yield();
			}
		}
		
		// Tell the daemon to wind down.
		void BeginFlush()
		{
			ASSERT(! singleton->_thread.IsCurrent());
			ASSERT(_state <= State::acknowledge_flush_begin);
			
			// Never called from within the thread.
			ASSERT(! _thread.IsCurrent());
			
			Call([] (Engine & engine) { engine.OnQuit(); });
		}
		
		// Wait until it has stopped working.
		void Synchronize()
		{
			ASSERT(! singleton->_thread.IsCurrent());
			ASSERT(_state <= State::acknowledge_flush_end);
			
			while (_state < State::acknowledge_flush_begin)
			{
				if (IsTimedout())
				{
					DEBUG_MESSAGE("Engine, %s, has spent too long in state, %d", _name, (int)_state);
					return;
				}
				
				smp::Yield();
			}

			ASSERT(_state <= State::acknowledge_flush_end);
		}
		
		// Wait until it has finished flushing.
		void EndFlush()
		{
			CRAG_VERIFY_FALSE(singleton->_thread.IsCurrent());			
			CRAG_VERIFY_EQUAL_ENUM(_state, State::acknowledge_flush_begin);
			
			SetState(State::request_flush_end);

			while (_state < State::acknowledge_flush_end)
			{
				if (IsTimedout())
				{
					DEBUG_MESSAGE("Engine, %s, has spent too long in state, %d", _name, (int)_state);
					return;
				}
				
				smp::Yield();
			}
			CRAG_VERIFY_EQUAL_ENUM(_state, State::acknowledge_flush_end);
			
			ASSERT(_messages.IsEmpty());
		}
		
		// generates a deferred, thread-safe call to the engine
		template <typename FUNCTION_TYPE>
		static void Call(FUNCTION_TYPE const & function)
		{
			ASSERT(singleton->_state < State::acknowledge_flush_end);

			// otherwise, wrap up the function and send it over.
			singleton->PushMessage(function);
		}

		// pass parameters to all Listeners of matching parameters
		template <typename EVENT>
		static void Broadcast(EVENT event)
		{
			ASSERT(singleton->_state < State::acknowledge_flush_end);

			typedef ListenerInterface<EVENT> ListenerInterface;
			ListenerInterface::Broadcast(event);
		}

	private:
		void Run()
		{
			FUNCTION_NO_REENTRY;

			ASSERT(this == singleton);
			ASSERT(_thread.IsCurrent());

			CRAG_VERIFY_EQUAL_ENUM(_state, State::initialized);
			SetState(State::running);

			{
				// create engine
				Engine engine;
				_engine = & engine;

				engine.Run(_messages);
				DEBUG_MESSAGE("Engine, %s, returned", _name);

				// Acknowledge that this won't be sending any more.
				SetState(State::acknowledge_flush_begin);
				
				// send an SDL event in case main even is asleep in SDL_WaitEvent
				ASSERT(! IsRunning());
				app::Quit();
				
				while (_state < State::request_flush_end)
				{
					if (_state != State::acknowledge_flush_begin)
					{
						CRAG_VERIFY_EQUAL_ENUM(_state, State::request_flush_end);
					}

					if (IsTimedout())
					{
						DEBUG_MESSAGE("Engine, %s, has spent too long in state, %d", _name, (int)_state);
						break;
					}

					FlushMessagesOrYield();
				}

				while (! _messages.IsEmpty() || ! ListenerBase::CanExit())
				{
					if (IsTimedout())
					{
						DEBUG_MESSAGE("Engine, %s, has spent too long in state, %d", _name, (int)_state);
						break;
					}

					FlushMessagesOrYield();
				}

				// destroy engine
				CRAG_VERIFY(engine);
				_engine = nullptr;
			}

			SetState(State::acknowledge_flush_end);
		}
		
		template <typename MESSAGE>
		void PushMessage(MESSAGE const & message)
		{
			if (_messages.PushBack(message))
			{
				DEBUG_MESSAGE("Engine, %s, received a deluge of messages on this thread", _name);
			}
		}
		
		bool FlushMessages()
		{
			if (! _messages.DispatchMessages(* _engine))
			{
				return false;
			}
			
			return true;
		}
		
		// Polite call to FlushMessages 
		// for when thread has nothing better to do
		void FlushMessagesOrYield()
		{
			if (! FlushMessages())
			{
				smp::Yield();
			}
		}
		
		void SetState(State state)
		{
			ASSERT(state > _state);
			_state = state;
			_state_change_time = app::GetTime();
		}
		
		// true iff it's been too long since the last state change
		bool IsTimedout() const
		{
			if (_state == State::running)
			{
				return false;
			}
			
			auto time = app::GetTime();
			auto duration = time - _state_change_time;
			if (duration < ShutdownTimeout())
			{
				return false;
			}
			
			return true;
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Engine * _engine;
		MessageQueue _messages;
		smp::Thread _thread;
		State _state;
		core::Time _state_change_time;
#if defined(CRAG_DEBUG)
		char const * _name;
#endif
		
		static Daemon * singleton;
	};
	
	// the singleton
	template <typename ENGINE>
	Daemon<ENGINE> * Daemon<ENGINE>::singleton = nullptr;
}
