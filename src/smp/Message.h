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


namespace smp
{
	
	// Tells the Actor to terminate execution.
	struct TerminateMessage
	{
	};
	
	
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
		virtual void Dispatch(CLASS & destination) const = 0;
		
	private:
		// friends
		friend class Actor<CLASS>;
	};
	
	
	// Message-type-specific message wrapper class.
	template <typename CLASS, typename MESSAGE>
	class SpecializedMessageEnvelope : public MessageEnvelope<CLASS>
	{
		typedef MessageEnvelope<CLASS> super;
	public:
		SpecializedMessageEnvelope(MESSAGE const & message)
		: _message(message) 
		{ 
		}
		
		// returns true if the Actor should continue (false=terminate)
		virtual void Dispatch(CLASS & destination) const
		{
			destination.OnMessage(_message);
		}
		
	private:
		MESSAGE _message;
	};
	
}
