//
//  Daemon.h
//  crag
//
//  Created by John McFarlane on 4/26/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "atomic.h"
#include "Lock.h"
#include "Message.h"
#include "MessageQueue.h"
#include "Thread.h"

#include "core/ring_buffer.h"


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
		typedef Lock<Mutex> Lock;
	public:
		typedef ENGINE Engine;
		typedef Message<Engine> Message;
		typedef MessageQueue<Engine, Message> MessageQueue;
		
		
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
		
		// Usually represents failure to completely encapsulate the engine.
		static Engine & Ref()
		{
			return ref(singleton->_engine);
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

			Thread::Launch<Daemon, & Daemon::_thread, & Daemon::Run>(* this, name);
			
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
			
			Call(& Engine::OnQuit);
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
			ASSERT(acknowledge_flush_end);
			
			ASSERT(_messages.IsEmpty());
		}
		
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message)
		{
			singleton->PushMessage(message);
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Call - generates a deferred function call to the thread-safe engine
		
		template <typename... PARAMETERS>
		static void Call(void (Engine::* function)(PARAMETERS const & ...), PARAMETERS const &... parameters)
		{
			CallCommand<PARAMETERS...> command(function, parameters...);
			SendMessage(command);
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Poll - generates a deferred function call to the thread-safe engine
		
		template <typename VALUE_TYPE, typename FUNCTION_TYPE, typename... PARAMETERS>
		static void Poll(VALUE_TYPE & result, bool & complete, FUNCTION_TYPE function, PARAMETERS const &... parameters)
		{
			// functor is sent to Engine's thread to call function and retrieve result
			PollCommand<VALUE_TYPE, FUNCTION_TYPE, PARAMETERS...> command(result, complete, function, parameters...);
			SendMessage(command);
		}
		
	private:
		template <typename... PARAMETERS>
		class CallCommand : public Message
		{
			// types
			typedef void (Engine::* FunctionType)(PARAMETERS const & ...);
			typedef std::tr1::tuple<PARAMETERS...> Tuple;
		public:
			// functions
			CallCommand(FunctionType function, PARAMETERS const & ... parameters) 
			: _function(function)
			, _parameters(parameters...)
			{ 
			}
		private:
			virtual void operator () (Engine & engine) const final
			{
				core::call(engine, _function, _parameters);
			}
			// variables
			FunctionType _function;
			Tuple _parameters;
		};
		
		template <typename VALUE_TYPE, typename FUNCTION_TYPE, typename... PARAMETERS>
		class PollCommand : public Message
		{
		public:
			// functions
			PollCommand(VALUE_TYPE & result, bool & complete, FUNCTION_TYPE function, PARAMETERS const & ... parameters) 
			: _result(result)
			, _complete(complete)
			, _function(function)
			, _parameters(parameters...)
			{ 
			}
		private:
			virtual void operator () (Engine & engine) const final
			{
				ASSERT(_complete == false);
				std::tr1::tuple<> t;
				_result = core::call(engine, _function, _parameters);
				AtomicCompilerBarrier();
				_complete = true;
			}
			// variables
			VALUE_TYPE & _result;
			bool & _complete;
			FUNCTION_TYPE _function;
			std::tr1::tuple<PARAMETERS...> _parameters;
		};
		
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
