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
#if ! defined(NDEBUG)
		class _Counter
		{
		public:
			_Counter()
			{
				CRAG_VERIFY_EQUAL(_num_counters, 0);
				++ _num_counters;
		
				CRAG_VERIFY_EQUAL(_num_objects, 0);
			}

			~_Counter()
			{
				CRAG_VERIFY_EQUAL(_num_objects, 0);

				-- _num_counters;
				CRAG_VERIFY_EQUAL(_num_counters, 0);
			}

			void IncrementObject()
			{
				CRAG_VERIFY_EQUAL(_num_counters, 1);
				++ _num_objects;
				CRAG_VERIFY_OP(_num_objects, >, 0);
			}

			void DecrementObject()
			{
				CRAG_VERIFY_EQUAL(_num_counters, 1);
				CRAG_VERIFY_OP(_num_objects, >, 0);
				-- _num_objects;
			}
	
		private:
			int _num_objects = 0;
			static int _num_counters;
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
	
#if ! defined(NDEBUG)
	template <typename ObjectType>
	int
	counted_object<ObjectType>::_Counter::_num_counters = 0;

	template <typename ObjectType>
	typename counted_object<ObjectType>::_Counter
	counted_object<ObjectType>::_counter;
#endif
}
