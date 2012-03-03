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

#include "gfx/Object/BranchNode.h"

#include "gfx/Renderer.h"


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
	class Simulation;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// function declarations
	
	template <typename GFX_TYPE, typename INIT_DATA>
	gfx::BranchNodeHandle AddModelWithTransform(INIT_DATA & init_data, gfx::BranchNodeHandle const & parent, gfx::Transformation const & transformation)
	{
		// create branch node and place it as a child of the given parent
		gfx::BranchNodeHandle branch_node;
		branch_node.Create(transformation);
		gfx::Daemon::Call(branch_node.GetUid(), parent.GetUid(), & gfx::Renderer::OnSetParent);
		
		// create the leaf node and place it as a child of the branch node
		smp::Handle<GFX_TYPE> model;
		model.Create(init_data);
		gfx::Daemon::Call(model.GetUid(), branch_node.GetUid(), & gfx::Renderer::OnSetParent);

		// return the branch node - the one needed to move the pair through space
		return branch_node;
	}
	
	template <typename GFX_TYPE, typename INIT_DATA>
	gfx::BranchNodeHandle AddModelWithTransform(INIT_DATA & init_data, gfx::BranchNodeHandle const & parent)
	{
		return AddModelWithTransform<GFX_TYPE>(init_data, parent, Transformation::Matrix::Identity());
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
}
