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
			allocate(num_bytes);
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
		
		// warning: performs bitwise copy on the contents of the buffer
		bool reserve(size_type new_capacity)
		{
			verify();

			ring_buffer copy(new_capacity);
			
			while (! empty())
			{
				size_type entry_size = get_header(_data_begin);
				void const * source = _data_begin + sizeof(size_type);
				
				void * destination = copy.allocate_entry(entry_size);
				
				// If we're out of space (can happen if new_capacity is less than old),
				if (destination == nullptr)
				{
					// nuke contents so they won't get destructed.
					ZeroObject(copy);
					
					// failure
					return false;
				}
				
				memcpy(destination, source, entry_size);
				
				pop_front();
			}
			// Now, copy has bitwise copy of this buffer
			
			// copy the pointers of the new copy back to this
			_buffer_begin = copy._buffer_begin;
			_buffer_end = copy._buffer_end;
			_data_begin = copy._data_begin;
			_data_end = copy._data_end;
			
			// and nuke copy, again, so its contents won't get destrcted.
			ZeroObject(copy);
			
			verify();

			// success
			return true;
		}
		
		// access
		BASE_CLASS & front() 
		{
			verify();
			assert(! empty());
			
			return * reinterpret_cast<BASE_CLASS *>(_data_begin + sizeof(size_type));
		}
		
		BASE_CLASS const & front() const
		{
			verify();
			assert(! empty());
			
			return * reinterpret_cast<BASE_CLASS *>(_data_begin + sizeof(size_type));
		}
		
		// modifiers
		template <typename CLASS>
		bool push_back(CLASS const & source)
		{
			verify();
			size_type source_size = round_up(sizeof(CLASS));
			
			// allocate space for the object
			CLASS * destination = reinterpret_cast<CLASS *>(allocate_entry(source_size));
			if (destination == nullptr)
			{
				verify();
				return false;
			}
			
			// copy-construct object
			new (destination) CLASS (source);
			
			verify();
			return true;
		}
		
		void pop_front()
		{
			assert(! empty());
			
			size_type entry_size = get_header(_data_begin);
			assert(entry_size > sizeof(size_type));
			assert(round_up(entry_size) == entry_size);
			
			// call d'tor for object
			BASE_CLASS & object = front();
			object.~BASE_CLASS();
			
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
				size_type const & next_entry_size = get_header(new_data_begin);
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
			
			verify();
		}
		
	private:
		
		void * allocate_entry(size_type source_size)
		{
			size_type header_size = sizeof(size_type);
			size_type entry_size = header_size + source_size;

			assert(source_size >= header_size);

			byte * destination_begin, * destination_end;
			if (! locate_next_entry(destination_begin, destination_end, entry_size))
			{
				// not enough room.
				return nullptr;
			}
			
			// write header
			get_header(destination_begin) = entry_size;
			
			// expand data range
			_data_end = (destination_end == _buffer_end) ? _buffer_begin : destination_end;
			
			return destination_begin + header_size;
		}
		
		// Finds the next adequate space of the given given size but doesn't reserve it.
		// Does, however, mark the end of the buffer if there's wraparound with a gap.
		bool locate_next_entry(byte * & destination_begin, byte * & destination_end, size_type entry_size)
		{
			destination_begin = _data_end;
			destination_end = destination_begin + entry_size;
			
			// If the existing data is contiguous,
			if (_data_begin <= _data_end)
			{
				// and there's enough room at the end,
				if (destination_end <= _buffer_end)
				{
					if (destination_end == _buffer_end && _data_begin == _buffer_begin)
					{
						// there's no room at the beginning and 
						// the ring buffer can never be full
						return false;
					}
					
					// we're good.
					return true;
				}

				// wrap around
				destination_begin = _buffer_begin;
				destination_end = destination_begin + entry_size;
				
				// and write blank header to denote wrap around.
				get_header(_data_end) = 0;
			}
			// Either way, proposed destination now starts earlier than _data_begin.
			
			// But does the destination end earlier than _data_begin? 
			return destination_end < _data_begin;
		}
		
		static size_type round_up(size_type num_bytes)
		{
			size_t mask = (sizeof(size_type) - 1);
			return (num_bytes + mask) & ~ mask;
		}
		
		size_type const & get_header(byte const * header_pos) const
		{
			assert(header_pos >= _buffer_begin);
			assert(header_pos < _buffer_end);
			assert(round_up(header_pos - _buffer_begin) == header_pos - _buffer_begin);
			
			size_type const & header = * reinterpret_cast<size_type const *>(header_pos);
			assert(header == round_up(header));
			assert(header == 0 || header >= sizeof(size_type) * 2);
			
			return header;
		}
		
		size_type & get_header(byte * header_pos) 
		{
			assert(header_pos >= _buffer_begin);
			assert(header_pos < _buffer_end);
			assert(round_up(header_pos - _buffer_begin) == (header_pos - _buffer_begin));
			
			size_type & header = * reinterpret_cast<size_type *>(header_pos);
			// header may be being got to initialize it so no tests apply
			
			return header;
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
			
			verify();
		}
		
		void verify() const
		{
#if ! defined(NDEBUG)
			if (_buffer_begin == 0)
			{
				assert(_buffer_end == 0);
				assert(_data_begin == 0);
				assert(_data_end == 0);
				return;
			}
			
			assert(capacity() >= sizeof(size_type) * 2);
			assert(capacity() == round_up(capacity()));
			assert(_data_begin >= _buffer_begin);
			assert(_data_begin < _buffer_end);
			assert(_data_end >= _buffer_begin);
			assert(_data_end < _buffer_end);
			
			if (empty())
			{
				assert(_data_begin == _buffer_begin);
				assert(_data_end == _buffer_begin);
				return;
			}
			
			if (_data_begin > _data_end)
			{
				size_type min_header = get_header(_buffer_begin);
				assert(min_header >= sizeof(size_type) * 2);
			}

			for (byte const * i = _data_begin; ; )
			{
				if (i == _data_end)
				{
					return;
				}
				
				size_type entry_size = get_header(i);
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
					
					size_type next_entry_size = get_header(i);
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
