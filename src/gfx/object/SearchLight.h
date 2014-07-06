//
//  SearchLight.h
//  crag
//
//  Created by John on 2014-05-27.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Light.h"

namespace gfx
{
	// a directed light with a cone-shaped beam
	class SearchLight final : public Light
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		SearchLight(
			Engine & engine, Transformation const & local_transformation, 
			Color4f const & color, Vector2 angle, 
			ObjectHandle exception = ObjectHandle());

		CRAG_VERIFY_INVARIANTS_DECLARE(SearchLight);
		
		// sets angle of the beam cone, (sin, cos)
		void SetAngle(Vector2 angle) override;
		Vector2 GetAngle() const override;
		
	private:
		// variables
		Vector2 _angle;
	};
}
