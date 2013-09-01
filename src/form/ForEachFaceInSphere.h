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

	template <typename FUNCTOR>
	void ForEachFaceInSphere(Polyhedron const &, Sphere3 const &, FUNCTOR);

	////////////////////////////////////////////////////////////////////////////////
	// helper classes and functions

	typedef std::array<Vector3, 3> Triangle;

	bool TouchesProjection(Vector3 const & pyramid_tip, Triangle const & pyramid_base, Sphere3 const & sphere)
	{
		return FastContains(pyramid_tip, pyramid_base[0], pyramid_base[2], sphere) 
		&& FastContains(pyramid_tip, pyramid_base[1], pyramid_base[0], sphere) 
		&& FastContains(pyramid_tip, pyramid_base[2], pyramid_base[1], sphere);
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

		void operator() (Node const & node, Triangle const & surface) const
		{
			// should hold, but doesn't
			//ASSERT(TouchesProjection(_polyhedron_center, surface, _sphere));
			ASSERT(surface[0] == node.GetCorner(0).pos);
			ASSERT(surface[1] == node.GetCorner(1).pos);
			ASSERT(surface[2] == node.GetCorner(2).pos);

			Node const * children = node.GetChildren();
			if (children == nullptr)
			{
				ForEachNodeFace(node, _poly_functor);
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

			Triangle child_surface;

			int center_counter = 0;

			// For each sub-dividing line that can be drawn between node mid-points,
			auto test_sub_division = [&] (int sub_division_index)
			{
				ASSERT(surface[sub_division_index] == node.GetCorner(sub_division_index).pos);

				auto sub_division_index1 = TriMod(sub_division_index + 1);
				auto sub_division_index2 = TriMod(sub_division_index + 2);

				Vector3 const & b = mid_points_pos[sub_division_index2];
				Vector3 const & c = mid_points_pos[sub_division_index1];

				// Test upon which side of the line the given sphere lies.
				Scalar d = FastDistanceToSurface(_polyhedron_center, b, c, _sphere.center);
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
					child_surface[sub_division_index] = surface[sub_division_index];
					child_surface[sub_division_index1] = b;
					child_surface[sub_division_index2] = c;

					(* this)(children[sub_division_index], child_surface);
				}
			};

			test_sub_division(0);
			test_sub_division(1);
			test_sub_division(2);

			if (center_counter == 3)
			{
				child_surface[0] = mid_points_pos[0];
				child_surface[1] = mid_points_pos[1];
				child_surface[2] = mid_points_pos[2];

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

		RootNode const & root_node = polyhedron.GetRootNode();
		if (! ForEachChildNode(root_node, [&] (Node const & child)
		{
			Triangle surface;
			surface[0] = child.GetCorner(0).pos;
			surface[1] = child.GetCorner(1).pos;
			surface[2] = child.GetCorner(2).pos;

			// Slightly inefficient as the same sides have their distance calculated multiple times.
			if (TouchesProjection(polyhedron_center, surface, sphere))
			{
				node_functor(child, surface);
			}
		}))
		{
			DEBUG_MESSAGE("Need to deal with tetrahedron case");
		}
	}
}