//
//  counted_object.h
//  crag
//
//  Created by John on 2014-07-10.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "debug.h"

namespace crag
{
	////////////////////////////////////////////////////////////////////////////////
	// crag::counted_object class definition
	
	// debug-only interface for catching lingering instances on shutdown;
	// derive class, C, from crag::counted_object<C> and run with debug settings;
	// (may have linker-related issues on some platforms)
	template <typename ObjectType>
	class counted_object
	{
#if defined(CRAG_DEBUG)
		class _Counter
		{
		public:
			_Counter()
			: _num_objects(0)
			{
				CRAG_VERIFY_EQUAL(_num_objects, 0);
			}

			~_Counter()
			{
				CRAG_VERIFY_EQUAL(_num_objects, 0);
			}

			void IncrementObject()
			{
				++ _num_objects;
				CRAG_VERIFY_OP(_num_objects, >, 0);
			}

			void DecrementObject()
			{
				CRAG_VERIFY_OP(_num_objects, >, 0);
				-- _num_objects;
			}
	
		private:
			std::atomic<int> _num_objects;
		};

	public:
		counted_object() { _counter.IncrementObject(); }
		~counted_object() { _counter.DecrementObject(); }
	private:
		static _Counter _counter;
#endif
	};

	////////////////////////////////////////////////////////////////////////////////
	// crag::counted_object::_Counter member definitions
	
#if defined(CRAG_DEBUG)
	template <typename ObjectType>
	typename counted_object<ObjectType>::_Counter
	counted_object<ObjectType>::_counter;
#endif
}
