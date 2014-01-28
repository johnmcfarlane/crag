//
//  core/Resource.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-19.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Resource.h"

using namespace crag::core;

namespace
{
	std::mutex resource_mutex;
}

////////////////////////////////////////////////////////////////////////////////
// crag::core::Resource member definitions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Resource, self)
	CRAG_VERIFY(self._type_id);
CRAG_VERIFY_INVARIANTS_DEFINE_END

Resource::Resource(CreateFunctionWrapperType create_function, TypeId type_id)
: _create_function(create_function)
, _type_id(type_id)
{
}

Resource::Resource(Resource && rhs)
: _object(std::move(rhs._object))
, _create_function(std::move(rhs._create_function))
, _type_id(rhs._type_id)
{
	rhs._type_id = TypeId();
}

void Resource::Prefetch() const
{
	// TODO: Need a lock here
	if (! _object)
	{
		_object = WrapperUniquePtr(_create_function());
	}
}

void Resource::Flush() const
{
	_object.reset();
}

TypeId Resource::GetTypeId() const
{
	return _type_id;
}
