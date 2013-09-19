//
//  form/CastRay.h
//  crag
//
//  Created by John McFarlane on 2013-08-29
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "CastRay.h"

#include "ForEachNodeFace.h"
#include "Polyhedron.h"

#include "geom/Intersection.h"

#if ! defined(NDEBUG)
//#define DEBUG_TRIANGLES
#endif

#if defined(DEBUG_TRIANGLES)
#include "gfx/Debug.h"
#include "core/Random.h"
#endif

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// RayCastResult function definitions

namespace form
{
#if defined(VERIFY)
	void RayCastResult::Verify() const
	{
		VerifyEqual(projection, projection);
		if (projection < std::numeric_limits<Scalar>::max())
		{
			VerifyTrue(node != nullptr);
			VerifyNearlyEqual(geom::Length(normal), 1.f, .001f);
		}
		else
		{
			VerifyTrue(node == nullptr);
		}
	}
#endif

	// the sort criteria applied to RayCastResult when choosing the lowest contact
	bool operator<(RayCastResult const & lhs, RayCastResult const & rhs)
	{
		VerifyObjectRef(lhs);
		VerifyObjectRef(rhs);

		// Being negative means behind the ray. That's worse than being in front.
		int positive_compare = int(lhs.projection >= 0) - int(rhs.projection >= 0);
		if (positive_compare)
		{
			return positive_compare > 0;
		}

		return lhs.projection < rhs.projection;
	}
}

////////////////////////////////////////////////////////////////////////////////
// file-local definitions

namespace
{
	namespace impl
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef ::form::RayCastResult::Scalar Scalar;
		typedef geom::Vector<Scalar, 3> Vector3;
		typedef geom::Ray<Scalar, 3> Ray3;
		typedef geom::Plane<Scalar, 3> Plane3;
		typedef geom::Triangle<Scalar, 3> Triangle3;
		typedef std::array<Scalar, 2> Range;

		// useful info about the collision between the ray 
		// and a side of the triangular pyramid occipied by a Node
		struct SideAttributes
		{
			Scalar intersection;
			Scalar dot_product;
		};

		// useful info about the entire trianglar pyramid which a Node represents
		struct Attributes
		{
			Range range;
			Node const * node;
			Triangle3 surface;
			SideAttributes sides[3];
		};

		// the data which remain invariant across a call to CastRay
		struct Uniforms
		{
			Vector3 center;	// center of the polyhedron lassed to CastRay function
			Ray3 ray;	// ray with normalized direction
			Scalar length;	// pre-normalized direction
		};

		// type of the function given to the ForEachFace function
		typedef RayCastResult ForEachFaceFunction(Uniforms const &, Attributes const &);

		////////////////////////////////////////////////////////////////////////////////
		// functions

		// the sort criteria applied in the ForEachFace function
		bool operator<(Attributes const & lhs, Attributes const & rhs)
		{
			return lhs.range[0] < rhs.range[0];
		}

#if defined(DEBUG_TRIANGLES)
		template <typename S>
		void AddLine(geom::Vector<S, 3> const & a, geom::Vector<S, 3> const & b)
		{
			gfx::Debug::Color random_color(Random::sequence.GetUnit<gfx::Scalar>(), Random::sequence.GetUnit<gfx::Scalar>(), Random::sequence.GetUnit<gfx::Scalar>(), .5f);

			gfx::Debug::AddLine(geom::Cast<gfx::Scalar>(a), geom::Cast<gfx::Scalar>(b), random_color);
		}

		void AddTriangle(Triangle3 const & triangle)
		{
			gfx::Debug::Color random_color(Random::sequence.GetUnit<gfx::Scalar>(), Random::sequence.GetUnit<gfx::Scalar>(), Random::sequence.GetUnit<gfx::Scalar>(), .5f);

			gfx::Debug::AddTriangle(geom::Cast<gfx::Scalar>(triangle), random_color);

			Plane3 plane(triangle);
			AddLine(plane.position, plane.position + geom::Normalized(plane.normal));
		}
#else
		inline void AddTriangle(Triangle3 const &)
		{
		}
#endif

		SideAttributes GenerateSideAttribute(Ray3 const & ray, Plane3 const & plane)
		{
			VerifyNearlyEqual(geom::Length(ray.direction), Scalar(1.), Scalar(.001));
			VerifyNearlyEqual(geom::Length(plane.normal), Scalar(1.), Scalar(.001));

			SideAttributes side_attributes;

			auto ray_to_plane = ray.position - plane.position;
			auto ray_to_plane_distance = geom::DotProduct(ray_to_plane, plane.normal);
			side_attributes.dot_product = - geom::DotProduct(plane.normal, ray.direction);
			side_attributes.intersection = ray_to_plane_distance / side_attributes.dot_product;

			return side_attributes;
		}

		SideAttributes GenerateSideAttribute(Ray3 const & ray, Triangle3 const & side)
		{
			// assumes contact will typically occur much closer to surface
			Plane3 plane;
			plane.position = (side.points[0] + side.points[1]) * Scalar(.5);
			plane.normal = geom::Normalized(geom::Normal(side));

			return GenerateSideAttribute(ray, plane);
		}

		// given attributes with all 3 side attributes filled out, calculate the range
		void GenerateAttributesRange(Attributes & attributes)
		{
			attributes.range[0] = std::numeric_limits<Scalar>::lowest();
			attributes.range[1] = std::numeric_limits<Scalar>::max();

			for (int side_index0 = 0; side_index0 != 3; ++ side_index0)
			{
				auto & side_attributes = attributes.sides[side_index0];

				if (side_attributes.dot_product > 0)
				{
					// going in to the area
					attributes.range[0] = std::max(attributes.range[0], side_attributes.intersection);
				}
				else
				{
					// going out of the area
					attributes.range[1] = std::min(attributes.range[1], side_attributes.intersection);
				}
			}
		}

		// fill out attributes given the ray cast invariants
		// and known information about the node it describes
		Attributes GenerateAttributes(Uniforms const & uniforms, Node const * node, form::Point const & a, form::Point const & b, form::Point const & c)
		{
			Attributes attributes;
			attributes.node = node;
			attributes.surface = Triangle3(geom::Cast<Scalar>(a.pos), geom::Cast<Scalar>(b.pos), geom::Cast<Scalar>(c.pos));

			for (int side_index0 = 0; side_index0 != 3; ++ side_index0)
			{
				const auto side_index1 = TriMod(side_index0 + 1);
				const auto side_index2 = TriMod(side_index0 + 2);

				const auto & a = attributes.surface.points[side_index2];
				const auto & b = attributes.surface.points[side_index1];
				const Triangle3 side(a, b, uniforms.center);

				attributes.sides[side_index0] = GenerateSideAttribute(uniforms.ray, side);
			}

			GenerateAttributesRange(attributes);

			return attributes;
		}

		// as above with a little less known information
		Attributes GenerateAttributes(Uniforms const & uniforms, Node const & node)
		{
			return GenerateAttributes(uniforms, & node, node.GetCorner(0), node.GetCorner(1), node.GetCorner(2));
		}

		// iterates over the given range of node attributes in asscending order of distance
		// from ray start; filters out unsuitable candidates; calls given function on the rest
		// and remembers the best result, i.e. where the first contact with the ray occurs
		RayCastResult ForEachFace(Attributes * begin, Attributes * end, Uniforms const & uniforms, ForEachFaceFunction function)
		{
			// sort the attribute array in ascending order of distance from ray start
			std::sort(begin, end);

			RayCastResult result;

			for (auto i = begin; i != end; ++ i)
			{
				Attributes const & attributes = * i;

				// If this node is behind the ray
				if (attributes.range[1] <= 0)
				{
					// then advance immediately to the next.
					continue;
				}

				// If the range of the ray this node covers is impossible,
				if (attributes.range[0] >= attributes.range[1])
				{
					// this node is to the side of the ray; skip it.
					continue;
				}

				// If this node is past the extend of the ray,
				bool is_past_the_end = attributes.range[0] >= uniforms.length;
				if (is_past_the_end)
				{
#if ! defined(NDEBUG) && 0
					// then all following nodes are also beyond the ray,
					std::for_each(i, end, [uniforms] (Attributes const & a) 
					{
						ASSERT(a.range[0] >= uniforms.length);
					});
#endif

					// so give up now.
					break;
				}

				// This is currently out best hope for a contact; call in.
				RayCastResult child_result = (* function)(uniforms, attributes);
				VerifyObject(child_result);

				// evaluate result
				result = std::min(result, child_result);
			}

			return result;
		}

		// generates a RayCastResult based on test of contact with an actual surface
		RayCastResult DoFace(Uniforms const & uniforms, Attributes const & leaf_attributes)
		{
			RayCastResult result;

			Triangle3 const & side = leaf_attributes.surface;
			Plane3 plane(side);
			geom::Normalize(plane.normal);
			const auto side_attributes = GenerateSideAttribute(uniforms.ray, plane);

			if (side_attributes.dot_product > 0 // only register entry - not exis
			&& side_attributes.intersection >= leaf_attributes.range[0] 
			&& side_attributes.intersection < leaf_attributes.range[1])
			{
				result.normal = geom::Cast<form::Scalar>(plane.normal);
				result.projection = side_attributes.intersection;
				result.node = leaf_attributes.node;

				VerifyObject(result);
			}
			else
			{
				result.normal = - geom::Cast<form::Scalar>(uniforms.ray.direction);

				VerifyObject(result);
			}

			AddTriangle(leaf_attributes.surface);

			return result;
		}

		// iterates over faces of a leaf node, testing for ray contact
		RayCastResult DoLeaf(Uniforms const & uniforms, Attributes const & attributes)
		{
			std::array<Attributes, 4> children;
			auto children_begin = std::begin(children);
			auto children_end = std::begin(children);
			ForEachNodeFace(* attributes.node, [& uniforms, & attributes, & children_end] (form::Point const & a, form::Point const & b, form::Point const & c, form::Vector3 const &, form::Scalar)
			{
				*children_end = GenerateAttributes(uniforms, attributes.node, a, b, c);
				++ children_end;
			});
			ASSERT(children_end >= std::begin(children));
			ASSERT(children_end <= std::end(children));

			return ForEachFace(children_begin, children_end, uniforms, & DoFace);
		}

		// iterates over children of a non-leaf node, testing for ray contact
		RayCastResult Recurse(Uniforms const & uniforms, Attributes const & attributes)
		{
			RayCastResult result;

			const auto & node = ref(attributes.node);
			auto children = node.GetChildren();
			if (children == nullptr)
			{
				return DoLeaf(uniforms, attributes);
			}

			Attributes child_attributes[4];

			// the center child node contains entirely novel edges
			child_attributes[3] = GenerateAttributes(uniforms, children[3]);

			// but each of the outer three children 
			// are some combination of the parent and the center child
			for (auto child_index0 = 0; child_index0 != 3; ++ child_index0)
			{
				auto child_index1 = TriMod(child_index0 + 1);
				auto child_index2 = TriMod(child_index0 + 2);

				auto & child_attribute = child_attributes[child_index0];

				child_attribute.node = children + child_index0;

				child_attribute.surface.points[child_index0] = attributes.surface.points[child_index0];
				child_attribute.surface.points[child_index1] = child_attributes[3].surface.points[child_index2];
				child_attribute.surface.points[child_index2] = child_attributes[3].surface.points[child_index1];

				child_attribute.sides[child_index0] = child_attributes[3].sides[child_index0];
				child_attribute.sides[child_index0].dot_product *= -1;
				child_attribute.sides[child_index1] = attributes.sides[child_index1];
				child_attribute.sides[child_index2] = attributes.sides[child_index2];

				GenerateAttributesRange(child_attribute);
			}

			return ForEachFace(std::begin(child_attributes), std::end(child_attributes), uniforms, & Recurse);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// CastRay implementation

RayCastResult form::CastRay(Polyhedron const & polyhedron, Ray3 const & ray, Scalar length)
{
	VerifyIsUnit(ray.direction, .0001f);
	VerifyOp(length, >=, 0.f);
	
	// get children
	auto & root_node = polyhedron.GetRootNode();
	auto children = root_node.GetChildren();
	if (children == nullptr)
	{
		DEBUG_BREAK("code path not implemented");
		return RayCastResult();
	}

	// generate uniforms
	auto polyhedron_center = geom::Cast<Scalar>(polyhedron.GetShape().center);
	impl::Uniforms uniforms = 
	{
		geom::Cast<impl::Scalar>(polyhedron_center),
		impl::Ray3(geom::Cast<impl::Scalar>(ray)),
		length
	};

	// generate child attributes
	impl::Attributes child_attributes[4];
	for (auto child_index = 0; child_index != 4; ++ child_index)
	{
		impl::Attributes & child_attribute = child_attributes[child_index];
		Node const & child_node = children[child_index];

		child_attribute = GenerateAttributes(uniforms, child_node);
	}

	// begin!
	return impl::ForEachFace(std::begin(child_attributes), std::end(child_attributes), uniforms, & impl::Recurse);
}
