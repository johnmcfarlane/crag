/*
 *  Scene.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#include "BranchNode.h"
#include "Pov.h"

#include "sim/defs.h"


namespace gfx
{
	class Object;
	class Cuboid;
	class Sphere;
	
	
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		void AddObject(Object & object, Uid parent_uid);
		void RemoveObject(Uid uid);
		
		template <typename OBJECT> 
		void UpdateObject(Uid uid, typename OBJECT::UpdateParams const & params)
		{
			ObjectMap::iterator i = _objects.find(uid);
			if (i == _objects.end())
			{
				Assert(false);
				return;
			}
			
			OBJECT & object = static_cast<OBJECT &>(* i->second);
			object.Update(params);
		}
		
		ObjectMap & GetObjectMap();
		ObjectMap const & GetObjectMap() const;
		
		BranchNode & GetRoot();
		BranchNode const & GetRoot() const;
		
		void SetResolution(Vector2i const & r);
		void SetCameraTransformation(sim::Transformation const & transformation);
		
		Pov & GetPov();
		Pov const & GetPov() const;
		
		Cuboid const & GetCuboid() const;
		Sphere const & GetSphere() const;
		
	private:
		// attributes
		Pov pov;
		
		ObjectMap _objects;	// fast-access container of all objects
		BranchNode _root;	// root of object heirachy
		
		Cuboid const & _cuboid;
		Sphere const & _sphere;
	};
}
