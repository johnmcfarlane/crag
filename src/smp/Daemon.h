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
#include "Thread.h"

#include "core/ring_buffer.h"

#include "smp/ListenerInterface.h"

namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// thread-safe wrapper for a service/sub-system (engine)
	
	template <typename ENGINE>
	class Daemon
	{
		OBJECT_NO_COPY(Daemon);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		enum State
		{
			initialized,
			running,
			acknowledge_flush_begin,
			request_flush_end,
			acknowledge_flush_end,
			request_destroy
		};

		typedef SimpleMutex Mutex;
		typedef std::lock_guard<Mutex> Lock;
	public:
		typedef ENGINE Engine;
		typedef ::smp::MessageQueue<Engine> MessageQueue;
		
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Daemon(size_t ring_buffer_size)
		: _engine(nullptr)
		, _messages(ring_buffer_size)
		, _state(initialized)
		{
			ASSERT(singleton == nullptr);
			singleton = this;
		}
		
		~Daemon() 
		{ 
			ASSERT(_state == acknowledge_flush_end);
			_state = request_destroy;
			
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
			return singleton->_state <= running;
		}
		
		// true iff called from the Daemon's own thread
		static bool IsCurrentThread()
		{
			return singleton->_thread.IsCurrent();
		}
		
		size_t GetQueueCapacity() const
		{
			return _messages.capacity();
		}
		
		void Start(char const * name)
		{
			ASSERT(! singleton->_thread.IsCurrent());

			_thread.Launch([this, name] () {
				SetThreadName(name);
				this->Run();
			});
			
			while (_engine == nullptr)
			{
				// block until the the engine is constructed and assigned
				Yield();
			}
		}
		
		// Tell the daemon to wind down.
		void BeginFlush()
		{
			ASSERT(! singleton->_thread.IsCurrent());
			ASSERT(_state <= acknowledge_flush_begin);
			
			// Never called from within the thread.
			ASSERT(! _thread.IsCurrent());
			
			Call([] (Engine & engine) { engine.OnQuit(); });
		}
		
		// Wait until it has stopped working.
		void Synchronize()
		{
			ASSERT(! singleton->_thread.IsCurrent());
			ASSERT(_state <= acknowledge_flush_begin);
			
			while (_state < acknowledge_flush_begin)
			{
				Yield();
			}
		}
		
		// Wait until it has finished flushing.
		void EndFlush()
		{
			ASSERT(! singleton->_thread.IsCurrent());			
			ASSERT(_state == acknowledge_flush_begin);
			
			_state = request_flush_end;
			while (_state < acknowledge_flush_end)
			{
				Yield();
			}
			ASSERT(_state == acknowledge_flush_end);
			
			ASSERT(_messages.IsEmpty());
		}
		
		// generates a deferred, thread-safe call to the engine
		template <typename FUNCTION_TYPE>
		static void Call(FUNCTION_TYPE const & function)
		{
			ASSERT(singleton->_state < acknowledge_flush_end);

			// If caller is on the same thread as the engine,
			if (singleton->_thread.IsCurrent())
			{
				// make the call directly
				function(ref(singleton->_engine));
			}
			else
			{
				// otherwise, wrap up the function and send it over.
				singleton->PushMessage(function);
			}
		}

		// pass parameters to any Listener of matching parameters
		template <typename ... PARAMETERS>
		static void Broadcast(PARAMETERS ... parameters)
		{
			ASSERT(singleton->_state < acknowledge_flush_begin);

			typedef ListenerInterface<Engine, PARAMETERS ...> ListenerInterface;
			ListenerInterface::Broadcast(parameters ...);
		}

	private:
		void Run()
		{
			FUNCTION_NO_REENTRY;
			
			ASSERT(this == singleton);
			ASSERT(_thread.IsCurrent());
			
			ASSERT(_state == initialized);
			_state = running;
			
			// create engine
			Engine engine;
			_engine = & engine;
			
			engine.Run(_messages);
		
			// Acknowledge that this won't be sending any more.
			_state = acknowledge_flush_begin;
			while (_state == acknowledge_flush_begin)
			{
				FlushMessagesOrYield();
			}
			
			ASSERT(_state == request_flush_end);
			while (! _messages.IsEmpty())
			{
				FlushMessagesOrYield();
			}
			
			_state = acknowledge_flush_end;
		
			// destroy engine
			_engine = nullptr;
		}
		
		template <typename MESSAGE>
		void PushMessage(MESSAGE const & message)
		{
			_messages.PushBack(message);
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
				Yield();
			}
		}

		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Engine * _engine;
		MessageQueue _messages;
		Thread _thread;
		State _state;
		
		static Daemon * singleton;
	};
	
	// the singleton
	template <typename ENGINE>
	Daemon<ENGINE> * Daemon<ENGINE>::singleton = nullptr;
}
