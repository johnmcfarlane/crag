/*
 *  UpdateGrandparentFunctor.h
 *  crag
 *
 *  Created by John on 8/6/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "NodeFunctor.h"


namespace form 
{
	class UpdateGrandparentFunctor : public NodeFunctor
	{
	public:
		bool PerformPrefetchPass() const
		{
			return true;
		}
		
		void OnPrefetchPass(Node const & node) const
		{
			PrefetchBlock(node.parent);
		}
		
		void operator()(Node & node)
		{
			if (node.children != nullptr)
			{
				node.parent->score = std::numeric_limits<float>::max();
			}
		}
	};
}
