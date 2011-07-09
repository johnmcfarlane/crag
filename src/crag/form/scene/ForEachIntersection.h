//
//  ForEachIntersection.h
//  crag
//
//  Created by John McFarlane on 6/11/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "form/scene/Polyhedron.h"
#include "form/node/ForEachNodeFace.h"

#include "geom/SphereOps.h"


namespace form
{
	////////////////////////////////////////////////////////////////////////////////
	// ForEachIntersection functions
	//
	// For the given polyhedron, calls functor for all faces which intersect with shape.

	// forward-declaration
	template <typename SHAPE, typename FUNCTOR>
	void ForEachIntersection(Polyhedron const & polyhedron, Vector3 const & polyhedron_center, SHAPE const & shape, FUNCTOR const & functor);
	
	
	////////////////////////////////////////////////////////////////////////////////
	// ForEachIntersection helpers
	
	// Scene-relative coordinates describing the 4-sided pyramid
	// made up of the polyhedron center and the surface face.
	struct Pyramid
	{
		Vector3 center;
		Vector3 a, b, c;
	};

	bool TouchesInfinitePyramid(Sphere<Scalar, 3> const & sphere, Pyramid const & pyramid) 
	{
		return Contains(pyramid.center, pyramid.a, pyramid.c, sphere) 
			&& Contains(pyramid.center, pyramid.b, pyramid.a, sphere) 
			&& Contains(pyramid.center, pyramid.c, pyramid.b, sphere);
	}
	
	// Get the details of the actual collision between a face and the sphere.
	bool GetCollision(Sphere<Scalar, 3> const & sphere, 
					  Pyramid const & pyramid, Vector3 const & face_norm, 
					  Vector3 & collision_pos, Vector3 & collision_normal, Scalar & collision_depth) 
	{
		// the distance from the center of the sphere to the surface. 
		Scalar distance = DistanceToSurface/*<Scalar>*/(pyramid.a, pyramid.b, pyramid.c, sphere.center);

		if (distance > sphere.radius)
		{
			// we're good. 
			return false;
		}
		
		// One last check to see if we're 'in the zone' becuase the given triangle
		// might have been put together by ForEachNodeFace.
		if (! TouchesInfinitePyramid(sphere, pyramid))
		{
			return false;
		}
		
		// The case where the sphere is completely within the surface. 
		if (distance < - sphere.radius)
		{
			Vector3 tri_center = (pyramid.a + pyramid.b + pyramid.c) / 3.f;
			Vector3 to_tri = tri_center - pyramid.center;
			Vector3 to_sphere = sphere.center - pyramid.center;
			Scalar dp = DotProduct(Normalized(to_tri), Normalized(to_sphere));
			if (dp < 0)
			{
				// yikes! colliding with a face on the opposite side of the formation
				return false;
			}
			
			// TODO: Is the normal passed to AddFace negative? If so, lighting's still wrong. 
			// Note: collision_pos is in global coordinates
			collision_pos = sphere.center;
			collision_normal = face_norm;
			collision_depth = sphere.radius - distance;
			
			return true;
		}
		
		// Proper contact between triangle (not plane) and sphere.
		Scalar intersection_depth;
		if (Intersects(sphere, pyramid.c, pyramid.b, pyramid.a, & intersection_depth))
		{
			//intersection_depth = distance;
			// TODO: Triangle-line intersection. (Might already have this somewhere.)
			// because sphere.center is the wrong position!
			// Note: collision_pos is in global coordinates
			collision_pos = sphere.center + Vector3(face_norm) * (intersection_depth - sphere.radius);
			collision_normal = face_norm;
			collision_depth = intersection_depth;
			
			return true;
		}
		
		return false;
	}
	

	// This class holds all the data necessary to test the shape for intersection 
	// against nodes, and mines down through the nodes, calling the functor on them. 
	template <typename SHAPE, typename COLLISION_FUNCTOR>
	class Traveler
	{
	public:
		Traveler(Vector3 const & polyhedron_center, SHAPE const & shape, sim::Vector3 const & origin, COLLISION_FUNCTOR & functor, Scalar min_length)
		: _shape(shape)
		, _origin(origin)
		, _functor(functor)
		, _min_length_squared(Square(min_length))
		{
			_pyramid.center = polyhedron_center;
		}
		
		void GatherPoints(Node const & node) 
		{
			_pyramid.a = node.GetCorner(0).pos;
			_pyramid.b = node.GetCorner(1).pos;
			_pyramid.c = node.GetCorner(2).pos;
			
			// TODO: The same borders will be being tested many times. Should be able to cut down on them quite a bit.
			if (TouchesInfinitePyramid(_shape, _pyramid))
			{
				form::Node const * const children = node.GetChildren();
				if (children == nullptr)
				{
					// We're at a leaf node; process faces.
					ForEachNodeFace(node, * this);
				}
				else if (LengthSq(_pyramid.a - _pyramid.b) < _min_length_squared)
				{
					// We've gone too deep; process faces.
					ForEachNodeFace(node, * this);
				}
				else
				{
					// Keep recurring into the tree.
					form::Node const * child = children;
					form::Node const * const children_end = child + 4;
					do
					{
						GatherPoints(* child);
						++ child;
					}	while (child != children_end);
				}
			}
		}
		
		// the ForEachNodeFace callback
		void operator()(form::Point const & a, form::Point const & b, form::Point const & c, form::Vector3 const & normal)
		{
			_pyramid.a = a.pos;
			_pyramid.b = b.pos;
			_pyramid.c = c.pos;
			
			Vector3 collision_pos, collision_normal;
			Scalar collision_depth;
			if (GetCollision(_shape, _pyramid, normal, collision_pos, collision_normal, collision_depth))
			{
				sim::Vector3 sim_collision_pos = form::SceneToSim(collision_pos, _origin);
				sim::Vector3 sim_collision_normal = collision_normal;
				sim::Scalar sim_collision_depth = collision_depth;
				_functor(sim_collision_pos, sim_collision_normal, sim_collision_depth);
			}
		}
		
	private:
		// data
		Vector3 const _origin;
		Pyramid _pyramid;
		SHAPE const & _shape;
		COLLISION_FUNCTOR & _functor;
		
		// After an arbitrarily chosen triangle edge becomes shorter than this,
		// recursion ends. This prevents expensive, exhaustive searches.
		Scalar _min_length_squared;
	};

	
	////////////////////////////////////////////////////////////////////////////////
	// ForEachIntersection implementation
	
	// general
	template <typename SHAPE, typename COLLISION_FUNCTOR>
	void ForEachIntersection(Polyhedron const & polyhedron, sim::Vector3 const & polyhedron_center, SHAPE const & shape, sim::Vector3 const & origin, COLLISION_FUNCTOR & functor, float min_length)
	{
		Traveler<SHAPE, COLLISION_FUNCTOR> traveler(polyhedron_center, shape, origin, functor, min_length);
		
		form::RootNode const & root_node = polyhedron.root_node;
		form::Node const * children = root_node.GetChildren();
		if (children != nullptr)
		{
			traveler.GatherPoints(children[0]);
			traveler.GatherPoints(children[1]);
			traveler.GatherPoints(children[2]);
			traveler.GatherPoints(children[3]);
		}
	}

}
