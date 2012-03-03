//
//  Thruster.h
//  crag
//
//  Created by John McFarlane on 2011-11-19.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Light.h"

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
		Thruster();
		
		void Init(Renderer & renderer, void const *);
		
		void Update(float const & thrust_factor);

		LeafNode::PreRenderResult PreRender(Renderer const & renderer) override;
	private:
		void AddPuff(float thrust_factor, Renderer & renderer);
		
		// variables
		Renderer * _renderer;
		float _thrust_factor;
	};
}
