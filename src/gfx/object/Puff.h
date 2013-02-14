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
		DECLARE_ALLOCATOR(Puff);

		Puff(Init const & init, Scalar spawn_volume);
		~Puff();
		
	private:
		virtual Transformation const & Transform(Transformation const & model_view, Transformation & scratch) const final;
		
		PreRenderResult PreRender() override;
		
		// Draw the object.
		void Render(Engine const & renderer) const override;
		
		core::Time CalculateAge(core::Time time) const;
		
		// variables
		Scalar _spawn_volume;
		core::Time _spawn_time;
		float _radius;
		Color4f _color;
	};
}
