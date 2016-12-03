//
//  ring_buffer.h
//  crag
//
//  Created by John McFarlane on 7/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

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

			char * buffer()
			{
				return reinterpret_cast<char *>((& _next) + 1);
			}

			static constexpr std::size_t header_size = sizeof(block *);
		};

	public:
		typedef BASE_CLASS value_type;
		typedef int size_type;

		// const_iterator
		class const_iterator
		{
		public:
			// d'tor
			const_iterator() : _position(nullptr) { }
			const_iterator(const_iterator const & rhs) : _position(rhs._position) { }
			const_iterator(block const * position) : _position(const_cast<block *>(position)) { }

			// dereference
			value_type & operator * () { return * reinterpret_cast<value_type *>(_position->buffer()); }
			value_type * operator -> () { return reinterpret_cast<value_type *>(_position->buffer()); }

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
			return static_cast<size_type>(reinterpret_cast<char const*>(_buffer_end) - reinterpret_cast<char const*>(_buffer_begin));
		}

		// access
		value_type & front()
		{
			CRAG_VERIFY(* this);
			assert(! empty());

			return * reinterpret_cast<value_type *>(_data_begin->buffer());
		}

		value_type const & front() const
		{
			CRAG_VERIFY(* this);
			assert(! empty());

			return * reinterpret_cast<value_type *>(_data_begin->buffer());
		}

		const_iterator begin() const
		{
			CRAG_VERIFY(* this);
			return const_iterator(_data_begin);
		}

		const_iterator end() const
		{
			CRAG_VERIFY(* this);
			return const_iterator(* _data_end);
		}

		// modifiers
		template <typename CLASS>
		bool push_back(CLASS const & source)
		{
			CRAG_VERIFY(* this);

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

			CRAG_VERIFY(* this);
			return true;
		}

		void pop_front()
		{
			CRAG_VERIFY(* this);
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

			CRAG_VERIFY(* this);
		}

		void clear()
		{
			while (! empty())
			{
				pop_front();
			}

			CRAG_VERIFY(* this);
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(ring_buffer, object)
			if (object.empty())
			{
				CRAG_VERIFY_EQUAL(* object._data_end, object._data_begin);
			}

			if (object._buffer_begin == nullptr)
			{
				assert(object._buffer_end == nullptr);
				assert(object._data_begin == nullptr);
				return;
			}

			static int entry = 0;
			if (entry > 0)
			{
				return;
			}
			++ entry;

			block const * data_end = * object._data_end;

			verify_address(object._buffer_begin);
			verify_address(object._buffer_end);
			verify_address(object._data_begin);
			verify_address(data_end);

			CRAG_VERIFY_OP(object.capacity(), >=, block::header_size + sizeof(value_type));
			CRAG_VERIFY_OP(object._data_begin, >=, object._buffer_begin);
			CRAG_VERIFY_OP(object._data_begin, <=, object._buffer_end);
			CRAG_VERIFY_OP(data_end, >=, object._buffer_begin);
			CRAG_VERIFY_OP(data_end, <=, object._buffer_end);

			for (const_iterator i = const_iterator(object._data_begin); i != const_iterator(* object._data_end); ++ i)
			{
			}

			-- entry;
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif	// defined(CRAG_VERIFY_ENABLED)

	private:
		// allocates memory for the object
		value_type * allocate_object_memory(size_type source_size)
		{
			assert(source_size >= static_cast<int>(sizeof(value_type)));
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

			CRAG_VERIFY(* this);
			return reinterpret_cast<value_type *>(block_begin->buffer());
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
			auto mask = static_cast<size_type>(sizeof(size_type)) - 1;
			return (num_bytes + mask) & ~ mask;
		}

		static size_type get_min_buffer_size()
		{
			// Return enough room for one block containing a base class object
			// plus the extra gap needed to avoid confusing begining and end.
			// Note: this doesn't even account for objects derived from
			// value_type being larger!
			return block::header_size * 2 + round_up(sizeof(value_type));
		}

		void allocate(size_type num_bytes)
		{
			// round up to a working bare minimum
			num_bytes = std::max(num_bytes, get_min_buffer_size());

			// round up to page size
			num_bytes = RoundToPageSize(num_bytes);

			char * buffer = reinterpret_cast<char *>(AllocatePage(num_bytes));
			_buffer_begin = reinterpret_cast<block *>(buffer);
			_buffer_end = reinterpret_cast<block *>(buffer + num_bytes);

			init_data();
		}

		void deallocate()
		{
			auto num_bytes = static_cast<size_type>(reinterpret_cast<char const *>(_buffer_end) - reinterpret_cast<char const *>(_buffer_begin));
			FreePage(_buffer_begin, num_bytes);
		}

		// initialize data pointers to correct values
		void init_data()
		{
			_data_begin = _buffer_begin;
			_data_end = & _data_begin;

			CRAG_VERIFY(* this);
		}

#if defined(CRAG_VERIFY_ENABLED)
		static void verify_address(std::intptr_t bytes)
		{
			CRAG_VERIFY_EQUAL(ring_buffer::round_down(bytes), bytes);
		}

		static void verify_address(void const * address)
		{
			verify_address(reinterpret_cast<std::intptr_t>(address));
		}
#endif

		////////////////////////////////////////////////////////////////////////////////
		// variables

		block * _buffer_begin;
		block * _buffer_end;
		block * _data_begin;
		block * * _data_end;

		// TODO: Slightly more efficient to append a zero-header to end of buffer?
	};
}
