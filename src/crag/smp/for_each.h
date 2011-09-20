/*
 *  Scheduler.h
 *  crag
 *
 *  Created by John on 8/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "scheduler.h"

#include "core/for_each.h"


namespace smp
{
	// ChunkedJob - parallelized for_each Job
	template <typename ITERATOR, typename FUNCTOR, size_t UNROLL_PITCH> 
	class ChunkedJob : public smp::scheduler::Job
	{
	public:
		// functions
		ChunkedJob(ITERATOR first, ITERATOR last, FUNCTOR functor, size_type chunk_size)
		: _functor(functor)
		, _first(first)
		, _last(last)
		, _chunk_size(chunk_size)
		{
		}
		
		size_type NumUnits() const
		{
			return ((_last - _first) + _chunk_size - 1) / _chunk_size;
		}
		
	private:
		// Note: These are offsets from init_first.
		virtual void operator () (size_type chunk_index)
		{
			ITERATOR chunk_first = _first + chunk_index * _chunk_size;
			ITERATOR chunk_last = std::min(chunk_first + _chunk_size, _last);
			core::for_each<ITERATOR, FUNCTOR, UNROLL_PITCH>(chunk_first, chunk_last, _functor);
		}
		
		// variables
		FUNCTOR _functor;
		ITERATOR _first;
		ITERATOR _last;
		size_type _chunk_size;
		size_type _num_chunks;
	};
	
	template <typename ITERATOR, typename FUNCTOR, size_t UNROLL_PITCH> 
	void for_each(ITERATOR first, ITERATOR last, FUNCTOR functor, size_t unit_size, int priority)
	{
		ChunkedJob<ITERATOR, FUNCTOR, UNROLL_PITCH> chunk_functor(first, last, functor, unit_size);

		scheduler::Complete(chunk_functor, chunk_functor.NumUnits(), priority);
	}
}
