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
	
}
