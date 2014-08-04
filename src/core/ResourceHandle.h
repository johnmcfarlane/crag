//
//  core/ResourceHandle.h
//  crag
//
//  Created by John McFarlane on 2014-08-02.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Resource.h"

namespace crag
{
	namespace core
	{
		// A fool-proof way of holding a reasonably efficient reference to a resource
		template <typename Type>
		class ResourceHandle
		{
		public:
			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(ResourceHandle, self)
				CRAG_VERIFY(self._resource);
				
				if (self._resource)
				{
					CRAG_VERIFY(* self._resource);
					
					// would require use of super type member and including Type's header
					//CRAG_VERIFY_TRUE(IsA<Type>() (self._resource->_type_id));
				}
			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
			
			ResourceHandle()
			{
				CRAG_VERIFY(* this);
			}
			
			ResourceHandle(ResourceHandle const & rhs)
			: _resource(rhs._resource)
			{
				CRAG_VERIFY(* this);
			}
			
			ResourceHandle(Resource const * resource)
			: _resource(resource)
			{
				CRAG_VERIFY(* this);
			}
			
			// cast to base types
			template <typename BaseType> 
			operator ResourceHandle<BaseType> () const
			{
				static_assert(std::is_base_of<BaseType, Type>::value, "invalid cast");
				return ResourceHandle<BaseType>(_resource);
			}

			operator bool() const
			{
				return _resource != nullptr;
			}
			
			Type const & operator*() const
			{
				ASSERT(* this);
				CRAG_VERIFY_TRUE(_resource);
				
				return _resource->get<Type>();
			}
			
			Type const * operator->() const
			{
				CRAG_VERIFY(* this);
				CRAG_VERIFY_TRUE(_resource);
				
				return & _resource->get<Type>();
			}
			
			Type const * get() const
			{
				CRAG_VERIFY(* this);

				return _resource ? & _resource->get<Type>() : nullptr;
			}
			
			void release()
			{
				CRAG_VERIFY(* this);

				_resource = nullptr;

				CRAG_VERIFY(* this);
			}
			
		private:
			
			Resource const * _resource = nullptr;
		};
	}
}
