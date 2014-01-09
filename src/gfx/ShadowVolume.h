//
//  gfx/ShadowVolume.h
//  crag
//
//  Created by John McFarlane on 2014-01-02.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "IndexedVboResource.h"
#include "Mesh.h"
#include "PlainVertex.h"

#include "geom/Intersection.h"

#if ! defined(NDEBUG)
#include "Debug.h"
#endif

namespace gfx
{
	// ShadowVolume class definition
	typedef gfx::IndexedVboResource<PlainVertex, GL_DYNAMIC_DRAW> ShadowVolume;
	
	typedef Mesh<PlainVertex> ShadowVolumeMesh;

	// given a mesh and a light position, generates geometry describing its shadow
	template <typename Vertex>
	ShadowVolumeMesh GenerateShadowVolumeMesh(Mesh<Vertex> const & mesh, Vector3 light_position)
	{
		// types
		typedef std::array<ElementIndex, 2> Edge;

		struct EdgeHash
		{
			std::size_t operator() (Edge const & edge) const
			{
				return edge[1] ^ (edge[0] << 16);
			}
		};

		// given an un matched edge, is its neighbouring surface lit?
		// (edges have two neighbouring surfaces once matched)
		typedef std::unordered_map<Edge, bool, EdgeHash> EdgeMap;
	
		// solid vertices
		auto & solid_vertices = mesh.GetVertices();
		auto num_solid_vertices = solid_vertices.size();
		auto solid_vertex_array = solid_vertices.data();
		
		// return object
		ShadowVolumeMesh shadow_volume_mesh(num_solid_vertices * 2, 0);

		// shadow vertices
		auto & shadow_vertices = shadow_volume_mesh.GetVertices();

		std::transform(solid_vertex_array, solid_vertex_array + num_solid_vertices, std::begin(shadow_vertices), [] (Vertex const & vertex) -> PlainVertex
		{
			return { vertex.pos };
		});

		std::transform(solid_vertex_array, solid_vertex_array + num_solid_vertices, std::begin(shadow_vertices) + num_solid_vertices, [& light_position] (Vertex const & vertex) -> PlainVertex
		{
			auto & pos = vertex.pos;
			auto dir = geom::Normalized(vertex.pos - light_position);
			return { pos + dir * 1000.f };
		});

		// shadow indices
		auto & shadow_indices = shadow_volume_mesh.GetIndices();

		EdgeMap unmatched_edges;

		auto & solid_indices = mesh.GetIndices();
		for (auto end = std::end(solid_indices), i = std::begin(solid_indices); i != end;)
		{
			typedef std::array<ElementIndex, 3> PointArray;
			typedef geom::Triangle<Scalar, 3> Triangle;

			PointArray points;
			Triangle triangle;

			for (auto p = 0; p != 3; ++ i, ++ p)
			{
				auto index = * i;
				CRAG_VERIFY_OP(index, >=, 0u);
				CRAG_VERIFY_OP(index, <, solid_indices.size());

				points[p] = index;
				triangle.points[p] = solid_vertices[index].pos;
			}

			bool lit = geom::Contains(geom::MakePlane(triangle), light_position);

			auto RegisterEdge = [&] (ElementIndex a, ElementIndex b, bool lit)
			{
				CRAG_VERIFY_OP(a, !=, b);
				CRAG_VERIFY_OP(a, <, num_solid_vertices);
				CRAG_VERIFY_OP(b, <, num_solid_vertices);

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
			
						auto a_shadow = a + num_solid_vertices;
						auto b_shadow = b + num_solid_vertices;
		
						shadow_indices.push_back(a);
						shadow_indices.push_back(a_shadow);
						shadow_indices.push_back(b);
						shadow_indices.push_back(b_shadow);
						shadow_indices.push_back(b);
						shadow_indices.push_back(a_shadow);
		
#if ! defined(NDEBUG) && 0
						// draw lines in direction of light rays
						Debug::AddLine(shadow_vertices[a].pos, shadow_vertices[a_shadow].pos);
						Debug::AddLine(shadow_vertices[b].pos, shadow_vertices[b_shadow].pos);

						// draw silhouettes
						Debug::AddLine(shadow_vertices[a].pos, shadow_vertices[b].pos);
						Debug::AddLine(shadow_vertices[a_shadow].pos, shadow_vertices[b_shadow].pos);
#endif
					}

					unmatched_edges.erase(found);
				}
			};

			RegisterEdge(points[0], points[1], lit);
			RegisterEdge(points[1], points[2], lit);
			RegisterEdge(points[2], points[0], lit);
		};

		return shadow_volume_mesh;
	}
}
