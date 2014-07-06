//
//  gfx/object/Puff.h
//  crag
//
//  Created by John McFarlane on 2011-12-10.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Object.h"

#include "gfx/Color.h"


namespace gfx
{
	// Puff class
	class Puff : public Object
	{
	public:
		DECLARE_ALLOCATOR(Puff);

		Puff(Engine & engine, Transformation const & local_transformation, Scalar spawn_volume);
		~Puff();
		
	private:
		virtual void UpdateModelViewTransformation(Transformation const & model_view) final;
		
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
