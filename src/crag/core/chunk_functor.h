/*
 *  chunk_functor.h
 *  crag
 *  Classes for making things to pass into core::for_each_chunk function
 *
 *  Created by John on 10/3/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/for_each.h"

#include "smp/ForEach.h"


namespace core
{
	
	////////////////////////////////////////////////////////////////////////////////
	// chunk_functor_for_each
	// 
	// A chunk functor that iterates over the input range 
	// and calls the given function for each element.
	
	template <typename ITERATOR, typename ITEM_FUNCTOR, int UNROLL_HINT>
	class chunk_functor_for_each
	{
	public:
		chunk_functor_for_each(ITEM_FUNCTOR init_item_functor)
		: item_functor(init_item_functor)
		{
		}
		
		void operator() (ITERATOR first, ITERATOR last)
		{
			for_each<ITERATOR, ITEM_FUNCTOR, UNROLL_HINT>(first, last, item_functor);
		}
		
	private:
		
		ITEM_FUNCTOR item_functor;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// chunk_functor_parallel
	// 
	// This one is the odd one out; 
	// scheduler system uses polymorphic functor.
	
	template <typename ITERATOR, typename CHUNK_FUNCTOR> class chunk_functor_parallel : public smp::Functor
	{
	public:
		chunk_functor_parallel(ITERATOR init_first, CHUNK_FUNCTOR init_cf)
		: first(init_first)
		, cf(init_cf)
		{
		}
		
	private:
		// Note: These are offsets from init_first.
		virtual void operator () (int chunk_first, int chunk_last)
		{
			cf(first + chunk_first, first + chunk_last);
		}
		
		ITERATOR first;
		CHUNK_FUNCTOR cf;
	};
	
}
