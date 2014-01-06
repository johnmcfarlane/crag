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
#include "Vertex.h"

#include "geom/Intersection.h"

#if ! defined(NDEBUG)
#include "Debug.h"
#endif

namespace gfx
{
	// ShadowVolume class definition
	typedef gfx::IndexedVboResource<Vector3, GL_DYNAMIC_DRAW> ShadowVolume;

	// given a mesh and a light position, generates its shadow volume
	template <typename Mesh>
	void GenerateShadowVolume(ShadowVolume & shadow_volume, Mesh const & mesh, Vector3 light_position)
	{
		// types
		typedef std::vector<Vector3> ShadowVertexBuffer;
		typedef std::vector<ElementIndex> ShadowIndexBuffer;

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
		auto num_solid_vertices = solid_vertices.GetSize();
		auto solid_vertex_array = solid_vertices.GetArray();

		// shadow vertices
		ShadowVertexBuffer shadow_vertices(num_solid_vertices * 2);

		std::transform(solid_vertex_array, solid_vertex_array + num_solid_vertices, std::begin(shadow_vertices), [] (Vertex const & vertex)
		{
			return vertex.pos;
		});

		std::transform(solid_vertex_array, solid_vertex_array + num_solid_vertices, std::begin(shadow_vertices) + num_solid_vertices, [& light_position] (Vertex const & vertex)
		{
			auto & pos = vertex.pos;
			return pos * 2.f - light_position;
		});

		// shadow indices
		ShadowIndexBuffer shadow_indices;

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
				CRAG_VERIFY_OP((int)index, >=, 0);
				CRAG_VERIFY_OP((int)index, <, solid_indices.GetSize());
	
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
			
#if ! defined(NDEBUG)
//						// draw lines in direction of light rays
//						Debug::AddLine(shadow_vertices[a], shadow_vertices[a_shadow]);
//						Debug::AddLine(shadow_vertices[b], shadow_vertices[b_shadow]);

//						// draw silhouettes
//						Debug::AddLine(shadow_vertices[a], shadow_vertices[b]);
//						Debug::AddLine(shadow_vertices[a_shadow], shadow_vertices[b_shadow]);
#endif
					}

					unmatched_edges.erase(found);
				}
			};

			RegisterEdge(points[0], points[1], lit);
			RegisterEdge(points[1], points[2], lit);
			RegisterEdge(points[2], points[0], lit);
		};

		shadow_volume.Set(& * std::begin(shadow_vertices), & * std::end(shadow_vertices), & * std::begin(shadow_indices), & * std::end(shadow_indices));
	}
}
