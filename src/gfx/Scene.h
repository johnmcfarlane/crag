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

#include "object/RootNode.h"
#include "object/LeafNode.h"
#include "object/Light.h"
#include "Pov.h"

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
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Scene);
		
		void SetTime(core::Time t);
		core::Time GetTime() const;
		
		void AddObject(Object & object);
		void RemoveObject(Object & object);
		void SortRenderList();
		
		Object & GetRoot();
		Object const & GetRoot() const;
		
		LeafNode::RenderList & GetRenderList();
		LeafNode::RenderList const & GetRenderList() const;

		Light::List & GetLightList();
		
		void SetResolution(geom::Vector2i const & r);
		void SetCameraTransformation(Transformation const & transformation);
		
		Pov & GetPov();
		Pov const & GetPov() const;
		
	private:
		// attributes
		core::Time _time;
		Pov pov;
		
		RootNode _root;	// root of object heirachy; uses list hook in Object
		LeafNode::RenderList _render_list;	// flat list of drawable objects
		Light::List _light_list;
	};
}
