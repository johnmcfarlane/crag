//
//  core/iterable_object_pool.h
//  crag
//
//  Created by John McFarlane on 2015-05-23.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "object_pool.h"

namespace crag
{
	namespace core
	{
		// an object pool that can be swept through
		template <typename TYPE>
		class iterable_object_pool
		{
			// types
			class Bitmap;

			using buffer_type = ::core::object_pool<TYPE>;
			
		public:
			using value_type = TYPE;
			
			iterable_object_pool (int capacity)
			: object_pool(capacity)
			, allocated_bitmap(object_pool.capacity())
			{
			}

			constexpr int capacity() const noexcept
			{
				return object_pool.capacity();
			}

			constexpr bool empty() const noexcept
			{
				return object_pool.empty();
			}

			void * allocate() noexcept
			{
				auto allocation = object_pool.allocate();
				if (! allocation)
				{
					return nullptr;
				}

				auto index = object_pool.get_index(* static_cast<value_type const *>(allocation));
				CRAG_VERIFY_FALSE(allocated_bitmap.get(index));
				allocated_bitmap.set(index);

				return allocation;
			}

			void free(void * allocation)
			{
				if (! allocation)
				{
					return;
				}

				auto index = object_pool.get_index(* static_cast<value_type const *>(allocation));
				CRAG_VERIFY_TRUE(allocated_bitmap.get(index));
				allocated_bitmap.clear(index);

				object_pool.free(allocation);
			}
			
			bool is_allocated(value_type const & element) const noexcept
			{
				auto index = object_pool.get_index(element);

				auto result = allocated_bitmap.get(index);
				if (result)
				{
					object_pool.VerifyAllocatedElement(element);
				}
				else
				{
					object_pool.VerifyFreeElement(element);
				}

				return result;
			}

			template <typename FUNCTION>
			void for_each(FUNCTION function)
			{
				allocated_bitmap.for_each_true([&] (int index)
				{
					function(object_pool[index]);
				});
			}
			
		private:

			buffer_type object_pool;
			Bitmap allocated_bitmap;
		};

		template <typename TYPE>
		class iterable_object_pool<TYPE>::Bitmap
		{
			// types
			using buffer_element_type = std::uintptr_t;
			using buffer_type = std::vector<buffer_element_type>;

			// constants
			static constexpr int buffer_element_type_bytes = sizeof(buffer_element_type);	// 4/8
			static constexpr int buffer_element_type_bits = buffer_element_type_bytes * CHAR_BIT;	// 32/64
			static constexpr int buffer_element_type_bit_shift = Number<buffer_element_type_bits - 1>::BIT_COUNT;	// 5/6

		public:
			// functions
			constexpr Bitmap(int num_bits) noexcept
				: buffer(GetBufferCapacity(num_bits), false)
			{
			}

			~Bitmap() noexcept
			{
				CRAG_VERIFY_EQUAL(std::count(std::begin(buffer), std::end(buffer), true), 0);
			}

			constexpr bool empty() const noexcept
			{
				return std::all_of(std::begin(buffer), std::end(buffer), [] (buffer_element_type element)
				{
					return ! element;
				});
			}

			constexpr bool get(int index) const noexcept
			{
				return (* GetAddress(index).first & GetAddress(index).second) != 0;
			}

			void set(int index) noexcept
			{
				CRAG_VERIFY_OP(index, >=, 0);
				auto address = GetAddress(index);
				*address.first |= address.second;
			}

			void clear(int index) noexcept
			{
				auto address = GetAddress(index);
				*address.first &= ~ address.second;
			}

			template <typename FUNCTION>
			void for_each_true(FUNCTION function)
			{
				auto index = 0;
				for (auto element : buffer)
				{
					if (element == all_off)
					{
						index += buffer_element_type_bits;
						continue;
					}

					for (auto bit = buffer_element_type(1); bit; bit <<= 1)
					{
						if (element & bit)
						{
							function(index);
						}

						++ index;
					}
				}
			}

		private:
			std::pair<buffer_type::iterator, buffer_element_type> GetAddress(int index) noexcept
			{
				CRAG_VERIFY_OP(index, >=, 0);

				auto address_indices = GetAddressIndices(index);
				CRAG_VERIFY_OP(address_indices.first, >=, 0);
				CRAG_VERIFY_OP(static_cast<std::size_t>(address_indices.first), <, buffer.size());
				CRAG_VERIFY_OP(address_indices.second, >=, 0);
				//CRAG_VERIFY_OP(address_indices.second, <, buffer_element_type_bits);	// TODO

				return std::make_pair(
					std::begin(buffer) + address_indices.first,
					buffer_element_type(1) << address_indices.second);
			}

			constexpr std::pair<buffer_type::const_iterator, buffer_element_type> GetAddress(int index) const noexcept
			{
				return std::make_pair(
					static_cast<buffer_type::const_iterator>(const_cast<Bitmap *>(this)->GetAddress(index).first),
					const_cast<Bitmap *>(this)->GetAddress(index).second);
			}

			static constexpr std::pair<int, int> GetAddressIndices(int index) noexcept
			{
				return std::make_pair(
					(index >> buffer_element_type_bit_shift),
					index & (buffer_element_type_bits - 1));
			}

			static constexpr int GetBufferCapacity(int num_bits) noexcept
			{
				return ((num_bits - 1) >> buffer_element_type_bit_shift) + 1;
			}

			// constants
			static constexpr auto all_off =  buffer_element_type(0);
			static constexpr auto all_on =  ~ all_off;
			static_assert(
				all_on == std::numeric_limits<buffer_element_type>::max(),
				"conflicting alternative methods of generating all-set bit masks");

			// variables
			buffer_type buffer;
		};
	}
}
