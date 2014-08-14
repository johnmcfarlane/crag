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
	OBJECT_NO_COPY(BufferNode);

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
	std::unique_ptr<BufferNode> next;
};

template <typename CLASS>
ipc::MessageQueue<CLASS>::MessageQueue(size_type capacity)
: _buffers(new BufferNode(capacity))
{
}

template <typename CLASS>
ipc::MessageQueue<CLASS>::~MessageQueue()
{
	CRAG_VERIFY(* this);
	ASSERT(IsEmpty());

	while (! GetPopBuffer().empty())
	{
		PopFront();
	}
	
	ASSERT(_buffers->buffer.empty());
	ASSERT(_buffers->next == nullptr);
}

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
	CRAG_VERIFY(* this);
	
	auto * node = & GetPushBufferNode();

	for (;;)
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

		ASSERT(node->next == nullptr);
		node->next.reset(new BufferNode(new_capacity));
		node = node->next.get();

		smp::Yield();
	}

	CRAG_VERIFY(* this);
}

template <typename CLASS>
bool ipc::MessageQueue<CLASS>::DispatchMessage(Class & object)
{
	_mutex.lock();
	CRAG_VERIFY(* this);
	
	auto & pull_buffer = GetPopBuffer();
	if (pull_buffer.empty())
	{
		_mutex.unlock();
		return false;
	}
	
	EnvelopeBase const & message_wrapper = pull_buffer.front();

	// calls complete dispatch with a (non-sliced) copy of object
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
	PopFront();
	_mutex.unlock();
	// WARNING: However, the popped envelope is the 'this' parameter below here in the stack. 
	// So an awkward silence must be maintained while returning to DispatchMessage.

	message(object);
}

template <typename CLASS>
void ipc::MessageQueue<CLASS>::PopFront()
{
	CRAG_VERIFY(* this);

	auto & front_buffer = _buffers->buffer;
	front_buffer.pop_front();

	if (! front_buffer.empty())
	{
		return;
	}
	
	auto & next = _buffers->next;
	if (! next)
	{
		return;
	}
	
	_buffers = std::move(next);

	CRAG_VERIFY(* this);
}

template <typename CLASS>
typename ipc::MessageQueue<CLASS>::BufferNode & ipc::MessageQueue<CLASS>::GetPushBufferNode()
{
	auto buffer_node = _buffers.get();

	for (;;)
	{
		auto next = buffer_node->next.get();
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
