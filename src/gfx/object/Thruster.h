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

#include "Puff.h"


namespace gfx
{
	// forward-declarations
	class Sphere;
	
	// graphical representation of a sim::Vehicle::Thruster
	class Thruster : public LeafNode
	{
	public:
		// types
		struct UpdateParams
		{
			float thrust_factor;
		};
		
		// functions
		Thruster();
		
		void Update(UpdateParams const & params);
	private:
		void AddPuff(float thrust_factor);
	};
}
