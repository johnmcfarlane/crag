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
		// types
		typedef MessageEnvelope<CLASS> _MessageEnvelope;
		typedef core::intrusive::list<_MessageEnvelope, & _MessageEnvelope::h> _list;
		typedef Thread<Actor> Thread;
		typedef SimpleMutex Mutex;
		typedef Lock<Mutex> Lock;
	public:
		virtual ~Actor() 
		{ 
			_thread.Join();
			Assert(_envelopes.empty());
		}
		
		void Launch()
		{
			// type of the smp::Thread templated member function we wish to call
			typedef void (Thread::*THREAD_FUNCTION)(Actor & object);
			
			// pointer to the function with template parameter of Actor member function
			// OnLaunch is the function that will be called by the thread class in the new thread.
			THREAD_FUNCTION f = & Thread::template Launch<& Actor::Run>;
			
			// This line is too simple to deserve a random stream of programming terms like the others.
			(_thread.*f)(* this);
		}
		
		// Called from a thread that will block until the actor terminates.
		void Join()
		{
			_thread.Join();
		}
		
		template <typename MESSAGE>
		static void SendMessage(CLASS & destination, MESSAGE const & message, bool blocking)
		{
			destination.PushMessage(destination, message, blocking);
		}
		
		void OnMessage(TerminateMessage const & message)
		{
		}
		
	protected:
		// returns false iff the Actor should quit
		bool ProcessMessages()
		{
			bool ok = true;
			
			while (true)
			{
				_MessageEnvelope * m = PopMessage();
				
				if (m == nullptr)
				{
					break;
				}
				
				ok &= m->Execute();
				delete m;
			}
			
			return ok;
		}
		
	private:
		template <typename MESSAGE>
		void PushMessage(CLASS & destination, MESSAGE const & message, bool blocking)
		{
			Assert(& destination == this);
			
			// TODO: aim towards zero-allocation model.
			_MessageEnvelope * envelope;
			
			if (blocking)
			{
				envelope = new BlockingMessageEnvelope<CLASS, MESSAGE>(destination, message);
			}
			else
			{
				envelope = new NonBlockingMessageEnvelope<CLASS, MESSAGE>(destination, message);
			}
			
			Push(* envelope);
			
			envelope->WaitForReply();
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
