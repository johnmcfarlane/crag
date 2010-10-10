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
	// This causes a compiler error for chunk_functor_for_each which never did nothing to noone. 
	// The irony is that MS's _For_each function writes over first, but it's f that causes the error.
	
	template <typename ITERATOR, typename ITEM_FUNCTOR>
	ITEM_FUNCTOR for_each (ITERATOR first, ITERATOR last, ITEM_FUNCTOR f)
	{
		for (ITERATOR i = first; i != last; ++ i)
		{
			f (* i);
		}
		
		return f;
	}
	
}
