//
//  MessageQueue_Impl.h
//  crag
//
//  Created by John McFarlane on 2012-04-04.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "MessageQueue.h"

////////////////////////////////////////////////////////////////////////////////
// MessageQueue member definitions

// base class for Envelope
template <typename CLASS>
class ipc::MessageQueue<CLASS>::EnvelopeBase
{
public:
	virtual ~EnvelopeBase() 
	{ 
	}

	virtual void operator()(MessageQueue & queue, CLASS & object) const = 0;
};

// containment for a message; contained, in turn, in MessageQueue's ring buffer
template <typename CLASS>
template <typename MESSAGE>
class ipc::MessageQueue<CLASS>::Envelope : public ipc::MessageQueue<CLASS>::EnvelopeBase
{
	MESSAGE _message;
public:
	Envelope(MESSAGE message)
		: _message(message)
	{
	}

	void operator() (MessageQueue & queue, CLASS & object) const
	{
		queue.CompleteDispatch<MESSAGE>(_message, object);
		// WARNING: 'this' is invalid
	}
};

template <typename CLASS>
ipc::MessageQueue<CLASS>::MessageQueue(size_type capacity)
: _buffer(capacity)
{
}

template <typename CLASS>
bool ipc::MessageQueue<CLASS>::IsEmpty() const
{
	return _buffer.empty();
}
		
template <typename CLASS>
void ipc::MessageQueue<CLASS>::Clear()
{
	Lock critical_section(_mutex);
	_buffer.clear();
}

template <typename CLASS>
template <typename MESSAGE>
void ipc::MessageQueue<CLASS>::PushBack(MESSAGE const & object)
{
	Lock critical_section(_mutex);
			
	if (! _buffer.push_back(Envelope<MESSAGE>(object)))
	{
		// may cause a lock-up, or buffer may clear
		assert(false);
		smp::Yield();
	}
}

template <typename CLASS>
bool ipc::MessageQueue<CLASS>::DispatchMessage(Class & object)
{
	// Slightly risky, but I think we can avoid a lock here.
	if (_buffer.empty())
	{
		return false;
	}
	
	_mutex.lock();
			
	EnvelopeBase const & message_wrapper = _buffer.front();
	message_wrapper(* this, object);

	return true;
}

template <typename CLASS>
int ipc::MessageQueue<CLASS>::DispatchMessages(Class & object)
{
	int num_messages = 0;
			
	while (DispatchMessage(object))
	{
		++ num_messages;
	}
			
	return num_messages;
}
		
template <typename CLASS>
template <typename MESSAGE>
void ipc::MessageQueue<CLASS>::CompleteDispatch(MESSAGE message, Class & object)
{
	// by copying EnvelopeBase::_message into message, the data is 'delivered';

	// it is now possible (although risky) to pop front and unlock this
	_buffer.pop_front();
	_mutex.unlock();
	// WARNING: However, the popped envelope is the 'this' parameter below here in the stack. 
	// So an awkward silence must be maintained while returning to DispatchMessage.

	message(object);
}
