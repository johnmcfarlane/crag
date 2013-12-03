//
//  union_buffer.h
//  crag
//
//  Created by john on 2013-11-29.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace crag
{
	namespace core
	{
		////////////////////////////////////////////////////////////////////////////////
		// union_buffer

		// storage for type which is specified at point of access; use with caution
		template <int max_size, int max_align>
		class union_buffer
		{
		public:
			////////////////////////////////////////////////////////////////////////////////
			// functions
			
			// c'tor
			union_buffer()
			{
				clear();
			}
			
			// comparison
			friend bool operator==(union_buffer const & lhs, union_buffer const & rhs)
			{
				return lhs.storage == rhs.storage;
			}
			
			friend bool operator!=(union_buffer const & lhs, union_buffer const & rhs)
			{
				return ! (lhs == rhs);
			}
			
			// zero the buffer
			void clear()
			{
				std::fill(std::begin(storage), std::end(storage), 0);
			}
			
			// access buffer as if it is an object of type, Type
			template <typename Type>
			Type const & get() const
			{
				// consider increasing max_size class template parameter
				static_assert(sizeof(Type) <= max_size, "sizeof(Type) too big");

				// consider increasing max_align class template parameter
				static_assert(alignof(Type) <= max_align, "alignof(Type) too big");
				
				auto & begin = storage[0];
				auto p = reinterpret_cast<Type const *>(& begin);
				auto & r = ref(p);
				return r;
			}
			
			template <typename Type>
			Type & get()
			{
				return const_cast<Type &>((static_cast<union_buffer const &>(* this)).get<Type>());
			}
			
		private:
			////////////////////////////////////////////////////////////////////////////////
			// friends
			
			// i/o - represents union_buffer as a comma-separated sequence of hex byte values
			template <int _max_size, int _max_align>
			friend std::ostream & operator << (std::ostream & out, union_buffer<_max_size, _max_align> const & ub)
			{
				// numbers to be output as hex
				out << std::hex;

				// output each element
				for (auto & element : ub.storage)
				{
					// if not the first element,
					if (& element != std::begin(ub.storage))
					{
						// prefix with comma
						out << ',';
					}
				
					// elements to be output as numbers
					out << static_cast<unsigned>(element);
				}
			
				// assume default setting was in place prior to call
				out << std::dec;
			
				return out;
			}

			template <int _max_size, int _max_align>
			friend std::istream & operator >> (std::istream & in, union_buffer<_max_size, _max_align> & ub)
			{
				// numbers to be input as hex
				in >> std::hex;

				// input each element
				for (auto & element : ub.storage)
				{
					// if not the first element,
					if (& element != std::begin(ub.storage))
					{
						// eat the comma
						char comma;
						in >> comma;
						CRAG_VERIFY_EQUAL(static_cast<unsigned>(comma), static_cast<unsigned>(','));
					}
				
					// elements to be input as numbers
					int n;
					in >> n;
					element = n;
				}
			
				// assume default setting was in place prior to call
				in >> std::dec;

				return in;
			}

			////////////////////////////////////////////////////////////////////////////////
			// types

			typedef uint8_t ElementType;
			typedef std::array<ElementType, max_size> StorageType;

			////////////////////////////////////////////////////////////////////////////////
			// variables

			alignas(max_align) StorageType storage;
		};
	}
}
