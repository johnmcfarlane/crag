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
		
		// 0-parameter version
		static void Call (void (Engine::* function)())
		{
			Command0 command(function);
			SendMessage(command);
		}
		
		// 1-parameter version
		template <typename PARAMETER1>
		static void Call (void (Engine::* function)(PARAMETER1 const &), PARAMETER1 const & parameter1)
		{
			Command1<PARAMETER1> command(function, parameter1);
			SendMessage(command);
		}
		
		// 2-parameter version
		template <typename PARAMETER1, typename PARAMETER2>
		static void Call (void (Engine::* function)(PARAMETER1 const &, PARAMETER2 const &), PARAMETER1 const & parameter1, PARAMETER2 const & parameter2)
		{
			Command2<PARAMETER1, PARAMETER2> command(function, parameter1, parameter2);
			SendMessage(command);
		}
		
		// 3-parameter version
		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		static void Call (void (Engine::* function)(PARAMETER1 const &, PARAMETER2 const &, PARAMETER3 const &), PARAMETER1 const & parameter1, PARAMETER2 const & parameter2, PARAMETER3 const & parameter3)
		{
			Command3<PARAMETER1, PARAMETER2, PARAMETER3> command(function, parameter1, parameter2, parameter3);
			SendMessage(command);
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Poll - generates a deferred function call to the thread-safe engine
		
		template <typename RETURN_TYPE>
		static void Poll(RETURN_TYPE (ENGINE::* function)() const, bool & complete, RETURN_TYPE & result)
		{
			// functor is sent to ENGINE's thread to call function and retrieve result
			PollCommand0<RETURN_TYPE, RETURN_TYPE> command(function, complete, result);
			ENGINE::Daemon::SendMessage(command);
		}
		
		template <typename RETURN_TYPE>
		static void Poll(RETURN_TYPE const & (ENGINE::* function)() const, bool & complete, RETURN_TYPE & result)
		{
			// functor is sent to ENGINE's thread to call function and retrieve result
			PollCommand0<RETURN_TYPE, RETURN_TYPE const &> command(function, complete, result);
			ENGINE::Daemon::SendMessage(command);
		}
		
	private:
		// 0-parameter Call helper
		class Command0 : public Message
		{
		public:
			typedef void (Engine::* FunctionType)();
			Command0(FunctionType function)
			: _function(function)
			{
			}
		private:
			void operator () (Engine & engine) const
			{
				(engine.*_function)();
			}
			FunctionType _function;
		};
		
		// 1-parameter Call helper
		template <typename PARAMETER1>
		class Command1 : public Message
		{
		public:
			typedef void (Engine::* FunctionType)(PARAMETER1 const &);
			Command1(FunctionType function, PARAMETER1 const & __parameter1) 
			: _function(function)
			, _parameter1(__parameter1) { 
			}
		private:
			void operator () (Engine & engine) const {
				(engine.*_function)(_parameter1);
			}
			FunctionType _function;
			PARAMETER1 _parameter1;
		};
		
		// 2-parameter Call helper
		template <typename PARAMETER1, typename PARAMETER2>
		class Command2 : public Message
		{
		public:
			typedef void (Engine::* FunctionType)(PARAMETER1 const &, PARAMETER2 const &);
			Command2(FunctionType function, PARAMETER1 const & __parameter1, PARAMETER2 const & __parameter2) 
			: _function(function)
			, _parameter1(__parameter1)
			, _parameter2(__parameter2) { 
			}
		private:
			void operator () (Engine & engine) const {
				(engine.*_function)(_parameter1, _parameter2);
			}
			FunctionType _function;
			PARAMETER1 _parameter1;
			PARAMETER2 _parameter2;
		};
		
		// 3-parameter Call helper
		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		class Command3 : public Message
		{
		public:
			typedef void (Engine::* FunctionType)(PARAMETER1 const &, PARAMETER2 const &, PARAMETER3 const &);
			Command3(FunctionType function, PARAMETER1 const & __parameter1, PARAMETER2 const & __parameter2, PARAMETER3 const & __parameter3) 
			: _function(function)
			, _parameter1(__parameter1)
			, _parameter2(__parameter2)
			, _parameter3(__parameter3) { 
			}
		private:
			void operator () (Engine & engine) const {
				(engine.*_function)(_parameter1, _parameter2, _parameter3);
			}
			FunctionType _function;
			PARAMETER1 _parameter1;
			PARAMETER2 _parameter2;
			PARAMETER3 _parameter3;
		};
		
		// calls an ENGINE function which returns a RETURN_TYPE and
		// assigns it (via a reference) to an object ot type, RESULT
		template <typename RESULT, typename RETURN>
		class PollCommand0 : public Message
		{
			typedef RETURN (ENGINE::* FUNCTION)() const;
		public:
			// functions
			PollCommand0(FUNCTION function, bool & complete, RESULT & result)
			: _function(function)
			, _complete(complete)
			, _result(result)
			{
			}
		private:
			virtual void operator() (ENGINE & engine) const final
			{
				ASSERT(_complete == false);
				_result = (engine.*_function)();
				AtomicCompilerBarrier();
				_complete = true;
			}
			FUNCTION _function;
			bool & _complete;
			RESULT & _result;
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
