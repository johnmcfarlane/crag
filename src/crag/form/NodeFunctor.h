/*
 *  NodeFunctor.h
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/debug.h"

namespace form { 
	
	class Node;

	class NodeFunctor
	{
	public:
		bool PerformPrefetchPass() const 
		{ 
			return false; 
		}
		
		void OnPrefetchPass(Node const & /*node*/)
		{
			// This function needs to be overridden in the derived class if PerformPrefetchPass returns true.
			// The node is already guranteed to be prefetched so use this fn to prefetch memory which is less direct.
			Assert(false);
		}
		
		void operator()(Node & /*node*/)
		{
			// This is the function that the derived class needs to override.
			Assert(false);
		}
	};

}
