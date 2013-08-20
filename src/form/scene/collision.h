//
//  collision.h
//  crag
//
//  Created by John McFarlane on 2011-06-11
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Polyhedron.h"

#include "geom/Intersection.h"


#if ! defined(NDEBUG)
//#define DEBUG_FEI
#endif

#if defined(DEBUG_FEI)
#include "gfx/Debug.h"
#define DEBUG_FEI_RAY(NODE_FUNCTOR, RAY, COLOR) \
	DO_STATEMENT( \
		sim::Vector3 debug_fei_start = geom::RelToAbs(RAY.position, NODE_FUNCTOR._origin); \
		sim::Vector3 debug_fei_end = debug_fei_start + sim::Vector3(RAY.direction); \
		gfx::Debug::AddLine(debug_fei_start, debug_fei_end, COLOR); \
	)
#define DEBUG_FEI_TRI(NODE_FUNCTOR, A, B, C, NORM, COLOR) \
	DO_STATEMENT( \
		sim::Vector3 debug_fei_start = geom::RelToAbs((A + B + C) * (1.f / 3.f), NODE_FUNCTOR._origin); \
		sim::Vector3 debug_fei_end = debug_fei_start + sim::Vector3(NORM); \
		gfx::Debug::AddLine(debug_fei_start, debug_fei_end, COLOR); \
	)
#else
#define DEBUG_FEI_RAY(NODE_FUNCTOR, RAY, COLOR) DO_NOTHING
#define DEBUG_FEI_TRI(NODE_FUNCTOR, A, B, C, NORM, COLOR) DO_NOTHING
#endif


namespace form { namespace collision 
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations

	template <typename Shape>
	class Object;

	template <typename SHAPE, typename FUNCTOR>
	void ForEachCollision(Polyhedron const & polyhedron, Vector3 const & polyhedron_center, Object<SHAPE> const & object, FUNCTOR & functor);
	
	
	////////////////////////////////////////////////////////////////////////////////
	// helper classes and functions
	
	// Scene-relative coordinates describing the 4-sided pyramid
	// made up of the polyhedron center and one of its surface faces.
	class Pyramid
	{
	public:
		// functions
		Pyramid(Vector3 const & c) : center(c) { }

		// Returns true of the given object touches the infinite pyramid projected
		// out from the center through the surface.
		bool TouchesProjection(Vector3 const & point) const
		{
			return FastContains(center, surface[0], surface[2], point) 
			&& FastContains(center, surface[1], surface[0], point) 
			&& FastContains(center, surface[2], surface[1], point);
		}
		
		bool TouchesProjection(Sphere3 const & sphere) const
		{
			return FastContains(center, surface[0], surface[2], sphere) 
			&& FastContains(center, surface[1], surface[0], sphere) 
			&& FastContains(center, surface[2], surface[1], sphere);
		}

		// variables
		Vector3 center;
		Vector3 surface[3];
	};
	
	// Object - geometry and a bounding sphere
	template <typename Shape>
	class Object
	{
	public:
		Sphere3 bounding_sphere;
		Shape shape;
	};
	
	// variables used during the calculation of collision which are handy to pass around
	struct CollisionInfo
	{
		Ray3 ray;	// unit ray from surface point outward
		Scalar t1, t2;	// entry/exit point along ray
		Scalar depth;	// computed from t1 & t2
	};
	
	// This class is a hold-all for data needed throughout the collision detection process.
	// It doubles as a functor which receives results from the ForEachNodeFace function.
	template <typename SHAPE, typename FUNCTOR>
	class CollisionFunctor
	{
	public:
		// types
		typedef SHAPE ShapeType;
		typedef ::form::collision::Object<ShapeType> Object;
		
		// functions
		CollisionFunctor(Vector3 const & polyhedron_center, Object const & object, FUNCTOR & functor)
		: _pyramid(polyhedron_center)
		, _object(object)
		, _functor(functor)
		{
		}
		
		// data
		Pyramid _pyramid;	// contains the center of the formation etc.
		Object const & _object;	// the object with which to collide the formation
		FUNCTOR & _functor;	// thing to call when collision is detected
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Sphere Collision support
	
	template <typename COLLISION_FUNCTOR>
	inline bool TestShapeCollision(CollisionFunctor<Ray3, COLLISION_FUNCTOR> const &, CollisionInfo const &)
	{
		ASSERT(false);
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Sphere Collision support
	
	template <typename COLLISION_FUNCTOR>
	inline bool TestShapeCollision(CollisionFunctor<Sphere3, COLLISION_FUNCTOR> const & node_functor, CollisionInfo const & collision_info)
	{
		// In the case of Sphere3, where the bounding_sphere IS the shape, shape becomes a placeholder.
		ASSERT(node_functor._object.bounding_sphere == node_functor._object.shape);

		Vector3 contact_point = Project(collision_info.ray, collision_info.t1);
		Vector3 contact_normal = node_functor._object.bounding_sphere.center - contact_point;
		Scalar contact_normal_distance_sq = LengthSq(contact_normal);
		if (contact_normal_distance_sq > 0)
		{
			contact_normal *= InvSqrt(contact_normal_distance_sq);
		}
		else
		{
			contact_normal = collision_info.ray.direction;
		}
		node_functor._functor(contact_point, contact_normal, collision_info.depth);
		DEBUG_FEI_RAY(node_functor, collision_info.ray, gfx::Color4f::White());
		return true;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Convex Hull Collision support
	
	// Convex hull represented by a collection of half-space planes.
	struct ConvexHull
	{
		// types
		typedef Ray3 Face;
		typedef std::array<Face, 6> Vector;
		
		// variables
		Vector faces;	// point cloud
	};
	
	bool GetIntersection(ConvexHull const & shape, Ray3 const & ray, Scalar & t1, Scalar & t2)
	{
		typedef ConvexHull::Vector::const_iterator iterator;
		iterator begin = shape.faces.begin();
		iterator end = shape.faces.end();
		return GetIntersection(begin, end, ray, t1, t2);
	}

	// Get the details of the actual collision between a face and a point cloud.
	template <typename COLLISION_FUNCTOR>
	inline bool TestShapeCollision(CollisionFunctor<ConvexHull, COLLISION_FUNCTOR> const & node_functor, CollisionInfo & collision_info)
	{
		if (! GetIntersection(node_functor._object.shape, collision_info.ray, collision_info.t1, collision_info.t2) || collision_info.t1 >= 0)
		{
			DEBUG_FEI_RAY(node_functor, collision_info.ray, gfx::Color4f::Black());
			return false;
		}
		
		Vector3 contact_point = geom::Project(collision_info.ray, collision_info.t1);
		Vector3 const & contact_normal = collision_info.ray.direction;

		node_functor._functor(contact_point, contact_normal, - collision_info.t1);
		DEBUG_FEI_RAY(node_functor, collision_info.ray, gfx::Color4f::White());
		return true;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Recursive node tree search 
	
	// Tests shape's outer bounding sphere against given surface point.
	template <typename SHAPE, typename COLLISION_FUNCTOR>
	inline bool TestObjectPointCollision(CollisionFunctor<SHAPE, COLLISION_FUNCTOR> const & node_functor, Vector3 const & surface)
	{
		Vector3 center_to_surface = surface - node_functor._pyramid.center;
		
		CollisionInfo collision_info;
		collision_info.ray = Ray3(surface, Normalized(center_to_surface));
		
		if (! GetIntersection(node_functor._object.bounding_sphere, collision_info.ray, collision_info.t1, collision_info.t2))
		{
			DEBUG_FEI_RAY(node_functor, collision_info.ray, gfx::Color4f::Black());
			return false;
		}
		
		if (collision_info.t1 >= 0)
		{
			// The sphere's near distance is positive; it's not in contact.
			DEBUG_FEI_RAY(node_functor, collision_info.ray, gfx::Color4f::Black());
			return false;
		}
		else if (collision_info.t2 >= 0)
		{
			// The sphere is part-way embedded within the surface.
			collision_info.depth = - collision_info.t1;
		}
		else
		{
			// The sphere's far distance is negative; it's embedded.
			Scalar altitude_squared = LengthSq(center_to_surface);
			if (Squared(collision_info.t2) > altitude_squared)
			{
				// The rare case that the object is past the surface AND past
				// the center of the polyhedron as well.
				DEBUG_FEI_RAY(node_functor, collision_info.ray, gfx::Color4f::Black());
				return false;
			}
			
			collision_info.depth = collision_info.t2 - collision_info.t1;
		}
		
		return TestShapeCollision(node_functor, collision_info);
	}
	
	// leaf function
	template <typename NODE_FUNCTOR>
	void ForEachCollision_Leaf (NODE_FUNCTOR & node_functor, Node const & node) 
	{
		// We want to and collide the shape against any known points associated with the node.
		// However, the mid-points are not an efficient choice. They often don't exist and 
		// when they do, they're often shared with neighbours and their children and will be
		// tested against anyway.
		
		// So just test the center point.
		if (TestObjectPointCollision(node_functor, node.center))
		{
			// So long as we gone some collision, we're probably good.
			return;
		}
		
		// If we failed to find a collision there's a risk that the node:shape
		// ratio is so great that the shape will sink down inside the node.
		
		// Find the nearest point on the node's surface to the shape.
		Ray3 line(node_functor._object.bounding_sphere.center, node.normal);
		Scalar t = DotProduct(node.center - line.position, node.normal);
		Vector3 nearest_point = node_functor._object.bounding_sphere.center + node.normal * t;
		
		// Set the three bottom corners of the pyramid.
		node_functor._pyramid.surface[0] = node.GetCorner(0).pos;
		node_functor._pyramid.surface[1] = node.GetCorner(1).pos;
		node_functor._pyramid.surface[2] = node.GetCorner(2).pos;
		if (! node_functor._pyramid.TouchesProjection(nearest_point))
		{
			// That point is not within the confines of the infinite pyramid.
			return;
		}
		
		// Use the nearest point to perform an intersection test.
		TestObjectPointCollision(node_functor, nearest_point);
	}
	
	// mid-level recursive function
	template <typename NODE_FUNCTOR>
	void ForEachCollision (NODE_FUNCTOR & node_functor, Node const & node) 
	{
		Node const * children = node.GetChildren();
		if (children == nullptr)
		{
			ForEachCollision_Leaf(node_functor, node);
			return;
		}
		
		Vector3 const * mid_points[3] = 
		{
			& ref(node.GetMidPoint(0)).pos,
			& ref(node.GetMidPoint(1)).pos,
			& ref(node.GetMidPoint(2)).pos
		};
		
		Vector3 const & a = node_functor._pyramid.center;

		unsigned contact_mask = 0;
		int center_counter = 0;
		
		// For each sub-dividing line that can drawn between node mid-points,
		int sub_division_index = 0;
		while (true)
		{
			Vector3 const & b = * mid_points[TriMod(sub_division_index + 2)];
			Vector3 const & c = * mid_points[TriMod(sub_division_index + 1)];
			
			// Test upon which side of the line the given shape lies.
			Scalar d = FastDistanceToSurface(a, b, c, node_functor._object.bounding_sphere.center);
			Scalar r = node_functor._object.bounding_sphere.radius;
			if (d > r)
			{
				// The shape is clear outside the center child triangle.
				// Recur for this outer child. 
				ForEachCollision(node_functor, children[sub_division_index]);
			}
			else if (d < - r)
			{
				// The shape lies within the line (closer to the node center).
				
				// Increment the center_counter; we're within one side of center child.
				++ center_counter;
			}
			else
			{
				// The shape straddles the center child triangle and this outer one. 
				
				// Increment the center_counter; we're within one side of center child.
				++ center_counter;
				
				// Flag that we're touching this side.
				contact_mask |= (1 << sub_division_index);

				// And recur for the outer child. 
				ForEachCollision(node_functor, children[sub_division_index]);
			}
			
			if (sub_division_index == 2)
			{
				break;
			}
			++ sub_division_index;
		}
		
		if ((contact_mask & 6) == 6)
		{
			TestObjectPointCollision(node_functor, * mid_points[0]);
		}
		if ((contact_mask & 5) == 5)
		{
			TestObjectPointCollision(node_functor, * mid_points[1]);
		}
		if ((contact_mask & 3) == 3)
		{
			TestObjectPointCollision(node_functor, * mid_points[2]);
		}
		
		if (center_counter == 3)
		{
			ForEachCollision(node_functor, children[3]);
		}
	}
	
	// root-level recursive function
	template <typename NODE_FUNCTOR>
	void ForEachCollision (NODE_FUNCTOR & node_functor, RootNode const & root_node) 
	{
		TestObjectPointCollision(node_functor, root_node.GetCorner(0).pos);
		TestObjectPointCollision(node_functor, root_node.GetMidPoint(0)->pos);
		TestObjectPointCollision(node_functor, root_node.GetMidPoint(1)->pos);
		TestObjectPointCollision(node_functor, root_node.GetMidPoint(2)->pos);
		
		Node const * children = root_node.GetChildren();
		if (children != nullptr)
		{
			Node const * children_end = children + 3;
			while (true)
			{
				Node const & node = * children;
				
				node_functor._pyramid.surface[0] = node.GetCorner(0).pos;
				node_functor._pyramid.surface[1] = node.GetCorner(1).pos;
				node_functor._pyramid.surface[2] = node.GetCorner(2).pos;
				
				// Slightly inefficient as the same sides have their distance calculated multiple times.
				if (node_functor._pyramid.TouchesProjection(node_functor._object.bounding_sphere))
				{
					ForEachCollision(node_functor, node);
				}
				
				if (children == children_end)
				{
					break;
				}
				
				++ children;
			}
		}
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// ForEachCollision implementation
	
	template <typename SHAPE, typename FUNCTOR>
	void ForEachCollision(Polyhedron const & polyhedron, Object<SHAPE> const & object, FUNCTOR & functor)
	{
		form::Vector3 const & polyhedron_center = geom::Cast<form::Scalar>(polyhedron.GetShape().center);
		CollisionFunctor<SHAPE, FUNCTOR> node_functor(polyhedron_center, object, functor);
		
		form::RootNode const & root_node = polyhedron.GetRootNode();
		ForEachCollision(node_functor, root_node);
	}

} }
