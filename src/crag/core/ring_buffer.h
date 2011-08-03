//
//  ring_buffer.h
//  crag
//
//  Created by John McFarlane on 7/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "atomic.h"

#include <algorithm>


namespace core
{
	template <typename BASE_CLASS, typename SIZE_TYPE = size_t>
	class ring_buffer
	{
		OBJECT_NO_COPY(ring_buffer);

		////////////////////////////////////////////////////////////////////////////////
		// types
		typedef unsigned char byte;
	public:
		typedef SIZE_TYPE size_type;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		ring_buffer(size_type num_bytes)
		{
			allocate(round_up(num_bytes));
		}
		
		~ring_buffer()
		{
			clear();
			deallocate();
		}
		
		// capacity
		bool empty() const
		{
			return _data_begin == _data_end;
		}
		
		size_type size() const
		{
			return (_data_begin <= _data_end)
			? _data_end - _data_begin
			: capacity() - (_data_begin - _data_end);
		}
		
		size_type capacity() const	// note: the ring buffer can only store capacity() - 1 bytes
		{
			return _buffer_end - _buffer_begin;
		}
		
		// access
		BASE_CLASS const & front() const
		{
			assert(! empty());
			
			return * reinterpret_cast<BASE_CLASS *>(_data_begin + sizeof(size_type));
		}
		
		// modifiers
		template <typename CLASS>
		bool push_back(CLASS const & source)
		{
			verify();
			
			size_type header_size = sizeof(size_type);
			size_type source_size = round_up(sizeof(CLASS));
			size_type entry_size = header_size + source_size;
			
			byte * destination_begin, * destination_end;
			if (! allocate_entry(destination_begin, destination_end, entry_size))
			{
				// not enough room.
				return false;
			}
			
			// write header
			* reinterpret_cast<size_type *>(destination_begin) = entry_size;
			
			// copy-construct object
			new (destination_begin + header_size) CLASS (source);
			
			// update member variable. this MUST be done last of all for thread safety.
			_data_end = (destination_end == _buffer_end) ? _buffer_begin : destination_end;
			
			verify();
			return true;
		}
		
		void pop_front()
		{
			verify();
			assert(! empty());
			
			size_type const & entry_size = * reinterpret_cast<size_type *>(_data_begin);
			assert(entry_size > sizeof(size_type));
			assert(round_up(entry_size) == entry_size);
			
			// call d'tor for object
			BASE_CLASS * object = reinterpret_cast<BASE_CLASS *>(_data_begin + sizeof(size_type));
			object->~BASE_CLASS();
			
			byte * new_data_begin = _data_begin + entry_size;
			
			// did the object fit snuggly to the end of the buffer?
			if (new_data_begin == _buffer_end)
			{
				// wrap around
				new_data_begin = _buffer_begin;
			}
			else if (new_data_begin != _data_end)
			{
				assert(new_data_begin + sizeof(size_type) <= _buffer_end);
				
				// is the next object too big to fit at the end of the buffer?
				size_type const & next_entry_size = * reinterpret_cast<size_type *>(new_data_begin);
				if (next_entry_size == 0)
				{
					// wrap around
					new_data_begin = _buffer_begin;
				}
			}

			// If the buffer is becomming empty,
			if (new_data_begin == _data_end)
			{
				// reset the pointers.
				// This prevents the situation where it is left
				// with a zero header at the end of the buffer.
				init();
			}
			else
			{
				_data_begin = new_data_begin;
			}
			
			verify();
		}
		
		void clear()
		{
			while (! empty())
			{
				pop_front();
			}
		}
		
	private:
		
		bool allocate_entry(byte * & destination_begin, byte * & destination_end, size_type entry_size)
		{
			destination_begin = _data_end;
			destination_end = destination_begin + entry_size;
			
			// If the existing data is contiguous,
			if (_data_begin <= _data_end)
			{
				// and there's enough room at the end,
				if (destination_end <= _buffer_end)
				{
					// we're good.
					return true;
				}

				// wrap around
				destination_begin = _buffer_begin;
				destination_end = destination_begin + entry_size;
				
				// and write blank header to denote wrap around.
				* reinterpret_cast<size_type *>(_data_end) = 0;
			}
			// Either way, proposed destination now starts earlier than _data_begin.
			
			// But does the destination end earlier than _data_begin? 
			return destination_end < _data_begin;
		}
		
		size_type round_up(size_type num_bytes)
		{
			size_t mask = (sizeof(size_type) - 1);
			return (num_bytes + mask) & ~ mask;
		}
		
		void allocate(size_type num_bytes)
		{
			assert((num_bytes & (sizeof(size_type) - 1)) == 0);
			
			_buffer_begin = new byte [num_bytes];
			_buffer_end = _buffer_begin + num_bytes;
			
			init();
		}
		
		void deallocate()
		{
			delete [] _buffer_begin;
		}
		
		void init()
		{
			_data_begin = _buffer_begin;
			_data_end = _buffer_begin;
		}
		
		void verify() const
		{
#if ! defined(NDEBUG)
			assert(_data_begin >= _buffer_begin);
			assert(_data_begin < _buffer_end);
			assert(_data_end >= _buffer_begin);
			assert(_data_end < _buffer_end);

			for (byte const * i = _data_begin; ; )
			{
				if (i == _data_end)
				{
					return;
				}
				
				size_type entry_size = * reinterpret_cast<size_type const *>(i);
				assert(entry_size > sizeof(size_type));
				assert((entry_size & (sizeof(size_type) - 3)) == 0);
				
				bool before_begin = i < _data_begin;
				i += entry_size;
				if (i == _buffer_end)
				{
					assert(_data_end < _data_begin);
					i = _buffer_begin;
				}
				else 
				{
					if (i == _data_end)
					{
						return;
					}
					
					size_type next_entry_size = * reinterpret_cast<size_type const *>(i);
					if (next_entry_size == 0)
					{
						assert(_data_end < _data_begin);
						i = _buffer_begin;
					}
				}
				
				assert(! before_begin || i < _data_begin);
			}
#endif
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		byte * _buffer_begin;
		byte * _buffer_end;
		byte * _data_begin;
		byte * _data_end;
		
		// TODO: Slightly more efficient to append a zero-header to end of buffer?
	};
}
