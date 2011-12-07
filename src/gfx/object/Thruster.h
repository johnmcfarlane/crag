//
//  Thruster.h
//  crag
//
//  Created by John McFarlane on 2011-11-19.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"


namespace gfx
{
	// forward-declarations
	class Sphere;
	
	// graphical representation of a sim::Vehicle::Thruster
	class Thruster : public LeafNode
	{
	public:
		struct UpdateParams
		{
			float thrust;
		};
		
		// functions
		Thruster();
		
		void Update(UpdateParams const & params);

	private:
		Sphere const * _sphere;
		float _thrust;
	};
}
