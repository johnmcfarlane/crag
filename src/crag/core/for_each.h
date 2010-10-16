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
	// for_each function
	// 
	// This is a reimplementation of std::for_each to compensate for the fact that 
	// Visual C++'s implementation myseriously fails to not copy when passed functor refs.
	// This causes a compiler error for chunk_functor_for_each which I don't have time to investigate. 
	
	template <typename ITERATOR, typename ITEM_FUNCTOR>
	ITEM_FUNCTOR for_each (ITERATOR first, ITERATOR last, ITEM_FUNCTOR f)
	{
		for (; first != last; ++ first)
		{
			f (* first);
		}
		
		return f;
	}
	

	////////////////////////////////////////////////////////////////////////////////
	// for_each function with unroll helper
	// 
	// As above but with the addition of an unroll helper value.
	// Assuming that the function loops (UNROLL_HINT * N) times, we can break it into two, nested loops. 
	// Because UNROLL_HINT is a constant, the compiler can unroll the inner loop to speed up the code.

	template <typename ITERATOR, typename ITEM_FUNCTOR, int UNROLL_HINT>
	ITEM_FUNCTOR for_each (ITERATOR first, ITERATOR last, ITEM_FUNCTOR f)
	{
		// UNROLL_HINT must be divisible by (last - first).
		Assert (((last - first) % UNROLL_HINT) == 0);
		
		while (first != last)
		{
			for (int count = UNROLL_HINT; count != 0; -- count)
			{
				f (* (first ++));
			}
		}
		
		return f;
	}

}
