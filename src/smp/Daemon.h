//
//  Daemon.h
//  crag
//
//  Created by John McFarlane on 4/26/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Lock.h"
#include "Message.h"
#include "MessageQueue.h"
#include "Thread.h"

#include "core/ring_buffer.h"


namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// thread-safe wrapper for a service/sub-system 
	
	template <typename CLASS>
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

		typedef CLASS Class;
		typedef SimpleMutex Mutex;
		typedef Lock<Mutex> Lock;
	public:
		typedef Message<Class> Message;
		typedef MessageQueue<Class, Message> MessageQueue;
		
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Daemon(size_t ring_buffer_size)
		: _object(nullptr)
		, _messages(ring_buffer_size)
		, _state(initialized)
		{
			Assert(singleton == nullptr);
			singleton = this;
		}
		
		~Daemon() 
		{ 
			Assert(_state == acknowledge_flush_end);
			_state = request_destroy;
			
			Assert(singleton == this);
			singleton = nullptr;
			
			_thread.Join();

			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			// Must call Stop from the outside thread first.
			Assert(! _thread.IsLaunched());
			
			Assert(_messages.IsEmpty());
			
			Assert(_object == nullptr);
		}
		
		// Usually represents failure to completely encapsulate the object.
		static Class & Ref()
		{
			return ref(singleton->_object);
		}
		
		// specifically, the thread is still in the object's Run function
		bool IsRunning() const
		{
			return _state <= running;
		}
		
		size_t GetQueueCapacity() const
		{
			return _messages.capacity();
		}
		
		void Start(char const * name)
		{
			Assert(! singleton->_thread.IsCurrent());

			Thread::Launch<Daemon, & Daemon::_thread, & Daemon::Run>(* this, name);
			
			while (_object == nullptr)
			{
				// block until the the object is constructed and assigned
				Yield();
			}
		}
		
		// Tell the daemon to wind down.
		void BeginFlush()
		{
			Assert(! singleton->_thread.IsCurrent());
			Assert(_state <= acknowledge_flush_begin);
			
			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			Call(& Class::OnQuit);
		}
		
		// Wait until it has stopped working.
		void Synchronize()
		{
			Assert(! singleton->_thread.IsCurrent());
			Assert(_state <= acknowledge_flush_begin);
			
			while (_state < acknowledge_flush_begin)
			{
				Yield();
			}
		}
		
		// Wait until it has finished flushing.
		void EndFlush()
		{
			Assert(! singleton->_thread.IsCurrent());			
			Assert(_state == acknowledge_flush_begin);
			
			_state = request_flush_end;
			while (_state < acknowledge_flush_end)
			{
				Yield();
			}
			Assert(acknowledge_flush_end);
			
			Assert(_messages.IsEmpty());
		}
		
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message)
		{
			singleton->PushMessage(message);
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// Call - generates a deferred function call to the thread-safe object
		
		// 0-parameter version
		static void Call (void (Class::* function)())
		{
			DerivedMessage0 message(function);
			SendMessage(message);
		}
		
		// 1-parameter version
		template <typename PARAMETER1>
		static void Call (PARAMETER1 const & parameter1, void (Class::* function)(PARAMETER1 const &))
		{
			DerivedMessage1<PARAMETER1> message(function, parameter1);
			SendMessage(message);
		}
		
		// 2-parameter version
		template <typename PARAMETER1, typename PARAMETER2>
		static void Call (PARAMETER1 const & parameter1, PARAMETER2 const & parameter2, void (Class::* function)(PARAMETER1 const &, PARAMETER2 const &))
		{
			DerivedMessage2<PARAMETER1, PARAMETER2> message(function, parameter1, parameter2);
			SendMessage(message);
		}
		
		// 3-parameter version
		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		static void Call (PARAMETER1 const & parameter1, PARAMETER2 const & parameter2, PARAMETER3 const & parameter3, void (Class::* function)(PARAMETER1 const &, PARAMETER2 const &, PARAMETER3 const &))
		{
			DerivedMessage3<PARAMETER1, PARAMETER2, PARAMETER3> message(function, parameter1, parameter2, parameter3);
			SendMessage(message);
		}
		
	private:
		// 0-parameter Call helper
		class DerivedMessage0 : public Message
		{
		public:
			typedef void (Class::* FunctionType)();
			DerivedMessage0(FunctionType function)
			: _function(function)
			{
			}
		private:
			void operator () (Class & object) const
			{
				(object.*_function)();
			}
			FunctionType _function;
		};
		
		// 1-parameter Call helper
		template <typename PARAMETER1>
		class DerivedMessage1 : public Message
		{
		public:
			typedef void (Class::* FunctionType)(PARAMETER1 const &);
			DerivedMessage1(FunctionType function, PARAMETER1 const & __parameter1) 
			: _function(function)
			, _parameter1(__parameter1) { 
			}
		private:
			void operator () (Class & object) const {
				(object.*_function)(_parameter1);
			}
			FunctionType _function;
			PARAMETER1 _parameter1;
		};
		
		// 2-parameter Call helper
		template <typename PARAMETER1, typename PARAMETER2>
		class DerivedMessage2 : public Message
		{
		public:
			typedef void (Class::* FunctionType)(PARAMETER1 const &, PARAMETER2 const &);
			DerivedMessage2(FunctionType function, PARAMETER1 const & __parameter1, PARAMETER2 const & __parameter2) 
			: _function(function)
			, _parameter1(__parameter1)
			, _parameter2(__parameter2) { 
			}
		private:
			void operator () (Class & object) const {
				(object.*_function)(_parameter1, _parameter2);
			}
			FunctionType _function;
			PARAMETER1 _parameter1;
			PARAMETER2 _parameter2;
		};
		
		// 3-parameter Call helper
		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		class DerivedMessage3 : public Message
		{
		public:
			typedef void (Class::* FunctionType)(PARAMETER1 const &, PARAMETER2 const &, PARAMETER3 const &);
			DerivedMessage3(FunctionType function, PARAMETER1 const & __parameter1, PARAMETER2 const & __parameter2, PARAMETER3 const & __parameter3) 
			: _function(function)
			, _parameter1(__parameter1)
			, _parameter2(__parameter2)
			, _parameter3(__parameter3) { 
			}
		private:
			void operator () (Class & object) const {
				(object.*_function)(_parameter1, _parameter2, _parameter3);
			}
			FunctionType _function;
			PARAMETER1 _parameter1;
			PARAMETER2 _parameter2;
			PARAMETER3 _parameter3;
		};
		
		void Run()
		{
			FUNCTION_NO_REENTRY;
			
			Assert(this == singleton);
			Assert(_thread.IsCurrent());
			
			Assert(_state == initialized);
			_state = running;
			
			// create object
			Class object;
			_object = & object;
			
			object.Run(_messages);
		
			// Acknowledge that this won't be sending any more.
			_state = acknowledge_flush_begin;
			while (_state == acknowledge_flush_begin)
			{
				FlushMessagesOrYield();
			}
			
			Assert(_state == request_flush_end);
			while (! _messages.IsEmpty())
			{
				FlushMessagesOrYield();
			}
			
			_state = acknowledge_flush_end;
		
			// destroy object
			_object = nullptr;
		}
		
		template <typename MESSAGE>
		void PushMessage(MESSAGE const & message)
		{
			_messages.PushBack(message);
		}
		
		bool FlushMessages()
		{
			if (! _messages.DispatchMessages(* _object))
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
		
		Class * _object;
		MessageQueue _messages;
		Thread _thread;
		State _state;
		
		static Daemon * singleton;
	};
	
	// the singleton
	template <typename CLASS>
	Daemon<CLASS> * Daemon<CLASS>::singleton = nullptr;
}
