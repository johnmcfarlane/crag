//
//  gfx/Thruster.h
//  crag
//
//  Created by John McFarlane on 2011-11-19.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Light.h"

#include "Puff.h"


namespace gfx
{
	// forward-declarations
	class Sphere;
	
	// graphical representation of a sim::Vehicle::Thruster
	class Thruster : public Light
	{
	public:
		// types
		typedef Light super;
		
		// functions
		DECLARE_ALLOCATOR(Thruster);

		Thruster(Init const & init, Transformation const & local_transformation, float thrust_max);
		
		void Update(float const & thrust_factor);

		LeafNode::PreRenderResult PreRender() override;
	private:
		void AddPuff(float thrust_factor);
		
		// variables
		float const _thrust_max;
		float _thrust_factor;
	};
}
