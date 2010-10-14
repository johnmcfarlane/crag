/*
 *  for_each_chunk.h
 *  crag
 *  Definition for core::for_each_chunk and other supporting routines and classes.
 *
 *  Created by John on 9/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "chunk_functor.h"


namespace core
{
	
	////////////////////////////////////////////////////////////////////////////////
	// for_each_chunk function
	// 
	// Breaks the given iterator range, [first, last), into chunks of chunk_size elements
	// and passes them into the given function with signature: void function(I chunk_first, I chunk_last).
	
	template <typename I, typename F>
	void for_each_chunk(I first, I last, size_t chunk_size, F function)
	{
		I chunk_first = first;
		while (true)
		{
			I chunk_last = chunk_first + chunk_size;
			if (chunk_last >= last)
			{
				if (last > chunk_first)
				{
					function(chunk_first, last);
				}
				
				break;
			}
			
			function(chunk_first, chunk_last);
			
			chunk_first = chunk_last;
		}
	}

	
	////////////////////////////////////////////////////////////////////////////////
	// for_each_chunk_parallel
	// 
	// Builds a scheduler-specific functor from the given functor 
	// and passes it to the scheduler.
	
	template <typename ITERATOR, typename CHUNK_FUNCTOR> 
	void for_each_chunk_parallel(ITERATOR first, ITERATOR last, size_t step_size, CHUNK_FUNCTOR cf)
	{
		int num_nodes = last - first;
		if (num_nodes == 0)
		{
			return;
		}
		
		typedef chunk_functor_parallel<ITERATOR, CHUNK_FUNCTOR> CHUNK_FUNCTOR_PARALLEL;
		CHUNK_FUNCTOR_PARALLEL cfp (first, cf);
		
		smp::ForEach(0, num_nodes, step_size, cfp);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// supporting for_each_chunk functions
	// 
	
	template <typename ITERATOR, typename CHUNK_FUNCTOR>
	void for_each_chunk(ITERATOR first, ITERATOR last, size_t chunk_size, CHUNK_FUNCTOR chunk_functor, bool parallel, bool prefetch)
	{
		if (prefetch)
		{
			// Construct a new functor out of the given functor and a prefetch functor.
			typedef chunk_functor_prefetch<ITERATOR> CHUNK_FUNCTOR_PREFETCH;
			CHUNK_FUNCTOR_PREFETCH cfp;
			
			typedef chunk_functor_node <ITERATOR, CHUNK_FUNCTOR_PREFETCH, CHUNK_FUNCTOR> CHUNK_FUNCTOR_NODE;
			CHUNK_FUNCTOR_NODE cfn(cfp, chunk_functor);
			
			for_each_chunk<ITERATOR, CHUNK_FUNCTOR_NODE>(first, last, chunk_size, cfn, parallel, false);
		}
		else 
		{
			if (parallel)
			{
				for_each_chunk_parallel<ITERATOR, CHUNK_FUNCTOR>(first, last, chunk_size, chunk_functor);
			}
			else
			{
				for_each_chunk<ITERATOR, CHUNK_FUNCTOR>(first, last, chunk_size, chunk_functor);
			}
		}
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// supporting for_each functions
	// 
	
	template <typename ITERATOR, typename ITEM_FUNCTOR>
	void for_each(ITERATOR first, ITERATOR last, size_t chunk_size, ITEM_FUNCTOR item_functor, bool parallel, bool prefetch)
	{
		typedef chunk_functor_for_each<ITERATOR, ITEM_FUNCTOR> CHUNK_FUNCTOR;
		CHUNK_FUNCTOR cf(item_functor);
		
		for_each_chunk<ITERATOR, CHUNK_FUNCTOR>(first, last, chunk_size, cf, parallel, prefetch);
	}
	
	template <typename ITERATOR, typename ITEM_FUNCTOR1, typename ITEM_FUNCTOR2>
	void for_each(ITERATOR first, ITERATOR last, size_t chunk_size, ITEM_FUNCTOR1 item_functor1, ITEM_FUNCTOR2 item_functor2, bool parallel, bool prefetch)
	{
		typedef chunk_functor_for_each<ITERATOR, ITEM_FUNCTOR1> CHUNK_FUNCTOR1;
		CHUNK_FUNCTOR1 cf1(item_functor1);
		
		typedef chunk_functor_for_each<ITERATOR, ITEM_FUNCTOR2> CHUNK_FUNCTOR2;
		CHUNK_FUNCTOR2 cf2(item_functor2);
		
		typedef chunk_functor_node<ITERATOR, CHUNK_FUNCTOR1, CHUNK_FUNCTOR2> CHUNK_FUNCTOR_NODE;
		CHUNK_FUNCTOR_NODE cfn(item_functor1, item_functor2);
		
		for_each_chunk<ITERATOR, CHUNK_FUNCTOR_NODE>(first, last, chunk_size, cfn, parallel, prefetch);
	}
	
}
