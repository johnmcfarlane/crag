//
//  MessageQueue.h
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/SimpleMutex.h"
#include "smp/smp.h"

#include "core/ring_buffer.h"


namespace ipc
{
	template <typename CLASS>
	class MessageQueue
	{
		OBJECT_NO_COPY(MessageQueue);
		
		//////////////////////////////////////////////////////////////////////////////
		// types

		class EnvelopeBase;

		template <typename MESSAGE>
		class Envelope;
	
		typedef CLASS Class;
		typedef core::ring_buffer<EnvelopeBase> Buffer;

		struct BufferNode;

		typedef smp::SimpleMutex Mutex;
		typedef std::lock_guard<Mutex> Lock;
	public:
		typedef typename Buffer::size_type size_type;
		
		//////////////////////////////////////////////////////////////////////////////
		// functions

		MessageQueue(size_type capacity);
		~MessageQueue();

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(MessageQueue, message_queue)
			CRAG_VERIFY_TRUE(message_queue._buffers != nullptr);

			// If there's only one buffer,
			if (message_queue._buffers->next == nullptr)
			{
				// then not much more can be tested.
				CRAG_VERIFY(message_queue._buffers->buffer);
				return;
			}

			// If there are multiple buffers,
			for (auto * buffer_node = message_queue._buffers; buffer_node != nullptr; buffer_node = buffer_node->next)
			{
				CRAG_VERIFY(buffer_node->buffer);

				// then none of them should be empty.
				ASSERT(! buffer_node->buffer.empty());
			}
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		bool IsEmpty() const;
		
		// adds message to queue
		template <typename MESSAGE>
		void PushBack(MESSAGE const & object);
		
		// returns false iff the Daemon should quit
		bool DispatchMessage(Class & object);
		
		// returns number of messages process
		int DispatchMessages(Class & object);
		
	private:
		template <typename MESSAGE>
		void CompleteDispatch(MESSAGE message, Class & object);

		void PopFront();

		BufferNode & GetPushBufferNode();
		BufferNode & GetPopBufferNode();

		Buffer & GetPushBuffer();
		Buffer & GetPopBuffer();
		
		//////////////////////////////////////////////////////////////////////////////
		// variables

		Mutex _mutex;
		BufferNode * _buffers;
	};
}
