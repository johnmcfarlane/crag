//
//  ring_buffer.h
//  crag
//
//  Created by John McFarlane on 7/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#if ! defined(NDEBUG)
//#define CORE_RING_BUFFER_VERFICATION
#endif


namespace core
{
	// A sequence container which stores variable-sized objects of base class, BASE_CLASS.
	// Designed to easily push_back and pop_front. Requires allocation only to adjust capacity.
	template <typename BASE_CLASS>
	class ring_buffer
	{
		OBJECT_NO_COPY(ring_buffer);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		struct block
		{
			block * _next;
			char _buffer [];	// used to store an object whose base class is BASE_CLASS
			
			enum { header_size = sizeof(block *) };
		};
		
	public:
		typedef BASE_CLASS value_type;
		typedef size_t size_type;
		
		// const_iterator
		class const_iterator
		{
		public:
			// d'tor
			const_iterator() : _position(nullptr) { }
			const_iterator(const_iterator const & rhs) : _position(rhs._position) { }
			const_iterator(block const * position) : _position(const_cast<block *>(position)) { }
			
			// dereference
			value_type & operator * () { return * reinterpret_cast<value_type *>(_position->_buffer); }
			value_type * operator -> () { return reinterpret_cast<value_type *>(_position->_buffer); }
			
			// equality
			friend bool operator==(const_iterator const & lhs, const_iterator const & rhs) { return lhs._position == rhs._position; }
			friend bool operator!=(const_iterator const & lhs, const_iterator const & rhs) { return lhs._position != rhs._position; }
			
			const_iterator & operator++()
			{
				_position = _position->_next;
				return * this;
			}
			
		private:
			// points to block containing current object
			block * _position;
		};
		
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
			return & _data_begin == _data_end;
		}
		
		size_type capacity() const
		{
			return size_type(_buffer_end) - size_type(_buffer_begin);
		}
		
		// access
		value_type & front() 
		{
			VerifyObject(* this);
			assert(! empty());
			
			return * reinterpret_cast<value_type *>(_data_begin->_buffer);
		}
		
		value_type const & front() const
		{
			VerifyObject(* this);
			assert(! empty());
			
			return * reinterpret_cast<value_type *>(_data_begin->_buffer);
		}
		
		const_iterator begin() const
		{
			VerifyObject(* this);
			return const_iterator(_data_begin);
		}
		
		const_iterator end() const
		{
			VerifyObject(* this);
			return const_iterator(* _data_end);
		}
		
		// modifiers
		template <typename CLASS>
		bool push_back(CLASS const & source)
		{
			VerifyObject(* this);
			
			// CLASS must be derived from BASE_CLASS.
			static_assert(std::is_base_of<BASE_CLASS, CLASS>::value, "wrong base type");
			
			size_type source_size = round_up(sizeof(CLASS));
			
			// allocate space for the object
			value_type * back = allocate_object_memory(source_size);
			if (back == nullptr)
			{
				return false;
			}
			
			// copy-construct object
			new (back) CLASS (source);
			
			VerifyObject(* this);
			return true;
		}
		
		void pop_front()
		{
			VerifyObject(* this);
			assert(! empty());
			
			// get reference to the object to be deleted
			value_type & object = front();
			
			// if this was the last block,
			if (_data_end == & _data_begin->_next)
			{
				_data_end = & _data_begin;
			}
			
			// advance the beginning of the sequence
			_data_begin = _data_begin->_next;
			
			// call d'tor for object
			object.~value_type();
			
			VerifyObject(* this);
		}
		
		void clear()
		{
			while (! empty())
			{
				pop_front();
			}
			
			VerifyObject(* this);
		}
		
#if defined(VERIFY)
		void Verify() const
		{
			if (empty())
			{
				VerifyEqual(* _data_end, _data_begin);
			}
			
			if (_buffer_begin == nullptr)
			{
				assert(_buffer_end == nullptr);
				assert(_data_begin == nullptr);
				return;
			}
			
			static int entry = 0;
			if (entry > 0)
			{
				return;
			}
			++ entry;
			
			block const * data_end = * _data_end;
			
			verify_address(_buffer_begin);
			verify_address(_buffer_end);
			verify_address(_data_begin);
			verify_address(data_end);
			
			assert(capacity() >= block::header_size + sizeof(value_type));
			assert(_data_begin >= _buffer_begin);
			assert(_data_begin <= _buffer_end);
			assert(data_end >= _buffer_begin);
			assert(data_end <= _buffer_end);
			
			for (const_iterator i = const_iterator(_data_begin); i != const_iterator(* _data_end); ++ i)
			{
			}
			
			-- entry;
		}
#endif
		
	private:
		// allocates memory for the object
		value_type * allocate_object_memory(size_type source_size)
		{
			assert(source_size >= sizeof(value_type));
			size_type block_size = block::header_size + source_size;
			
			// Determine the beginning/end of the block where the object will live.
			block * block_begin, * block_end;
			
			// If there isn't space with the current buffer,
			if (! locate_next_block(block_begin, block_end, block_size))
			{
				// then fail.
				return nullptr;
			}
			
			// in case new block is not contiguous,
			// make sure the previous back's _next pointer is correct.
			* _data_end = block_begin;
			
			// write header
			block_begin->_next = block_end;
			
			// expand data range
			_data_end = & block_begin->_next;
			
			VerifyObject(* this);
			return reinterpret_cast<value_type *>(block_begin->_buffer);
		}
		
		// Finds the next adequate space of the given given size but doesn't reserve it.
		// Does, however, mark the end of the buffer if there's wraparound with a gap.
		bool locate_next_block(block * & block_begin, block * & block_end, size_type block_size) const
		{
			block_begin = * _data_end;
			block_end = reinterpret_cast<block *>(reinterpret_cast<char *>(block_begin) + block_size);
			
			// If the existing data is contiguous,
			if (_data_begin <= block_begin)
			{
				// and there's enough room at the end,
				if (block_end <= _buffer_end)
				{
					if (block_end == _buffer_end && _data_begin == _buffer_begin)
					{
						// there's no room at the beginning and 
						// the ring buffer can never be full
						return false;
					}
					
					// we're good.
					return true;
				}
				
				// wrap around
				block_begin = _buffer_begin;
				block_end = reinterpret_cast<block *>(reinterpret_cast<char *>(block_begin) + block_size);
			}
			// Either way, proposed destination now starts earlier than _data_begin.
			
			// But does the destination end earlier than _data_begin? 
			return block_end < _data_begin;
		}
		
		template <typename LHS, typename RHS>
		static ptrdiff_t bytewise_sub(LHS const * lhs, RHS const * rhs)
		{
			return reinterpret_cast<char const *>(lhs) - reinterpret_cast<char const *>(rhs);
		}
		
		static size_type round_down(size_type num_bytes)
		{
			size_t mask = (sizeof(size_type) - 1);
			return num_bytes & ~ mask;
		}
		
		static size_type round_up(size_type num_bytes)
		{
			size_t mask = (sizeof(size_type) - 1);
			return (num_bytes + mask) & ~ mask;
		}
		
		static size_type get_min_buffer_size()
		{
			// Return enough room for one block containing a base class object
			// plus the extra gap needed to avoid confusing begining and end.
			return block::header_size * 2 + sizeof(value_type);
		}
		
		void allocate(size_type num_bytes)
		{
			num_bytes = std::max(round_up(num_bytes), get_min_buffer_size());
			
			char * buffer = new char [num_bytes];
			_buffer_begin = reinterpret_cast<block *>(buffer);
			_buffer_end = reinterpret_cast<block *>(buffer + num_bytes);
			
			init_data();
		}
		
		void deallocate()
		{
			delete [] _buffer_begin;
		}
		
		// initialize data pointers to correct values
		void init_data()
		{
			_data_begin = _buffer_begin;
			_data_end = & _data_begin;
			
			VerifyObject(* this);
		}
		
		// Null this object (except _data_end can never be null).
		// Is used to ensure that new deallocation occurs in d'tor.
		void nuke()
		{
			_buffer_begin = _buffer_end = nullptr;
			init_data();
		}
		
		static void verify_address(size_type CRAG_DEBUG_PARAM(num_bytes))
		{
			assert(ring_buffer::round_down(num_bytes) == num_bytes);
		}
		
		static void verify_address(void const * num_bytes)
		{
			verify_address(size_type(num_bytes));
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		block * _buffer_begin;
		block * _buffer_end;
		block * _data_begin;
		block * * _data_end;
		
		// TODO: Slightly more efficient to append a zero-header to end of buffer?
	};
}
