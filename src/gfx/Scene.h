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

#include "Pov.h"
#include "ShadowMap.h"

#include "object/Object.h"
#include "object/Light.h"

namespace gfx
{
	// forward-declarations
	class Object;
	
	// Scene class definition
	// Stores graphical representation of the simulation
	// including point-of-view info needed to render.
	class Scene
	{
		OBJECT_NO_COPY(Scene);
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		Scene(Engine & engine);
		~Scene();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Scene);
		
		void SetTime(core::Time t);
		core::Time GetTime() const;
		
		void AddObject(Object & object);
		void RemoveObject(Object & object);
		void SortRenderList();
		
		void AddLight(Light & light);
		void RemoveLight(Light & light);
		
		Object & GetRoot();
		Object const & GetRoot() const;
		
		Object::RenderList & GetRenderList();
		Object::RenderList const & GetRenderList() const;

		Light::List & GetLightList();
		Light::List const & GetLightList() const;

		ShadowMap & GetShadows();
		ShadowMap const & GetShadows() const;
		
		void SetPov(Pov const & pov);
		Pov & GetPov();
		Pov const & GetPov() const;
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// attributes
		core::Time _time;
		Pov _pov;
		
		Object _root;	// root of object heirachy; uses list hook in Object
		Object::RenderList _render_list;	// flat list of drawable objects
		Light::List _light_list;
		ShadowMap _shadows;
	};
}
