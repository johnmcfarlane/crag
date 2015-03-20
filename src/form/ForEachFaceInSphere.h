//
//  form/ForEachFaceInSphere.h
//  crag
//
//  Created by John McFarlane on 2013-08-29
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Polyhedron.h"
#include "ForEachNodeFace.h"
#include "ForEachChildNode.h"

#include "geom/Intersection.h"

namespace form 
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declaration

	// calls functor on any faces which are close enough to sphere to consider collision
	template <typename FUNCTOR>
	void ForEachFaceInSphere(Polyhedron const &, Sphere3 const &, FUNCTOR);

	////////////////////////////////////////////////////////////////////////////////
	// helper classes and functions

	bool TouchesProjection(Vector3 const & pyramid_tip, Triangle3 const & pyramid_base, Sphere3 const & sphere)
	{
		// checks consistent winding of pyramid_base
		ASSERT(geom::Distance(pyramid_base, pyramid_tip) < 0);
		
		return Intersects(Plane3(Triangle3(pyramid_tip, pyramid_base.points[0], pyramid_base.points[2])), sphere) 
		&& Intersects(Plane3(Triangle3(pyramid_tip, pyramid_base.points[1], pyramid_base.points[0])), sphere) 
		&& Intersects(Plane3(Triangle3(pyramid_tip, pyramid_base.points[2], pyramid_base.points[1])), sphere);
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// ForEachFaceInSphere implementation

	template <typename POLY_FUNCTOR>
	class ForEachFaceInSphereFunctor
	{
		typedef POLY_FUNCTOR PolyFunctor;
	public:
		ForEachFaceInSphereFunctor(Sphere3 const & sphere, Vector3 const & polyhedron_center, PolyFunctor poly_functor)
		: _sphere(sphere)
		, _polyhedron_center(polyhedron_center)
		, _poly_functor(poly_functor)
		{
		}

		void operator() (Node const & node, Triangle3 const & surface) const
		{
			ASSERT(surface.points[0] == node.GetCorner(0)->pos);
			ASSERT(surface.points[1] == node.GetCorner(1)->pos);
			ASSERT(surface.points[2] == node.GetCorner(2)->pos);

			Node const * children = node.GetChildren();
			if (children == nullptr)
			{
				ForEachNodeFace(node, [this] (Point const & a, Point const & b, Point const & c, geom::Vector3f const & normal, float)
				{
					Triangle3 face(a.pos, b.pos, c.pos);
					CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(geom::Normalized(geom::Normal(face)) - normal), 0.f, .01f);
					
					_poly_functor(face, normal);
				});
				return;
			}

			Point const * mid_points[3] = 
			{
				node.GetMidPoint(0),
				node.GetMidPoint(1),
				node.GetMidPoint(2)
			};

			Vector3 mid_points_pos[3] = 
			{
				mid_points[0]->pos,
				mid_points[1]->pos,
				mid_points[2]->pos
			};

			Triangle3 child_surface;

			int center_counter = 0;

			// For each sub-dividing line that can be drawn between node mid-points,
			auto test_sub_division = [&] (int sub_division_index)
			{
				ASSERT(surface.points[sub_division_index] == node.GetCorner(sub_division_index)->pos);

				auto sub_division_index1 = TriMod(sub_division_index + 1);
				auto sub_division_index2 = TriMod(sub_division_index + 2);

				Vector3 const & b = mid_points_pos[sub_division_index2];
				Vector3 const & c = mid_points_pos[sub_division_index1];

				// Test upon which side of the line the given sphere lies.
				auto sphere_center = _sphere.center;
				Scalar d = Distance(Triangle3(_polyhedron_center, b, c), sphere_center);
				Scalar r = _sphere.radius;
				if (d <= - r)
				{
					// The shape lies within the line (closer to the node center).

					// Increment the center_counter; we're within one side of center child.
					++ center_counter;
				}
				else
				{
					if (d <= r)
					{
						// The shape straddles the center child triangle and this outer triangle.

						// Flag that we're touching this side.
						++ center_counter;
					}

					// Recur for this outer child. 
					child_surface.points[sub_division_index] = surface.points[sub_division_index];
					child_surface.points[sub_division_index1] = b;
					child_surface.points[sub_division_index2] = c;

					(* this)(children[sub_division_index], child_surface);
				}
			};

			test_sub_division(0);
			test_sub_division(1);
			test_sub_division(2);

			if (center_counter == 3)
			{
				child_surface.points[0] = mid_points_pos[0];
				child_surface.points[1] = mid_points_pos[1];
				child_surface.points[2] = mid_points_pos[2];

				(* this)(children[3], child_surface);
			}
		}

	private:
		Sphere3 _sphere;
		Vector3 _polyhedron_center;
		PolyFunctor _poly_functor;
	};

	template <typename POLY_FUNCTOR>
	void ForEachFaceInSphere(Polyhedron const & polyhedron, Sphere3 const & sphere, POLY_FUNCTOR poly_functor)
	{
		typedef ForEachFaceInSphereFunctor<POLY_FUNCTOR> ForEachFaceInSphereFunctor;

		Vector3 const & polyhedron_center = geom::Cast<Scalar>(polyhedron.GetShape().center);
		ForEachFaceInSphereFunctor node_functor(sphere, polyhedron_center, poly_functor);

		Node const & root_node = polyhedron.GetRootNode();
		if (! ForEachChildNode(root_node, [&] (Node const & child)
		{
			Triangle3 surface;
			surface.points[0] = child.GetCorner(0)->pos;
			surface.points[1] = child.GetCorner(1)->pos;
			surface.points[2] = child.GetCorner(2)->pos;

			// Slightly inefficient as the same sides have their distance calculated multiple times.
			if (TouchesProjection(polyhedron_center, surface, sphere))
			{
				node_functor(child, surface);
			}
		}))
		{
			// if root node has no children, treat it as a tetrahedron
			auto f = [& polyhedron_center, & sphere, & poly_functor] (Point const & a, Point const & b, Point const & c)
			{
				Triangle3 surface;
				surface.points[0] = a.pos;
				surface.points[1] = b.pos;
				surface.points[2] = c.pos;
				
				if (TouchesProjection(polyhedron_center, surface, sphere))
				{
					Vector3 normal = geom::Normalized(geom::Normal(surface));
					poly_functor(surface, normal);
				}
			};
			
			f(* root_node.GetCorner(0), * root_node.GetMidPoint(2), * root_node.GetMidPoint(1));
			f(* root_node.GetCorner(1), * root_node.GetMidPoint(0), * root_node.GetMidPoint(2));
			f(* root_node.GetCorner(2), * root_node.GetMidPoint(1), * root_node.GetMidPoint(0));
			f(* root_node.GetMidPoint(0), * root_node.GetMidPoint(1), * root_node.GetMidPoint(2));
		}
	}
}
