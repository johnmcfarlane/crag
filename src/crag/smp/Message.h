//
//  Message.h
//  crag
//
//  Created by John McFarlane on 5/5/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/intrusive_list.h"
#include "Semaphore.h"


namespace smp
{
	
	// Tells the Actor to terminate execution.
	// Can be overridden in CLASS but don't forget to call through!
	struct TerminateMessage
	{
	};
	
	
	template <class MESSAGE> bool IsTerminateMessage();
	
	template <>
	inline bool IsTerminateMessage<TerminateMessage>() 
	{
		return true;
	}
	
	template <class MESSAGE> 
	inline bool IsTerminateMessage() 
	{
		return false;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// MessageEnvelope classes
	//
	// Store copies of messages in an Actor's message queue.
	
	// forward-declaration
	template <typename CLASS> class Actor;
	
	
	// Polymorphic base class for message wrapper.
	// Required so that messages can be listed in Actor.
	template <typename CLASS>
	class MessageEnvelope
	{
	public:
		virtual ~MessageEnvelope() 
		{ 
		}
		
		// returns false if the Actor should quit
		virtual bool Execute() = 0;
		
		virtual void WaitForReply() = 0;
		
	private:
		core::intrusive::hook h;
		
		// friends
		friend class Actor<CLASS>;
	};
	
	
	// Message-type-specific message wrapper class.
	// Does all the work.
	template <typename CLASS, typename MESSAGE>
	class NonBlockingMessageEnvelope : public MessageEnvelope<CLASS>
	{
	public:
		NonBlockingMessageEnvelope(CLASS & destination, MESSAGE const & message)
		: _message(message) 
		, _destination(destination)
		{ 
		}
		
		// returns true if the Actor should continue (false=terminate)
		virtual bool Execute()
		{
			_destination.OnMessage(_message);
			return ! IsTerminateMessage<MESSAGE>();
		}
		
		virtual void WaitForReply()
		{
			// No need to wait for reply.
		}
		
	private:
		MESSAGE _message;
		CLASS & _destination;
	};
	
	
	// As NonBlockingMessageEnvelope but blocks until the message has been processed.
	// Try and avoid this kind of message.
	template <typename CLASS, typename MESSAGE, typename REPLY>
	class BlockingMessageEnvelope : public MessageEnvelope<CLASS>
	{
	public:
		BlockingMessageEnvelope(CLASS & destination, MESSAGE const & message, REPLY & reply)
		: _destination(destination)
		, _message(message)
		, _reply(reply)
		, _semaphore(0)
		{
		}
		
		void WaitForReply()
		{
			//_condition.Wait(_mutex);
			_semaphore.Decrement();
		}
		
		// returns true if the Actor should continue (false=terminate)
		bool Execute()
		{
			_destination.OnMessage(_message, _reply);
			//_condition.Restart();
			_semaphore.Increment();
			return ! IsTerminateMessage<MESSAGE>();
		}
		
	private:
		MESSAGE _message;
		CLASS & _destination;
		REPLY & _reply;
		//ThreadCondition _condition;
		//Mutex _mutex;
		Semaphore _semaphore;
	};
	
}
