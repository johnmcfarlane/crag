//
//  EntityFunctions.h
//  crag
//
//  Created by John McFarlane on 2011/11/15.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "axes.h"
#include "defs.h"

#include "gfx/object/BranchNode.h"

#include "gfx/Engine.h"


namespace physics
{
	class Engine;
}


namespace sim
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	class Entity;
	class EntitySet;
	class Engine;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// function declarations
	
	template <typename GFX_TYPE, typename INIT_DATA>
	gfx::BranchNodeHandle AddModelWithTransform(INIT_DATA & init_data, gfx::BranchNodeHandle const & parent, gfx::Transformation const & transformation)
	{
		// create branch node and place it as a child of the given parent
		gfx::BranchNodeHandle branch_node;
		branch_node.Create(transformation);
		gfx::Daemon::Call([branch_node, parent] (gfx::Engine & engine) {
			engine.OnSetParent(branch_node.GetUid(), parent.GetUid());
		});
		
		// create the leaf node and place it as a child of the branch node
		smp::Handle<GFX_TYPE> model;
		model.Create(init_data);
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
	
	// sends render info from simulation to renderer
	void UpdateModels(EntitySet const & entity_set);
	
	// physically attaches the given entities to one another
	void AttachEntities(Uid const & uid1, Uid const & uid2, EntitySet & entity_set, physics::Engine & physics_engine);
	
	// sets physical property of being collidable
	void SetCollidable(Entity & entity, bool collidable);
	
	void ResetOrigin(EntitySet & entity_set, axes::VectorRel const & delta);
}
