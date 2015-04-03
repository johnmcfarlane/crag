//
//  gfx/GenerateShadowVolumeMesh.h
//  crag
//
//  Created by John McFarlane on 2014-03-29.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "PlainVertex.h"

#include "geom/Intersection.h"

namespace gfx
{
	// given a mesh and a light position, generates geometry describing its shadow
	template <typename MeshType>
	std::vector<PlainVertex> GenerateShadowVolumeMesh(MeshType const & mesh, Vector3 light_position)
	{
		// types
		typedef std::array<Vector3 const *, 2> Edge;

		struct EdgeHash
		{
			std::size_t operator() (Edge const & edge) const
			{
				auto constexpr half_bits = (sizeof(std::size_t) * CHAR_BIT) >> 1;
				return std::size_t(edge[1]) ^ (std::size_t(edge[0]) << half_bits);
			}
		};

		// given an un matched edge, is its neighbouring surface lit?
		// (edges have two neighbouring surfaces once matched)
		typedef std::unordered_map<Edge, bool, EdgeHash> EdgeMap;
		
		// return object
		std::vector<PlainVertex> shadow_vertices;

		EdgeMap unmatched_edges;

		for (auto i = std::begin(mesh), end = std::end(mesh); i != end;)
		{
			typedef std::array<Vector3 const *, 3> PointArray;
			typedef geom::Triangle<Scalar, 3> Triangle;

			PointArray points;
			Triangle triangle;

			for (auto p = 0; p != 3; ++ i, ++ p)
			{
				auto const & vertex = * i;
				points[p] = & vertex.pos;
				triangle.points[p] = vertex.pos;
			}

			bool lit = geom::Contains(triangle, light_position);

			auto RegisterEdge = [&] (Vector3 const * a, Vector3 const * b, bool lit)
			{
				CRAG_VERIFY_OP(a, !=, b);

				auto edge = (a < b) ? Edge {{ a, b }} : Edge {{ b, a }};
				auto found = unmatched_edges.find(edge);
				if (found == unmatched_edges.end())
				{
					unmatched_edges.emplace(edge, lit);
				}
				else
				{
					// of two adjacent faces, if one is in light and the other is in shadow,
					// their edge defines part of a silhouette
					bool has_silhouette = found->second != lit;
					if (has_silhouette)
					{
						if (lit)
						{
							std::swap(a, b);
						}
			
						auto project = [&] (Vector3 const & corner)
						{
							Ray3 edge_ray;
							edge_ray.position = corner;
							edge_ray.direction = geom::Normalized(corner - light_position);
							return geom::Project(edge_ray, 1000.f);	// TODO: magic number
						};
						auto a_projection = project(* a);
						auto b_projection = project(* b);
						
						auto push_back = [&] (Vector3 const & vertex)
						{
							shadow_vertices.push_back({{ vertex }});
						};
						
						push_back(* a);
						push_back(a_projection);
						push_back(* b);
						push_back(b_projection);
						push_back(* b);
						push_back(a_projection);
					}

					unmatched_edges.erase(found);
				}
			};

			RegisterEdge(points[0], points[1], lit);
			RegisterEdge(points[1], points[2], lit);
			RegisterEdge(points[2], points[0], lit);
		};

		ASSERT((shadow_vertices.size() % 3) == 0);
		return shadow_vertices;
	}
}
