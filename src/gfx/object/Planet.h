//
//  gfx/object/Planet.h
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"


namespace gfx
{
	// graphical representation of a Planet
	class Planet : public LeafNode
	{
	public:
		// types
		class UpdateParams
		{
		public:
			Scalar _radius_min, _radius_max;
		};
		
		// functions
		Planet();
		
		virtual void Update(UpdateParams const & params);
	private:
		virtual bool GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, RenderRange & range) const;
		virtual void Render() const;
		
		// variables
		UpdateParams _salient;
	};
}

