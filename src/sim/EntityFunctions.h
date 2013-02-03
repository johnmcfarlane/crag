//
//  EntityFunctions.h
//  crag
//
//  Created by John McFarlane on 2011/11/15.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "gfx/object/BranchNode.h"

#include "gfx/Engine.h"

#include "geom/origin.h"

namespace physics
{
	class Engine;
}


namespace sim
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	class Engine;
	class Entity;
	
	////////////////////////////////////////////////////////////////////////////////
	// function declarations
	
	template <typename GFX_TYPE, typename INIT_DATA>
	gfx::BranchNodeHandle AddModelWithTransform(INIT_DATA & init_data, gfx::BranchNodeHandle const & parent, gfx::Transformation const & transformation)
	{
		// create branch node and place it as a child of the given parent
		auto branch_node = gfx::BranchNodeHandle::CreateHandle(transformation);
		gfx::Daemon::Call([branch_node, parent] (gfx::Engine & engine) {
			engine.OnSetParent(branch_node.GetUid(), parent.GetUid());
		});
		
		// create the leaf node and place it as a child of the branch node
		auto model = smp::Handle<GFX_TYPE>::CreateHandle(init_data);
		gfx::Daemon::Call([model, branch_node] (gfx::Engine & engine) {
			engine.OnSetParent(model.GetUid(), branch_node.GetUid());
		});

		// return the branch node - the one needed to move the pair through space
		return branch_node;
	}
	
	template <typename GFX_TYPE, typename INIT_DATA>
	gfx::BranchNodeHandle AddModelWithTransform(INIT_DATA & init_data, gfx::BranchNodeHandle const & parent)
	{
		return AddModelWithTransform<GFX_TYPE>(init_data, parent, Transformation::Matrix44::Identity());
	}
	
	template <typename GFX_TYPE, typename INIT_DATA>
	gfx::BranchNodeHandle AddModelWithTransform(INIT_DATA & init_data)
	{
		gfx::BranchNodeHandle parent;
		return AddModelWithTransform<GFX_TYPE>(init_data, parent);
	}
	
	// physically attaches the given entities to one another
	void AttachEntities(Entity & entity1, Entity & entity2, physics::Engine & physics_engine);
	
	void ResetOrigin(Entity & entity, geom::rel::Vector3 const & delta);
}
