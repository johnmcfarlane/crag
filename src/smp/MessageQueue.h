//
//  MessageQueue.h
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "SimpleMutex.h"
#include "smp.h"

#include "core/ring_buffer.h"


namespace smp
{
	template <typename CLASS, typename MESSAGE_BASE>
	class MessageQueue
	{
		typedef CLASS Class;
		typedef core::ring_buffer<MESSAGE_BASE, true> Buffer;
		typedef SimpleMutex Mutex;
		typedef std::lock_guard<std::mutex> Lock;
	public:
		typedef typename Buffer::value_type value_type;
		typedef typename Buffer::size_type size_type;
		
		MessageQueue(size_type capacity)
		: _buffer(capacity)
		{
		}
		
		// returns false iff the Daemon should quit
		bool DispatchMessage(Class & object)
		{
			// Slightly risky, but I think we can avoid a lock here.
			if (_buffer.empty())
			{
				return false;
			}
			
			// TODO: Look into ways to prevent locking of the buffer.
			// TODO: Look into ways to prevent locking of the buffer while executing a command.
			Lock critical_section(_mutex);
			
			Message<Class> const & envelope = _buffer.front();
			envelope(object);
			
			_buffer.pop_front();
			
			return true;
		}
		
		// returns number of messages process
		int DispatchMessages(Class & object)
		{
			int num_messages = 0;
			
			while (DispatchMessage(object))
			{
				++ num_messages;
			}
			
			return num_messages;
		}
		
		template <typename MESSAGE>
		void PushBack(MESSAGE const & object)
		{
			Lock critical_section(_mutex);
			
			if (! _buffer.push_back(object))
			{
				// may cause a lock-up, or buffer may clear
				assert(false);
				smp::Yield();
			}
		}
		
		bool IsEmpty() const
		{
			return _buffer.empty();
		}
		
		void Clear()
		{
			Lock critical_section(_mutex);
			_buffer.clear();
		}
		
	private:
		std::mutex _mutex;
		Buffer _buffer;
	};
}
