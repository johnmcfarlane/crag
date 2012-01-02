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
	// forward-declarations
	class SphereMesh;
	
	// Puff class
	class Puff : public LeafNode
	{
	public:
		Puff(Scalar spawn_volume);
		
	private:
		void Init(Scene const & scene) override;
		
		gfx::Transformation const & Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;
		
		PreRenderResult PreRender() override;
		
		// Draw the object.
		void Render() const override;
		
		Time CalculateAge() const;
		
		// variables
		SphereMesh const * _sphere;
		Scalar _spawn_volume;
		Time _spawn_time;
		Scalar _radius;
		Color4b _color;
	};
}
