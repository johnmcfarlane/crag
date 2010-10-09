/*
 *  chunk_functor.h
 *  crag
 *  Classes for making things to pass into core::for_each_chunk function
 *
 *  Created by John on 10/3/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "smp/ForEach.h"


namespace core
{
	
	////////////////////////////////////////////////////////////////////////////////
	// chunk_functor_null
	// 
	// A chunk functor that does nothing.
	
	template <typename ITERATOR>
	class chunk_functor_null
	{
	public:
		void operator() (ITERATOR, ITERATOR)
		{
			// Here's the line of code where we do nothing.
			// TODO: Consider removing this functor; it doesn't seem very useful.
		}
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// chunk_functor_for_each
	// 
	// A chunk functor that iterates over the input range 
	// and calls the given function for each element.
	
	template <typename ITERATOR, typename ITEM_FUNCTOR>
	class chunk_functor_for_each
	{
	public:
		chunk_functor_for_each(ITEM_FUNCTOR init_item_functor)
		: item_functor(init_item_functor)
		{
		}
		
		void operator() (ITERATOR first, ITERATOR last)
		{
			std::for_each<ITERATOR, ITEM_FUNCTOR>(first, last, item_functor);
		}
		
	private:
		
		ITEM_FUNCTOR item_functor;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// chunk_functor_prefetch
	// 
	// A chunk functor that prefetches the input range.
	// TODO: How do we eliminate this in favor of the actual prefetch function?
	
	template <typename ITERATOR>
	class chunk_functor_prefetch
	{
	public:
		chunk_functor_prefetch()
		{
		}
		
		void operator() (ITERATOR first, ITERATOR last)
		{
			PrefetchArray(first, last);
		}
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// chunk_functor_node
	// 
	
	template <typename ITERATOR, typename CHUNK_FUNCTOR, typename PRIOR_CHUNK_FUNCTOR>
	class chunk_functor_node
	{
	public:
		
		chunk_functor_node(CHUNK_FUNCTOR init_chunk_functor, PRIOR_CHUNK_FUNCTOR init_prior_chunk_functor)
		: chunk_functor(init_chunk_functor)
		, prior_chunk_functor(init_prior_chunk_functor)
		{
		}
		
		void operator() (ITERATOR first, ITERATOR last)
		{
			prior_chunk_functor(first, last);
			chunk_functor(first, last);
		}
		
	private:
		
		CHUNK_FUNCTOR chunk_functor;
		PRIOR_CHUNK_FUNCTOR prior_chunk_functor;
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
