//
//  Actor.h
//  crag
//
//  Created by John McFarlane on 4/26/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Message.h"
#include "Lock.h"
#include "SimpleMutex.h"
#include "Thread.h"


namespace smp
{
	
	// Super-simplistic interpretation of the Actor model.
	// Each actor has its own thread and they address one another by memory address.
	// class, CLASS, must derive from Actor<CLASS>
	template <typename CLASS>
	class Actor
	{
		OBJECT_NO_COPY(Actor);
		
		// types
		typedef MessageEnvelope<CLASS> _MessageEnvelope;
		typedef core::intrusive::list<_MessageEnvelope, & _MessageEnvelope::h> _list;
		typedef Thread<Actor> Thread;
		typedef SimpleMutex Mutex;
		typedef Lock<Mutex> Lock;
		
	public:
		Actor()
		{
		}
		
		virtual ~Actor() 
		{ 
			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			// Must call Stop from the outside thread first.
			Assert(! _thread.IsLaunched());
			
			//Assert(_envelopes.empty());
			FlushMessages();
		}
		
		void Start()
		{
			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			// type of the smp::Thread templated member function we wish to call
			typedef void (Thread::*THREAD_FUNCTION)(Actor & object);
			
//			// pointer to the function with template parameter of Actor member function
			// OnLaunch is the function that will be called by the thread class in the new thread.
			THREAD_FUNCTION f = & Thread::template Launch<& Actor::Run, & Actor::_thread>;
			
			// This line is too simple to deserve a random stream of programming terms like the others.
			(_thread.*f)(* this);
		}
		
		// Called from a thread that will block until the actor terminates.
		void Stop()
		{
			// Never called from within the thread.
			Assert(! _thread.IsCurrent());
			
			smp::TerminateMessage message;
			SendMessage(* static_cast<CLASS *>(this), message);
			_thread.Join();
		}
		
		template <typename MESSAGE>
		static void SendMessage(CLASS & destination, MESSAGE const & message)
		{
			destination.PushMessage(destination, message);
		}
		
	protected:
		// returns false iff the Actor should quit
		bool ProcessMessage()
		{
			_MessageEnvelope * m = PopMessage();
			
			if (m == nullptr)
			{
				return false;
			}

			m->Execute();
			delete m;
			
			return true;
		}
		
		// returns number of messages process
		int ProcessMessages()
		{
			int num_messages = 0;
			
			while (ProcessMessage())
			{
				++ num_messages;
			}
			
			return num_messages;
		}
		
		// empty the message queue without executing the messages
		void FlushMessages()
		{
			while (true)
			{
				_MessageEnvelope * m = PopMessage();
				
				if (m == nullptr)
				{
					break;
				}
				
				delete m;
			}
		}
		
	private:
		template <typename MESSAGE>
		void PushMessage(CLASS & destination, MESSAGE const & message)
		{
			Assert(& destination == this);
			
			// TODO: aim towards zero-allocation model.
			_MessageEnvelope * envelope;
			
			envelope = new SpecializedMessageEnvelope<CLASS, MESSAGE>(destination, message);
			
			Push(* envelope);
		}
		
		_MessageEnvelope * PopMessage()
		{
			// Reading _envelopes outside the lock is potentially risky.
			// However, the result needs to be double-checked next so bad results
			// either way are far from catastrophic. 
			if (_envelopes.empty())
			{
				return nullptr;
			}
			
			_MessageEnvelope * envelope = Pop();
			
			return envelope;
		}
		
		void Push(_MessageEnvelope & envelope)
		{
			Lock critical_section(_mutex);
			
			_envelopes.push_back(envelope);
		}
		
		_MessageEnvelope * Pop()
		{
			Lock critical_section(_mutex);
			
			if (! _envelopes.empty())
			{			
				_MessageEnvelope & envelope = _envelopes.front();
				_envelopes.pop_front();
				return & envelope;
			}
			else
			{
				return nullptr;
			}
		}
		
		virtual void Run() = 0;
		
		// attributes
		_list _envelopes;
		Thread _thread;
		Mutex _mutex;
	};
	
}
