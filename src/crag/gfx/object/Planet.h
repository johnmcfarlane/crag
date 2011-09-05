//
//  gfx/object/Planet.h
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Object.h"


namespace gfx
{
	// graphical representation of a Planet
	class Planet : public Object
	{
	public:
		// types
		class UpdateParams
		{
		public:
			Vector _position;
			Scalar _radius_min, _radius_max;
		};
		
		// functions
		Planet(Vector const & position);
		
		virtual void Update(UpdateParams const & params);
	private:
		virtual bool GetRenderRange(Ray const & camera_ray, double * range, bool wireframe) const;
		virtual void Render(Layer::type layer, gfx::Scene const & scene) const;
		virtual bool IsInLayer(Layer::type layer) const;
		
		// variables
		UpdateParams _salient;
	};
}

