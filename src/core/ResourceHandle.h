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
					// TODO: allow Type to be base type
					CRAG_VERIFY_EQUAL(self._resource->_type_id, TypeId::Create<Type>());
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
			
			ResourceHandle(Resource const & resource)
			{
				Set(resource);
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
			
			void Flush()
			{
				CRAG_VERIFY(* this);

				if (_resource)
				{
					_resource->Flush();
				}
			}
			
		private:
			void Set(Resource const & resource)
			{
				CRAG_VERIFY(* this);
				CRAG_VERIFY(resource);
				
				_resource = & resource;

				CRAG_VERIFY(* this);
			}
			
			Resource const * _resource = nullptr;
		};
	}
}
