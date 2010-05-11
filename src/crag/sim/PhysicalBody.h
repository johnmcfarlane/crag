#pragma once

/*
 *  PhysicalBody.h
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "form/Mesh.h"

#include <ode/ode.h>


namespace sim
{
	class PhysicalBody
	{
	public:
		PhysicalBody(class Entity & entity, dGeomID geom);
		~PhysicalBody();

		//void BeginCollisions();
		//void EndCollisions();

		dBodyID const GetBody() const { return body; }
		dGeomID GetGeom() const { return geom; }
		//Mesh & GetFormationMesh() { return formation_mesh; }
		//dGeomID GetFormationGeom() { return formation_geom; }

	private:
		// The physical thing itself.
		dBodyID body;
		dGeomID geom;
	};
	
	
	class PhysicalSphere : public PhysicalBody
	{
	public:
		PhysicalSphere(Entity & entity, float density, float radius);

		float GetRadius() const;
	};
}
