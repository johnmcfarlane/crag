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
#include "object/Light.h"
#include "Pov.h"

#include "sim/defs.h"


namespace gfx
{
	// forward-declarations
	class Object;
	
	// Scene class definition
	// Stores graphical representation of the simulation
	// including point-of-view info needed to render.
	class Scene
	{
	public:
		Scene(Engine & engine);
		~Scene();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		void SetTime(core::Time t);
		core::Time GetTime() const;
		
		void AddObject(Object & object);
		void RemoveObject(Uid uid);
		void SortRenderList();
		
		Object * GetObject(Uid object_uid);
		Object const * GetObject(Uid object_uid) const;
		
		BranchNode & GetRoot();
		BranchNode const & GetRoot() const;
		
		LeafNode::RenderList & GetRenderList();
		LeafNode::RenderList const & GetRenderList() const;

		Light::List & GetLightList();
		
		void SetResolution(geom::Vector2i const & r);
		void SetCameraTransformation(sim::Transformation const & transformation);
		
		Pov & GetPov();
		Pov const & GetPov() const;
		
	private:
		// remove the children of the given branch node
		void RemoveChildren(BranchNode & parent);

		// attributes
		core::Time _time;
		Pov pov;
		
		ObjectMap _objects;	// fast-access container of all objects
		BranchNode _root;	// root of object heirachy
		LeafNode::RenderList _render_list;
		Light::List _light_list;
	};
}
