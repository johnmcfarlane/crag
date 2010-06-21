/*
 *  Planet.h
 *  Crag
 *
 *  Created by john on 4/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Entity.h"

#include "form/Formation.h"

#include "glpp/Texture.h"


namespace physics
{
}


namespace sim
{
	class PlanetaryBody;

	class Planet : public Entity
	{
	public:
		Planet(sim::Vector3 const & init_pos, float init_radius, int init_seed);
		~Planet();

		bool IsShadowCatcher() const;
	
		//void Tick();
		//void Draw() const;
		void GetGravitationalForce(sim::Vector3 const & pos, sim::Vector3 & gravity) const;
	
		Scalar GetBoundingRadius() const;
		sim::Vector3 const & GetPosition() const;
	
		//bool CustomCollision(PhysicalBody & that_body) const;

		//DUMP_OPERATOR_DECLARATION(Entity);

	private:
		form::Formation formation;
		sim::PlanetaryBody * body;
		gl::TextureRgba8 shadow_map;
	};
}
