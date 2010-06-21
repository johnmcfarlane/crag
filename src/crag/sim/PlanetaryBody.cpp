/*
 *  FormationBody.cpp
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "PlanetaryBody.h"


sim::PlanetaryBody::PlanetaryBody(sim::Entity & entity, dBodyID body_id, dGeomID geom_id)
: SphericalBody(entity, body_id, geom_id)
{
}

bool sim::PlanetaryBody::OnCollision(Body & that_body)
{
	// Rely on other body being a sphere and calling this->OnCollisionWithSphericalBody.
	return false;
}

bool sim::PlanetaryBody::OnCollisionWithSphericalBody(SphericalBody & that_sphere)
{
	return SphericalBody::OnCollisionWithSphericalBody(that_sphere);
}




#if 0
class CollideSphereFunctor
{
public:
	CollideSphereFunctor(Mesh & init_mesh, Sphere3f const & init_sphere) 
	: mesh(init_mesh) 
	, sphere(init_sphere)
	, min_poly_area(init_sphere.Area() * 0.01f)
	{ 
	}
	
	bool IsInsideSurface(Vector3f const & j, Vector3f const & k, Vector3f const & l) const
	{
		Vector3f normal = TriangleNormal(j, k, l);
		Normalize(normal);
		return DotProduct(normal, sphere.center - k) < sphere.radius;
	}
	
	bool CanTraverse(FormationNode const & node) const
	{ 
		Vector3f center = Vector3f::Origin();
		Vector3f const & a = node.GetCorner(0).pos;
		Vector3f const & b = node.GetCorner(1).pos;
		Vector3f const & c = node.GetCorner(2).pos;
		
		if ( ! (IsInsideSurface(center, a, b) 
				&& IsInsideSurface(center, b, c) 
				&& IsInsideSurface(center, c, a)))
		{
			return false;
		}
		
		return true;
	}
	
	bool CanElaborate(FormationNode const & node) const
	{
		Vector3f const & a = node.GetCorner(0).pos;
		Vector3f const & b = node.GetCorner(1).pos;
		Vector3f const & c = node.GetCorner(2).pos;
		
		return TriangleArea(a, b, c) >= min_poly_area;
	}
	
	void operator()(Vertex & a, Vertex & b, Vertex & c, Vector3f const & normal)
	{
		mesh.AddFace(a, b, c, normal);
	}
	
private:
	Mesh & mesh;
	Sphere3f const & sphere;
	float min_poly_area;
};
#endif
