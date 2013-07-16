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
		auto body1 = entity1.GetBody();
		if (body1 == nullptr)
		{
			return;
		}

		auto body2 = entity2.GetBody();
		if (body2 == nullptr)
		{
			return;
		}

		physics_engine.Attach(* body1, * body2);
	}

	void ResetOrigin(Entity & entity, geom::rel::Vector3 const & delta)
	{
		auto body = entity.GetBody();
		if (body == nullptr)
		{
			return;
		}

		Vector3 bodyTranslation = body->GetTranslation();
		body->SetTranslation(bodyTranslation - delta);
	}
}
