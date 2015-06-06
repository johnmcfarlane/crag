//
//  core/RosterObjectDeclare.h
//  crag
//
//  Created by John McFarlane on 2015-05-29.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

// forward-declaration
namespace crag
{
	namespace core
	{
		template <typename TYPE>
		class iterable_object_pool;
	}
}

// inserted into an OBJECT's class definition;
// provides an allocation strategy that is geared towards efficient storage and visiting
#define CRAG_ROSTER_OBJECT_DECLARE(OBJECT_TYPE) \
	static crag::core::iterable_object_pool<OBJECT_TYPE> & GetPool() noexcept; \
	void* operator new(size_t sz) noexcept; \
	void* operator new [](size_t sz) noexcept; \
	void operator delete(void* p) noexcept; \
	void operator delete [](void* p) noexcept
