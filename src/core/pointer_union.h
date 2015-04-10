//
//  core/pointer_union.h
//  crag
//
//  Created by john on 5/9/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/union_buffer.h"

namespace crag
{
	namespace core
	{
		namespace impl
		{
			// only works for variadic type with unique types
			template <typename Type, typename ... Types>
			struct tuple_index;

			template <typename Head, typename ... Types>
			struct tuple_index <Head, Head, Types ...>
			{
				static constexpr std::size_t value = 0;
			};

			template <typename Type, typename Head, typename ... Tail>
			struct tuple_index <Type, Head, Tail ...>
			{
				static constexpr std::size_t value = tuple_index<Type, Tail ...>::value + 1;
			};
		}

		////////////////////////////////////////////////////////////////////////////////
		// pointer_union type definition
		//
		// simplifies parameter set of pointer_union_base class template
		// allows run-time safe storage of alternate pointers in the same space

		template <typename ... Types>
		class pointer_union
		{
			// types
			using IndexType = std::uintptr_t;

		public:
			// functions
			bool operator == (std::nullptr_t) const
			{
				CRAG_VERIFY(* this);

				return _data == _null;
			}
			bool operator != (std::nullptr_t) const
			{
				CRAG_VERIFY(* this);

				return _data != _null;
			}

			template <typename Type>
			Type * operator=(Type * p)
			{
				CRAG_VERIFY(* this);
				static_assert(sizeof(p) == sizeof(IndexType), "all pointers used by pointer_union must be vanilla");

				if (p == nullptr)
				{
					(* this) = nullptr;
					CRAG_VERIFY_TRUE((* this) == nullptr);
					CRAG_VERIFY_EQUAL(find<Type>(), nullptr);

					CRAG_VERIFY(* this);
					return nullptr;
				}

				auto index = get_index<Type>();
				auto data = reinterpret_cast<IndexType>(p);
				CRAG_VERIFY_FALSE(data & _index_mask);

				_data = index | data;
				CRAG_VERIFY_TRUE((* this) != nullptr);
				CRAG_VERIFY_EQUAL(this->find<Type>(), p);

				CRAG_VERIFY(* this);
				return p;
			}

			std::nullptr_t operator=(std::nullptr_t)
			{
				CRAG_VERIFY(* this);

				_data = _null;
				return nullptr;
			}

			template <typename Type>
			Type * find()
			{
				CRAG_VERIFY(* this);

				// if no (non-null) pointer value is stored,
				if (! _data)
				{
					// return nullptr
					return nullptr;
				}

				// if a different type of pointer is stored,
				if (get_index() != get_index<Type>())
				{
					ASSERT(get_ptr());
					return nullptr;
				}

				return reinterpret_cast<Type *>(get_ptr());
			}

			template <typename Type>
			Type const * find() const
			{
				pointer_union & non_const_this = const_cast<pointer_union &>(* this);
				return non_const_this.find<Type>();
			}

			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(pointer_union, self)
				CRAG_VERIFY_EQUAL((self.get_index() == _null_index), (self.get_ptr() == _null));
			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

		private:
			IndexType get_index() const
			{
				return _data & _index_mask;
			}

			IndexType get_ptr() const
			{
				return _data & _ptr_mask;
			}

			// return index of given type
			template <typename Type>
			static constexpr IndexType get_index()
			{
				return impl::tuple_index<Type, Types...>::value + 1;
			}

			// constants
			static constexpr IndexType _num_types = sizeof...(Types);
			static constexpr IndexType _null_index = 0;
			static constexpr IndexType _null = 0;
			static constexpr IndexType _max_index = _num_types + 1;
			static constexpr IndexType _num_index_bits = Number<_max_index>::FIELD_SIZE;
			static constexpr IndexType _index_limit = 1 << _num_index_bits;
			static constexpr IndexType _index_mask = _index_limit - 1;
			static constexpr IndexType _ptr_mask = ~ _index_mask;

			// variables
			IndexType _data = 0;
		};
	}
}
