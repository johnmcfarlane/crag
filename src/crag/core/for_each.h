/*
 *  for_each.h
 *  crag
 *
 *  Created by John on 10/9/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

namespace core
{

	////////////////////////////////////////////////////////////////////////////////
	// for_each function with unroll helper
	// 
	// As std::for_each but with the addition of an unroll helper value.
	// Assuming that the function loops (UNROLL_HINT * N) times, we can break it into two, nested loops. 
	// Because UNROLL_HINT is a constant, the compiler can unroll the inner loop to speed up the code.

	template <typename ITERATOR, typename ITEM_FUNCTOR, size_t UNROLL_PITCH>
	ITEM_FUNCTOR for_each (ITERATOR first, ITERATOR last, ITEM_FUNCTOR f)
	{
		// UNROLL_HINT must be divisible by (last - first).
		Assert (((last - first) % UNROLL_PITCH) == 0);
		
		while (first != last)
		{
			for (size_t count = UNROLL_PITCH; count != 0; -- count)
			{
				f (* (first ++));
			}
		}
		
		return f;
	}
	
	template <typename ITERATOR, typename ITEM_FUNCTOR>
	ITEM_FUNCTOR for_each (ITERATOR first, ITERATOR last, ITEM_FUNCTOR f)
	{
		return for_each <ITERATOR, ITEM_FUNCTOR, 1> (first, last, f);
	}

}
