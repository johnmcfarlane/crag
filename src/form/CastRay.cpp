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
#include "RayCastResult.h"

#include "geom/Intersection.h"

#if ! defined(NDEBUG)
//#define DEBUG_SHOW_LINE
//#define DEBUG_SHOW_RESULT
#endif

#if defined(DEBUG_SHOW_LINE) || defined(DEBUG_SHOW_RESULT)
#define DEBUG_DRAW_TRIANGLES
#include "gfx/Debug.h"
#include "core/Random.h"
#endif

using namespace form;

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

#if defined(DEBUG_DRAW_TRIANGLES)
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
#endif

		SideAttributes GenerateSideAttribute(Ray3 const & ray, Plane3 const & plane)
		{
			CRAG_VERIFY_NEARLY_EQUAL(geom::Length(ray.direction), Scalar(1.), Scalar(.001));
			CRAG_VERIFY_NEARLY_EQUAL(geom::Length(plane.normal), Scalar(1.), Scalar(.001));

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

				const auto & p = attributes.surface.points[side_index2];
				const auto & q = attributes.surface.points[side_index1];
				const Triangle3 side(p, q, uniforms.center);

				attributes.sides[side_index0] = GenerateSideAttribute(uniforms.ray, side);
			}

			GenerateAttributesRange(attributes);

			return attributes;
		}

		// as above with a little less known information
		Attributes GenerateAttributes(Uniforms const & uniforms, Node const & node)
		{
			return GenerateAttributes(uniforms, & node, ref(node.GetCorner(0)), ref(node.GetCorner(1)), ref(node.GetCorner(2)));
		}

		// iterates over the given range of node attributes in asscending order of distance
		// from ray start; filters out unsuitable candidates; calls given function on the rest
		// and remembers the best result, i.e. where the first contact with the ray occurs
		template <typename Iterator>
		RayCastResult ForEachFace(Iterator begin, Iterator end, Uniforms const & uniforms, ForEachFaceFunction function)
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
				CRAG_VERIFY(child_result);
				CRAG_VERIFY_TRUE(child_result.GetDistance() == std::numeric_limits<Scalar>::max() || child_result.GetDistance() <= uniforms.length);

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

			if (side_attributes.dot_product > 0 // only register entry - not exit
			&& side_attributes.intersection >= leaf_attributes.range[0]	// within the volume covered by the node
			&& side_attributes.intersection < leaf_attributes.range[1]
			&& side_attributes.intersection <= uniforms.length	// within the overall range
			&& side_attributes.intersection >= 0)
			{
				return RayCastResult(geom::Cast<form::Scalar>(plane.normal), side_attributes.intersection, leaf_attributes.node);
			}
			else
			{
				return RayCastResult();
			}

#if defined(DEBUG_SHOW_LINE)
			AddTriangle(leaf_attributes.surface);
#endif

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
	CRAG_VERIFY_UNIT(ray.direction, .0001f);
	CRAG_VERIFY_OP(length, >=, 0.f);
	
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
	{
		auto & root_node = polyhedron.GetRootNode();
		auto children = root_node.GetChildren();
		if (children != nullptr)
		{
			for (auto child_index = 0; child_index != 4; ++ child_index)
			{
				impl::Attributes & child_attribute = child_attributes[child_index];
				Node const & child_node = children[child_index];

				child_attribute = GenerateAttributes(uniforms, child_node);
			}
		}
		else
		{
			child_attributes[0] = GenerateAttributes(uniforms, & root_node, * root_node.GetCorner(0), * root_node.GetMidPoint(2), * root_node.GetMidPoint(1));
			child_attributes[1] = GenerateAttributes(uniforms, & root_node, * root_node.GetCorner(1), * root_node.GetMidPoint(0), * root_node.GetMidPoint(2));
			child_attributes[2] = GenerateAttributes(uniforms, & root_node, * root_node.GetCorner(2), * root_node.GetMidPoint(1), * root_node.GetMidPoint(0));
			child_attributes[3] = GenerateAttributes(uniforms, & root_node, * root_node.GetMidPoint(0), * root_node.GetMidPoint(1), * root_node.GetMidPoint(2));
		}
	}

	// begin!
#if defined(DEBUG_SHOW_RESULT)
	auto result = impl::ForEachFace(std::begin(child_attributes), std::end(child_attributes), uniforms, & impl::Recurse);
	if (result.node)
	{
		impl::AddTriangle(Triangle3(result.node->GetCorner(0).pos, result.node->GetCorner(1).pos, result.node->GetCorner(2).pos));
	}

	CRAG_VERIFY(result);
	CRAG_VERIFY_TRUE(result.GetDistance() == std::numeric_limits<Scalar>::max() || result.GetDistance() <= uniforms.length);

	return result;
#else
	return impl::ForEachFace(std::begin(child_attributes), std::end(child_attributes), uniforms, & impl::Recurse);
#endif
}
