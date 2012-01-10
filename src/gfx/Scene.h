//
//  Scene.h
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "object/BranchNode.h"
#include "object/LeafNode.h"
#include "Pov.h"

#include "sim/defs.h"


namespace gfx
{
	// forward-declarations
	class Cuboid;
	class Object;
	class SphereMesh;
	
	// Scene class definition
	// Stores graphical representation of the simulation
	// including point-of-view info needed to render.
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		void SetTime(Time t);
		Time GetTime() const;
		
		void AddObject(Object & object, BranchNode & parent);
		void RemoveObject(Uid uid);
		void SortRenderList();
		
		Object * GetObject(Uid object_uid);
		Object const * GetObject(Uid object_uid) const;
		
		BranchNode & GetRoot();
		BranchNode const & GetRoot() const;
		
		LeafNode::RenderList & GetRenderList();
		LeafNode::RenderList const & GetRenderList() const;
		
		void SetResolution(Vector2i const & r);
		void SetCameraTransformation(sim::Transformation const & transformation);
		
		Pov & GetPov();
		Pov const & GetPov() const;
		
		Cuboid const & GetCuboid() const;
		SphereMesh const & GetSphere() const;
		
	private:
		// remove the children of the given branch node
		void RemoveChildren(BranchNode & parent);

		// attributes
		Time _time;
		Pov pov;
		
		ObjectMap _objects;	// fast-access container of all objects
		BranchNode _root;	// root of object heirachy
		LeafNode::RenderList _render_list;
		
		Cuboid const & _cuboid;
		SphereMesh const & _sphere;
	};
}
