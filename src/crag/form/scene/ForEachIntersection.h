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

#include "geom/Intersection.h"


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
	
	inline bool TouchesInfinitePyramid(Sphere3 const & sphere, Pyramid const & pyramid) 
	{
		return Contains(pyramid.center, pyramid.a, pyramid.c, sphere) 
		&& Contains(pyramid.center, pyramid.b, pyramid.a, sphere) 
		&& Contains(pyramid.center, pyramid.c, pyramid.b, sphere);
	}
	
	template <typename SHAPE, typename FUNCTOR> class Traveler;
	
	// Get the details of the actual collision between a face and the sphere.
	template <typename SHAPE, typename FUNCTOR>
	void TestCollisions(Traveler<SHAPE, FUNCTOR> const & traveler,
						Pyramid const & pyramid, 
						Vector3 const & face_norm);
	
	// This class holds all the data necessary to test the shape for intersection 
	// against nodes, and mines down through the nodes, calling the functor on them. 
	template <typename SHAPE, typename FUNCTOR>
	class Traveler
	{
	public:
		// types
		typedef SHAPE ShapeType;
		
		// functions
		Traveler(Vector3 const & polyhedron_center, ShapeType const & shape, sim::Vector3 const & origin, FUNCTOR & functor, Scalar min_score)
		: _shape(shape)
		, _origin(origin)
		, _functor(functor)
		, _min_score(min_score)
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
				if (children == nullptr || node.score < _min_score)
				{
					// We're at a leaf node or one which is too insignificant; process faces.
					ForEachNodeFace(node, * this);
				}
				else
				{
					// Keep recurring into the tree.
					GatherPoints(children[0]);
					GatherPoints(children[1]);
					GatherPoints(children[2]);
					GatherPoints(children[3]);
				}
			}
		}
		
		// the ForEachNodeFace callback
		void operator()(form::Point const & a, form::Point const & b, form::Point const & c, form::Vector3 const & normal)
		{
			_pyramid.a = a.pos;
			_pyramid.b = b.pos;
			_pyramid.c = c.pos;
			
			TestCollisions(* this, _pyramid, normal);
		}
		
		// data
		sim::Vector3 const _origin;
		Pyramid _pyramid;
		ShapeType const & _shape;
		FUNCTOR & _functor;
		
		// After an arbitrarily chosen triangle edge becomes shorter than this,
		// recursion ends. This prevents expensive, exhaustive searches.
		Scalar _min_score;
	};
	
	// Get the details of the actual collision between a face and the sphere.
	template <typename FUNCTOR>
	inline void TestCollisions<Sphere, FUNCTOR>(Traveler<Sphere3, FUNCTOR> const & traveler,
												Pyramid const & pyramid, 
												Vector3 const & face_norm) 
	{
		Sphere3 const & sphere = traveler._shape;
		
		// the distance from the center of the sphere to the surface. 
		Scalar distance = DistanceToSurface(pyramid.a, face_norm, sphere.center);
		
		if (distance > sphere.radius)
		{
			// we're good. 
			return;
		}
		
		// One last check to see if we're 'in the zone' becuase the given triangle
		// might have been put together by ForEachNodeFace.
		if (! TouchesInfinitePyramid(sphere, pyramid))
		{
			return;
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
				return;
			}
			
			// Note: collision_pos is in global coordinates
			sim::Vector3 sim_collision_pos = form::SceneToSim<Vector3>(sphere.center, traveler._origin);
			sim::Vector3 sim_collision_normal = face_norm;
			sim::Scalar sim_collision_depth = sphere.radius - distance;
			
			traveler._functor(sim_collision_pos, sim_collision_normal, sim_collision_depth);
			return;
		}
		
		// Proper contact between triangle (not plane) and sphere.
		Scalar intersection_depth;
		if (GetIntersection(sphere, pyramid.c, pyramid.b, pyramid.a, & intersection_depth))
		{
			//intersection_depth = distance;
			// TODO: Triangle-line intersection. (Might already have this somewhere.)
			// because sphere.center is the wrong position!
			// Note: collision_pos is in global coordinates
			sim::Vector3 sim_collision_pos = form::SceneToSim<Vector3>(sphere.center + Vector3(face_norm) * (intersection_depth - sphere.radius), traveler._origin);
			sim::Vector3 sim_collision_normal = face_norm;
			sim::Scalar sim_collision_depth = intersection_depth;
			
			traveler._functor(sim_collision_pos, sim_collision_normal, sim_collision_depth);
			return;
		}
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// ForEachIntersection implementation
	
	// general
	template <typename SHAPE, typename FUNCTOR>
	void ForEachIntersection(Polyhedron const & polyhedron, sim::Vector3 const & polyhedron_center, SHAPE const & shape, sim::Vector3 const & origin, FUNCTOR & functor, float min_score)
	{
		Traveler<SHAPE, FUNCTOR> traveler(polyhedron_center, shape, origin, functor, min_score);
		
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
