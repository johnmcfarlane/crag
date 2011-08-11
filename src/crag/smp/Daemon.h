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
	
	template <typename CLASS, bool THREADED>
	class Daemon
	{
		OBJECT_NO_COPY(Daemon);
		
		////////////////////////////////////////////////////////////////////////////////
		// types

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
		{
			Assert(singleton == nullptr);
			singleton = this;
		}
		
		~Daemon() 
		{ 
			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			// Must call Stop from the outside thread first.
			Assert(! _thread.IsLaunched());
			
			//Assert(_envelopes.empty());
			_envelopes.Clear();
			
			Assert(_object == nullptr);
			
			Assert(singleton == this);
			singleton = nullptr;
		}
		
		// TODO: Always represents failure to completely encapsulate the object.
		static Class & Ref()
		{
			return ref(singleton->_object);
		}
		
		size_t GetQueueCapacity() const
		{
			return _envelopes.capacity();
		}
		
		void Start()
		{
			if (! THREADED)
			{
				Run();
			}
			
			Thread::Launch<Daemon, & Daemon::_thread, & Daemon::Run>(* this);
		}
		
		// Called from a thread that will block until the actor terminates.
		void Stop()
		{
			if (! THREADED)
			{
				return;
			}
			
			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			smp::TerminateMessage message;
			SendMessage(message);
			_thread.Join();
		}
		
		void Run()
		{
			FUNCTION_NO_REENTRY;
			Assert(! THREADED || singleton->_thread.IsCurrent());
			
			_object = new Class;
			_object->Run(_envelopes);
			delete _object;
			_object = nullptr;
		}

		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message)
		{
			singleton->PushMessage(message);
		}
		
	private:
		template <typename MESSAGE>
		void PushMessage(MESSAGE const & message)
		{
			SpecializedMessageEnvelope<CLASS, MESSAGE> envelope (message);
			
			_envelopes.PushBack(envelope);
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Class * _object;
		MessageQueue _envelopes;
		Thread _thread;
		
		static Daemon * singleton;
	};
	
	// the singleton
	template <typename CLASS, bool THREADED>
	Daemon<CLASS, THREADED> * Daemon<CLASS, THREADED>::singleton = nullptr;
}
