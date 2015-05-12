//
//  GhostBody.h
//  crag
//
//  Created by John on 2014-01-10.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Body.h"

namespace physics
{
	// a body with no collision or gravity
	class GhostBody final : public Body
	{
	public:
		CRAG_VERIFY_INVARIANTS_DECLARE(GhostBody);

		GhostBody(Transformation const & transformation, Vector3 const & velocity, Engine & engine);

		bool ObeysGravity() const override;

		void SetMass(Mass const & m) const override;
	private:
		bool OnCollision(Body & that_body, ContactFunction & contact_function) override;
		
		Vector3 const & GetGeomTranslation() const override;
		void SetGeomTranslation(Vector3 const & translation) override;

		Matrix33 const & GetGeomRotation() const override;
		void SetGeomRotation(Matrix33 const & matrix) override;
	};
}
