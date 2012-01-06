//
//  gfx/object/Puff.h
//  crag
//
//  Created by John McFarlane on 2011-12-10.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"

#include "gfx/Color.h"


namespace gfx
{
	// Puff class
	class Puff : public LeafNode
	{
	public:
		Puff(Scalar spawn_volume);
		
	private:
		bool Init(Scene & scene) override;
		
		gfx::Transformation const & Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch, Time time) const override;
		
		PreRenderResult PreRender(Renderer const & renderer) override;
		
		// Draw the object.
		void Render(Renderer const & renderer) const override;
		
		Time CalculateAge(Time time) const;
		
		// variables
		Scalar _spawn_volume;
		Time _spawn_time;
		Scalar _radius;
		Color4b _color;
	};
}
