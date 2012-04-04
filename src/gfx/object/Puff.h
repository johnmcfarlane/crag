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
		~Puff();
		
		bool Init(gfx::Engine & renderer);
		
	private:
		gfx::Transformation const & Transform(gfx::Engine & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;
		
		PreRenderResult PreRender(gfx::Engine const & renderer) override;
		
		// Draw the object.
		void Render(gfx::Engine const & renderer) const override;
		
		Time CalculateAge(Time time) const;
		
		// variables
		Scalar _spawn_volume;
		Time _spawn_time;
		float _radius;
		Color4f _color;
	};
}
