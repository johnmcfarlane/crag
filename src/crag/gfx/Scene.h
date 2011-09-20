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
		
		bool Empty() const;
		
		void AddObject(Object & object);
		void RemoveObject(Object & object);
		
		ObjectSet & GetObjects(Layer::type layer);
		ObjectSet const & GetObjects(Layer::type layer) const;
		
		void SetResolution(Vector2i const & r);
		void SetCamera(sim::Vector3 const & pos, sim::Matrix4 const & rot);
		
		Pov & GetPov();
		Pov const & GetPov() const;
		
		Cuboid const & GetCuboid() const;
		Sphere const & GetSphere() const;
		
		void GetRenderRange(sim::Ray3 const & camera_ray, double & range_min, double & range_max, bool wireframe) const;
		
	private:
		// attributes
		Pov pov;
		
		ObjectSet _objects[Layer::num];
		Cuboid const & _cuboid;
		Sphere const & _sphere;
	};
}
