//
//  EntityFunctions.cpp
//  crag
//
//  Created by John McFarlane on 2011/11/15.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "EntityFunctions.h"

#include "Entity.h"
#include "Engine.h"

#include "physics/Body.h"
#include "physics/Engine.h"

#include "gfx/Engine.h"


namespace sim
{
	void AttachEntities(Entity & entity1, Entity & entity2, physics::Engine & physics_engine)
	{
		auto location1 = entity1.GetLocation();
		auto & body1 = core::StaticCast<physics::Body>(ref(location1));
		
		auto location2 = entity2.GetLocation();
		auto & body2 = core::StaticCast<physics::Body>(ref(location2));

		physics_engine.Attach(body1, body2);
	}

	void ResetOrigin(Entity & entity, geom::rel::Vector3 const & delta)
	{
		auto location = entity.GetLocation();
		if (location == nullptr)
		{
			return;
		}

		auto transformation = location->GetTransformation();
		transformation.SetTranslation(transformation.GetTranslation() - delta);
		location->SetTransformation(transformation);
	}
}
