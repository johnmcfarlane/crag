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
struct ipc::MessageQueue<CLASS>::BufferNode
{
	BufferNode(size_type capacity)
	: buffer(capacity)
	, next(nullptr)
	{
	}

	~BufferNode()
	{
		ASSERT(next == nullptr);
	}

	Buffer buffer;
	BufferNode * next;
};

template <typename CLASS>
ipc::MessageQueue<CLASS>::MessageQueue(size_type capacity)
: _buffers(new BufferNode(capacity))
{
}

template <typename CLASS>
ipc::MessageQueue<CLASS>::~MessageQueue()
{
	VerifyObject(* this);
	ASSERT(IsEmpty());

	auto * buffer = _buffers; 
	do
	{
		auto * next = _buffers->next;
		delete buffer;
		buffer = next;
	}
	while (buffer != nullptr);
}

#if defined(VERIFY)
template <typename CLASS>
void ipc::MessageQueue<CLASS>::Verify() const
{
	VerifyTrue(_buffers != nullptr);

	// If there's only one buffer,
	if (_buffers->next == nullptr)
	{
		// then not much more can be tested.
		_buffers->buffer.Verify();
		return;
	}

	// If there are multiple buffers,
	for (auto * buffer_node = _buffers; buffer_node != nullptr; buffer_node = buffer_node->next)
	{
		buffer_node->buffer.Verify();

		// then none of them should be empty.
		ASSERT(! buffer_node->buffer.empty());
	}
}
#endif

template <typename CLASS>
bool ipc::MessageQueue<CLASS>::IsEmpty() const
{
	return _buffers->buffer.empty();
}

template <typename CLASS>
template <typename MESSAGE>
void ipc::MessageQueue<CLASS>::PushBack(MESSAGE const & object)
{
	Lock critical_section(_mutex);
	VerifyObject(* this);
	
	auto * node = & GetPushBufferNode();

	while (true)
	{
		auto & buffer = node->buffer;

		// if the push_buffer isn't full,
		if (buffer.push_back(Envelope<MESSAGE>(object)))
		{
			// success!
			break;
		}
		ASSERT(! buffer.empty());

		typename Buffer::size_type required_capacity = sizeof(Envelope<MESSAGE>);
		typename Buffer::size_type existing_capacity = buffer.capacity();
		typename Buffer::size_type new_capacity = std::max(required_capacity, existing_capacity) << 2;
		DEBUG_MESSAGE("allocating larger ring buffer. new:" SIZE_T_FORMAT_SPEC "; msg:" SIZE_T_FORMAT_SPEC "; prev:" SIZE_T_FORMAT_SPEC, new_capacity, required_capacity, existing_capacity);

		auto & new_buffer = ref(new BufferNode(new_capacity));

		ASSERT(node->next == nullptr);
		node->next = & new_buffer;
		node = & new_buffer;

		smp::Yield();
	}

	VerifyObject(* this);
}

template <typename CLASS>
bool ipc::MessageQueue<CLASS>::DispatchMessage(Class & object)
{
	Lock critical_section(_mutex);
	VerifyObject(* this);
	
	auto & pull_buffer = GetPopBuffer();
	if (pull_buffer.empty())
	{
		return false;
	}
			
	EnvelopeBase const & message_wrapper = pull_buffer.front();
	message_wrapper(* this, object);

	VerifyObject(* this);
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
	PopFront();
	_mutex.unlock();
	// WARNING: However, the popped envelope is the 'this' parameter below here in the stack. 
	// So an awkward silence must be maintained while returning to DispatchMessage.

	message(object);
}

template <typename CLASS>
void ipc::MessageQueue<CLASS>::PopFront()
{
	auto & front = * _buffers;
	auto & pop_buffer = front.buffer;
	pop_buffer.pop_front();

	if (! pop_buffer.empty())
	{
		return;
	}

	auto next = front.next;
	if (next == nullptr)
	{
		return;
	}

	ASSERT(! next->buffer.empty());

	_buffers = next;
	delete & front;
}

template <typename CLASS>
typename ipc::MessageQueue<CLASS>::BufferNode & ipc::MessageQueue<CLASS>::GetPushBufferNode()
{
	auto * buffer_node = _buffers;

	while (true)
	{
		auto * next = buffer_node->next;
		if (next == nullptr)
		{
			return * buffer_node;
		}
		buffer_node = next;
	}
}

template <typename CLASS>
typename ipc::MessageQueue<CLASS>::BufferNode & ipc::MessageQueue<CLASS>::GetPopBufferNode()
{
	return * _buffers;
}

template <typename CLASS>
typename ipc::MessageQueue<CLASS>::Buffer & ipc::MessageQueue<CLASS>::GetPushBuffer()
{
	return GetPushBufferNode().buffer;
}

template <typename CLASS>
typename ipc::MessageQueue<CLASS>::Buffer & ipc::MessageQueue<CLASS>::GetPopBuffer()
{
	return GetPopBufferNode().buffer;
}
