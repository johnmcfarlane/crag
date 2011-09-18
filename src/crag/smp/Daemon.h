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
		typedef MessageEnvelope<Class> MessageEnvelope;
		typedef SimpleMutex Mutex;
		typedef Lock<Mutex> Lock;
	public:
		typedef MessageQueue<Class, MessageEnvelope> MessageQueue;
		
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Daemon(size_t ring_buffer_size)
		: _object(nullptr)
		, _envelopes(ring_buffer_size)
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
			
			Assert(_envelopes.IsEmpty());
			
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
			return _envelopes.capacity();
		}
		
		void Start()
		{
			Assert(! singleton->_thread.IsCurrent());

			Thread::Launch<Daemon, & Daemon::_thread, & Daemon::Run>(* this);
			
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
			
			smp::TerminateMessage message;
			SendMessage(message);
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
			
			Assert(_envelopes.IsEmpty());
		}
		
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message)
		{
			singleton->PushMessage(message);
		}
		
	private:
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
			
			object.Run(_envelopes);
		
			// Acknowledge that this won't be sending any more.
			_state = acknowledge_flush_begin;
			while (_state == acknowledge_flush_begin)
			{
				FlushMessages();
			}
			
			Assert(_state == request_flush_end);
			while (! _envelopes.IsEmpty())
			{
				FlushMessages();
			}
			
			_state = acknowledge_flush_end;
		
			// destroy object
			_object = nullptr;
		}
		
		template <typename MESSAGE>
		void PushMessage(MESSAGE const & message)
		{
			SpecializedMessageEnvelope<CLASS, MESSAGE> envelope (message);
			
			_envelopes.PushBack(envelope);
		}
		
		bool FlushMessages()
		{
			if (! _envelopes.DispatchMessages(* _object))
			{
				return false;
			}
			
			return true;
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Class * _object;
		MessageQueue _envelopes;
		Thread _thread;
		State _state;
		
		static Daemon * singleton;
	};
	
	// the singleton
	template <typename CLASS>
	Daemon<CLASS> * Daemon<CLASS>::singleton = nullptr;
}
