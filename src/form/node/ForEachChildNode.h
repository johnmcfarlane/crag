//
//  ForEachChildNode.h
//  crag
//
//  Created by john on 2013-08-28.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace form
{
	// for given node with children, passes a reference to each of them 
	// to the given function and returns true; 
	// for leaves, returns false
	template <typename FUNCTION, typename NODE>
	bool ForEachChildNode(NODE & node, FUNCTION function)
	{
		auto children = node.GetChildren();
		if (children == nullptr)
		{
			return false;
		}
		
		function(children[0]);
		function(children[1]);
		function(children[2]);
		function(children[3]);
		
		return true;
	}
}
