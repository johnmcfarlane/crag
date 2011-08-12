/*
 *  chunk_functor.h
 *  crag
 *  Classes for making things to pass into core::for_each_chunk function
 *
 *  Created by John on 10/3/10.
 *  Copyright 2010-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
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
	
}
