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
	// ForEachIntersection implementation
	
	// general
	template <typename SHAPE, typename FUNCTOR>
	void ForEachIntersection(Polyhedron const & polyhedron, sim::Vector3 const & polyhedron_center, SHAPE const & shape, sim::Vector3 const & origin, FUNCTOR & functor, float min_area)
	{
		IntersectionNodeFunctor<SHAPE, FUNCTOR> node_functor(polyhedron_center, origin, shape, functor, min_area);
		
		form::RootNode const & root_node = polyhedron.root_node;
		form::Node const * children = root_node.GetChildren();
		if (children != nullptr)
		{
			GatherPoints(node_functor, children[0]);
			GatherPoints(node_functor, children[1]);
			GatherPoints(node_functor, children[2]);
			GatherPoints(node_functor, children[3]);
		}
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// ForEachIntersection helper classes and functions
	
	// Scene-relative coordinates describing the 4-sided pyramid
	// made up of the polyhedron center and one of its surface faces.
	struct Pyramid
	{
		Vector3 center;
		Vector3 a, b, c;
	};

	// This class is a hold-all for data needed throughout the intersection process.
	// Doubles as a functor which receives results from the ForEachNodeFace function.
	template <typename SHAPE, typename FUNCTOR>
	class IntersectionNodeFunctor
	{
	public:
		// types
		typedef SHAPE ShapeType;
		
		// functions
		IntersectionNodeFunctor(Vector3 const & polyhedron_center, sim::Vector3 const & origin, ShapeType const & shape, FUNCTOR & functor, Scalar min_area)
		: _origin(origin)
		, _shape(shape)
		, _functor(functor)
		, _min_area(min_area)
		{
			_pyramid.center = polyhedron_center;
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
		Pyramid _pyramid;
		sim::Vector3 const _origin;
		ShapeType const & _shape;
		FUNCTOR & _functor;
		
		// After the node area becomes less than this, recursion ends. 
		// This prevents expensive exhaustive searches.
		Scalar _min_area;
	};


	////////////////////////////////////////////////////////////////////////////////
	// Sphere Intersection support
	
	inline bool TouchesInfinitePyramid(Sphere3 const & sphere, Pyramid const & pyramid) 
	{
		return FastContains(pyramid.center, pyramid.a, pyramid.c, sphere) 
		&& FastContains(pyramid.center, pyramid.b, pyramid.a, sphere) 
		&& FastContains(pyramid.center, pyramid.c, pyramid.b, sphere);
	}
	
	// Get the details of the actual collision between a face and the sphere.
	template <typename INTERSECTION_FUNCTOR>
	inline void TestCollisions(IntersectionNodeFunctor<Sphere3, INTERSECTION_FUNCTOR> const & node_functor,
												Pyramid const & pyramid, 
												Vector3 const & face_norm) 
	{
		Sphere3 const & sphere = node_functor._shape;
		
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
			sim::Vector3 sim_collision_pos = form::SceneToSim<Vector3>(sphere.center, node_functor._origin);
			sim::Vector3 sim_collision_normal = face_norm;
			sim::Scalar sim_collision_depth = sphere.radius - distance;
			
			node_functor._functor(sim_collision_pos, sim_collision_normal, sim_collision_depth);
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
			sim::Vector3 sim_collision_pos = form::SceneToSim<Vector3>(sphere.center + Vector3(face_norm) * (intersection_depth - sphere.radius), node_functor._origin);
			sim::Vector3 sim_collision_normal = face_norm;
			sim::Scalar sim_collision_depth = intersection_depth;
			
			node_functor._functor(sim_collision_pos, sim_collision_normal, sim_collision_depth);
			return;
		}
	}


	////////////////////////////////////////////////////////////////////////////////
	// Point-Cloud Intersection support
	
	// cheap'n'dirty way of representing a convex hull
	struct PointCloud
	{
		// types
		typedef std::vector<Vector3> Vector;
		
		// variables
		Sphere3 sphere;	// bounding sphere
		Vector points;	// point cloud
	};
	
	inline bool TouchesInfinitePyramid(PointCloud const & shape, Pyramid const & pyramid) 
	{
		return FastContains(pyramid.center, pyramid.a, pyramid.c, shape.sphere) 
		&& FastContains(pyramid.center, pyramid.b, pyramid.a, shape.sphere) 
		&& FastContains(pyramid.center, pyramid.c, pyramid.b, shape.sphere);
	}
	
	inline bool TouchesInfinitePyramid(Vector3 const & point, Pyramid const & pyramid) 
	{
		return FastContains(pyramid.center, pyramid.a, pyramid.c, point) 
		&& FastContains(pyramid.center, pyramid.b, pyramid.a, point) 
		&& FastContains(pyramid.center, pyramid.c, pyramid.b, point);
	}
	
	// Get the details of the actual collision between a face and a point cloud.
	template <typename INTERSECTION_FUNCTOR>
	inline void TestCollisions(IntersectionNodeFunctor<PointCloud, INTERSECTION_FUNCTOR> const & node_functor,
								Pyramid const & pyramid, 
								Vector3 const & face_norm) 
	{
		PointCloud const & shape = node_functor._shape;
		
		// the distance from the center of the sphere to the surface. 
		Scalar sphere_distance = DistanceToSurface(pyramid.a, face_norm, shape.sphere.center);
		
		if (sphere_distance > shape.sphere.radius)
		{
			// shape is clear. 
			return;
		}
		
		// One last check to see if we're 'in the zone' becuase the given triangle
		// might have been put together by ForEachNodeFace.
		if (! TouchesInfinitePyramid(shape.sphere, pyramid))
		{
			return;
		}
		
		PointCloud::Vector::const_iterator end = shape.points.end();
		for (PointCloud::Vector::const_iterator it = shape.points.begin(); it != end; ++ it)
		{
			Vector3 const & point = * it;
			Scalar distance = DistanceToSurface(pyramid.a, face_norm, point);
			if (distance < 0)
			{
				if (TouchesInfinitePyramid(point, pyramid))
				{
					sim::Vector3 sim_collision_pos = form::SceneToSim<Vector3>(point - face_norm * distance, node_functor._origin);
					sim::Vector3 sim_collision_normal = face_norm;
					sim::Scalar sim_collision_depth = - distance;
					
					node_functor._functor(sim_collision_pos, sim_collision_normal, sim_collision_depth);
				}
			}
		}
	}
	

	////////////////////////////////////////////////////////////////////////////////
	// Recursive node tree search 
	
	template <typename NODE_FUNCTOR>
	void GatherPoints (NODE_FUNCTOR & node_functor, Node const & node) 
	{
		node_functor._pyramid.a = node.GetCorner(0).pos;
		node_functor._pyramid.b = node.GetCorner(1).pos;
		node_functor._pyramid.c = node.GetCorner(2).pos;
		
		// TODO: The same borders will be being tested many times. Should be able to cut down on them quite a bit.
		if (TouchesInfinitePyramid(node_functor._shape, node_functor._pyramid))
		{
			// If we're as far into the tree as we're going to get,
			// TODO: Return to pruning based on area.
			if (node.area < node_functor._min_area)
			{
				// process node as a single face.
				TestCollisions(node_functor, node_functor._pyramid, node.normal);
				return;
			}

			// We're at a leaf node and haven't reached the min score,
			form::Node const * const children = node.GetChildren();
			if (children == nullptr)
			{
				// try and go a bit deeper by calling ForEachNodeFace.
				ForEachNodeFace(node, node_functor);
				return;
			}

			// Keep recurring into the tree.
			GatherPoints(node_functor, children[0]);
			GatherPoints(node_functor, children[1]);
			GatherPoints(node_functor, children[2]);
			GatherPoints(node_functor, children[3]);
		}
	}
	
}
