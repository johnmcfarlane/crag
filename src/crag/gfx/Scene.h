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

#include "Pov.h"

#include "sim/defs.h"


namespace sim
{
	class Entity;
}


namespace gfx
{
	class Light;
	class Skybox;
	class Sphere;


	class Scene
	{
		friend class Renderer;
		
	public:
		Scene();
		~Scene();
		
		void AddLight(Light const & light);
		
		void AddEntity(sim::Entity const & entity);
		void RemoveEntity(sim::Entity const & entity);
		
		void SetResolution(Vector2i const & r);
		void SetCamera(sim::Vector3 const & pos, sim::Matrix4 const & rot);
		
		Pov & GetPov();
		Pov const & GetPov() const;
				
		void SetSkybox(Skybox const * s) { skybox = s; }
		Skybox const * GetSkybox() const { return skybox; }
		
		Sphere const & GetSphere() const;

		void GetRenderRange(sim::Ray3 const & camera_ray, double & range_min, double & range_max, bool wireframe) const;
		
	private:
		// types
		typedef std::vector<Light const *> LightVector;
		typedef std::vector<sim::Entity const *> EntityVector;
		
		// attributes
		Pov pov;

		LightVector lights;
		EntityVector entities;
		Skybox const * skybox;
		Sphere const & _sphere;
	};
}
