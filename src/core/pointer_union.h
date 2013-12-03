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
			////////////////////////////////////////////////////////////////////////////////
			// traits
			
			template <typename ... Types>
			struct traits;
			
			template <typename HeadType>
			struct traits <HeadType>
			{
				template <typename Type>
				static constexpr int index()
				{
					return std::is_same<Type, HeadType>::value ? 0 : -1;
				}

				static constexpr int pointer_size()
				{
					return sizeof(HeadType *);
				}
				
				static constexpr int pointer_align()
				{
					return alignof(HeadType *);
				}

				static constexpr int object_size()
				{
					return sizeof(HeadType);
				}
				
				static constexpr int object_align()
				{
					return alignof(HeadType);
				}
			};
			
			template <typename HeadType, typename ... TailTypes>
			struct traits <HeadType, TailTypes ...>
			{
				template <typename Type>
				static constexpr int index()
				{
					return std::is_same<Type, HeadType>::value ? sizeof ... (TailTypes) : traits<TailTypes ...>::template index<Type>();
				}

				static constexpr int pointer_size()
				{
					return (traits<HeadType *>::pointer_size() > traits<TailTypes ...>::pointer_size()) ? traits<HeadType *>::pointer_size() : traits<TailTypes ...>::pointer_size();
				}
				
				static constexpr int pointer_align()
				{
					return (traits<HeadType *>::pointer_align() > traits<TailTypes ...>::pointer_align()) ? traits<HeadType *>::pointer_align() : traits<TailTypes ...>::pointer_align();
				}

				static constexpr int object_size()
				{
					return (traits<HeadType *>::object_size() > traits<TailTypes ...>::object_size()) ? traits<HeadType *>::object_size() : traits<TailTypes ...>::object_size();
				}
				
				static constexpr int object_align()
				{
					return (traits<HeadType *>::object_align() > traits<TailTypes ...>::object_align()) ? traits<HeadType *>::object_align() : traits<TailTypes ...>::object_align();
				}
			};
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// pointer_union_base class definition
		
		// base class template for pointer_union type
		template <int pointer_size, int pointer_align, int object_size, int object_align, typename ... Types>
		class pointer_union_base
		{
			// type
			typedef union_buffer<pointer_size, pointer_align> Unionbuffer;
			typedef uintptr_t IndexType;
			
		public:
			// functions
			pointer_union_base() = default;
			pointer_union_base(std::nullptr_t) { }
			pointer_union_base(pointer_union_base const &) = default;
			~pointer_union_base()
			{
				CRAG_VERIFY(* this);
			}
			
			pointer_union_base & operator=(pointer_union_base const &) = default;

			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(pointer_union_base, self)
			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
			
			template <typename Type>
			static constexpr bool can_contain()
			{
				return get_index_of_type<Type>() != -1;
			}
			
			void clear()
			{
				buffer.clear();
			}
			
			operator bool() const
			{
				return * this != nullptr;
			}
			
			bool operator==(std::nullptr_t) const
			{
				return * this == pointer_union_base();
			}
			
			bool operator==(pointer_union_base const & rhs) const
			{
				return buffer == rhs.buffer;
			}
			
			template <typename Type>
			bool operator==(Type const * rhs) const
			{
				return find<Type>() == rhs;
			}
			
			template <typename Type>
			bool operator!=(Type const & rhs) const
			{
				return ! operator==(rhs);
			}
			
			template <typename Type>
			pointer_union_base & operator=(Type * ptr)
			{
				CRAG_VERIFY(* this);
				CRAG_VERIFY(ptr);
				CRAG_VERIFY_FALSE(reinterpret_cast<uintptr_t>(ptr) & (object_align - 1));
				
				// get index of Type
				constexpr auto index = get_index_of_type<Type>();
				static_assert(index != -1, "Type not found in Types");
				static_assert(! (index & ~ index_mask), "index exceeds capacity");
				
				// deal with the special case of a nullptr
				if (! ptr)
				{
					// in case of null, we want to blank the entire store for easy null checking
					buffer.clear();
					return * this;
				}

				// get a reference to the pointer as the index type
				auto & ptr_as_index = reinterpret_cast<IndexType &>(ptr);
				ptr_as_index |= index;
				
				auto & dest = buffer.template get<Type *>();
				dest = ptr;
				
				CRAG_VERIFY(* this);

				return * this;
			}
			
			template <typename Type>
			Type * find() const
			{
				CRAG_VERIFY(* this);
				
				// get index of Type
				constexpr IndexType required_index = get_index_of_type<Type>();
				static_assert(required_index != -1, "Type not stored");
				static_assert(! (required_index & ~ index_mask), "index exceeds capacity");

				// access this' data assuming that it's the type in question
				auto ptr = buffer.template get<Type *>();
				
				// test the previous assumption
				auto & ptr_as_index = reinterpret_cast<IndexType &>(ptr);
				auto stored_index = ptr_as_index & index_mask;
				
				if (stored_index != required_index)
				{
					// this doesn't represent a pointer to Type
					return nullptr;
				}
				
				ptr_as_index &= ~ index_mask;
				
				return ptr;
			}
			
		private:
			IndexType get_index() const
			{
				return buffer.template get<IndexType>() & index_mask;
			}
			
			template <typename Type>
			static constexpr int get_index_of_type()
			{
				typedef ::crag::core::impl::traits<Types ...> traits;
				return traits::template index<Type>();
			}
			
			// friends
			template <int _pointer_size, int _pointer_align, int _object_size, int _object_align, typename ... _Types>
			friend std::ostream & operator << (std::ostream & out, pointer_union_base<_pointer_size, _pointer_align, _object_size, _object_align, _Types ...> const & pu)
			{
				return out << pu.buffer;
			}

			template <int _pointer_size, int _pointer_align, int _object_size, int _object_align, typename ... _Types>
			friend std::istream & operator >> (std::istream & in, pointer_union_base<_pointer_size, _pointer_align, _object_size, _object_align, _Types ...> & pu)
			{
				return in >> pu.buffer;
			}

			// variables
			Unionbuffer buffer;
			
			static constexpr IndexType index_limit = object_align;
			static constexpr IndexType index_mask = index_limit - 1;
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// pointer_union type definition
		//
		// simplifies parameter set of pointer_union_base class template
		// allows run-time safe storage of alternate pointers in the same space

		template <typename ... Types>
		using pointer_union = pointer_union_base<
			::crag::core::impl::traits<Types ...>::pointer_size(), 
			::crag::core::impl::traits<Types ...>::pointer_align(),
			::crag::core::impl::traits<Types ...>::object_size(), 
			::crag::core::impl::traits<Types ...>::object_align(), 
			Types ...>;
	}
}
