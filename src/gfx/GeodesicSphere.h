//
//  GeodesicSphere.h
//  crag
//
//  Created by John McFarlane on 7/7/11.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "icosahedron.h"

#include "geom/Sphere.h"


namespace gfx
{
	
	////////////////////////////////////////////////////////////////////////////////
	// GeodesicSphere class
	
	// represents a geodesic sphere as a mesh, 
	// conduicive to sending to vbo/ibo pair.
	template <typename VERTEX>
	class GeodesicSphere
	{
	public:
		// types
		typedef VERTEX Vertex;
		typedef float Scalar;
		typedef Vector<Scalar, 3> Vector;
		
		struct Face
		{
			unsigned _indices[3];
		};		
		
		typedef std::vector<Vertex> VertexVector;
		typedef std::vector<Face> FaceVector;
		typedef std::vector<Scalar> ScalarVector;
		
		
		GeodesicSphere(VertexVector & verts, FaceVector & faces, unsigned max_depth)
		: _verts(verts)
		, _faces(faces)
		, _max_depth(max_depth)
		{
			_verts.resize(TotalNumVerts(max_depth));
			_faces.resize(TotalNumFaces(max_depth));
			_coefficients.resize(max_depth);
			
			CreateIcosahedron();
			_coefficients[0] = CalculateCoefficient(0);
			
			for (unsigned depth = 1; depth < max_depth; ++ depth)
			{
				SubDivide(depth);
				_coefficients[depth] = CalculateCoefficient(depth);
			}
		}
		
		ScalarVector GetCoefficients() const
		{
			return _coefficients;
		}
		
		static signed LodNumFaces(signed lod)
		{
			if (lod > 0)
			{
				return LodNumFaces(lod - 1) * 4;
			}
			else 
			{
				return 20;
			}
		}
		
		static unsigned TotalNumFaces(int lod)
		{
			if (lod < 0)
			{
				return 0;
			}
			
			unsigned total = LodNumFaces(lod);
			if (lod > 0)
			{
				total += TotalNumFaces(lod - 1);
			}
			return total;
		}
		
		static unsigned LodNumVerts(unsigned lod)
		{
			if (lod > 0)
			{
				return (LodNumFaces(lod - 1) >> 1) * 3;
			}
			else 
			{
				return 12;
			}
		}
		
		static unsigned TotalNumVerts(int lod)
		{
			if (lod < 0)
			{
				return 0;
			}
			
			unsigned total = LodNumVerts(lod);
			if (lod > 0)
			{
				total += TotalNumVerts(lod - 1);
			}
			return total;
		}
		
	private:
		
		void CreateIcosahedron()
		{
			assert(icosahedron::num_verts == TotalNumVerts(0));
			for (unsigned i = 0; i != icosahedron::num_verts; ++ i)
			{
				Vertex & v = _verts[i];
				v.pos.x = icosahedron::verts[i][0];
				v.pos.y = icosahedron::verts[i][1];
				v.pos.z = icosahedron::verts[i][2];
			}
			
			assert(icosahedron::num_faces == TotalNumFaces(0));
			for (unsigned i = 0; i != icosahedron::num_faces; ++ i)
			{
				Face & f = _faces[i];
				f._indices[0] = icosahedron::faces[i][0];
				f._indices[1] = icosahedron::faces[i][1];
				f._indices[2] = icosahedron::faces[i][2];
			}
		}
		
		void SubDivide(unsigned depth)
		{
			assert(depth >= 1);
			
			unsigned const parent_face_begin = TotalNumFaces(depth - 2);
			unsigned const parent_face_end = TotalNumFaces(depth - 1);
			
			unsigned const child_face_begin = parent_face_end;
			unsigned child_face_end = child_face_begin;
			
			unsigned const child_vert_begin = TotalNumVerts(depth - 1);
			unsigned child_vert_end = child_vert_begin;
			
			for (unsigned face_index = parent_face_begin; face_index != parent_face_end; ++ face_index)
			{
				Face parent = _faces[face_index];
				Face * children = & _faces[child_face_end];
				
				for (unsigned t = 0; t != 3; ++ t)
				{
					// Set the outer corner of one of the three outer children.
					children[t]._indices[t] = parent._indices[t];
					
					// Calculate the (accumulated) mid-point of the opposite edge.
					unsigned adjacent_index_1 = TriMod(t + 1);
					unsigned adjacent_vert_index_1 = parent._indices[adjacent_index_1];
					assert(adjacent_vert_index_1 < child_vert_begin);
					Vertex const & vertex_1 = _verts[adjacent_vert_index_1];
					
					unsigned adjacent_index_2 = TriMod(t + 2);
					unsigned adjacent_vert_index_2 = parent._indices[adjacent_index_2];
					assert(adjacent_vert_index_2 < child_vert_begin);
					Vertex const & vertex_2 = _verts[adjacent_vert_index_2];
					
					// Don't normalize - or even average - the sum; save that for later(*).
					Vector calculated_midpoint = vertex_1.pos + vertex_2.pos;
					
					// See if it got calculated already.
					typename VertexVector::iterator midpoint;
					typename VertexVector::const_iterator end = _verts.begin() + child_vert_end;
					for (midpoint = _verts.begin() + child_vert_begin; ; ++ midpoint)
					{
						if (midpoint == end)
						{
							// If not, here's a new one for ya!
							assert(child_vert_end < TotalNumVerts(depth));
							midpoint->pos = calculated_midpoint;
							++ child_vert_end;
							break;
						}
						
						if (midpoint->pos == calculated_midpoint)
						{
							break;
						}
					}
					
					unsigned midpoint_index = midpoint - _verts.begin();
					
					// Set inner corners of two of the outer children,
					children[adjacent_index_1]._indices[adjacent_index_2] = midpoint_index;
					children[adjacent_index_2]._indices[adjacent_index_1] = midpoint_index;
					
					// and a corner of the inner child.
					children[3]._indices[t] = midpoint_index;
				}
				
				child_face_end += 4;
			}
			
			assert(child_face_end == TotalNumFaces(depth));
			assert(child_vert_end == TotalNumVerts(depth));
			
			// Now(*), normalize.
			for (unsigned child_vert_index = child_vert_begin; child_vert_index != child_vert_end; ++ child_vert_index)
			{
				Normalize(_verts[child_vert_index].pos);
			}
		}
		
		float CalculateCoefficient(int depth) const
		{
			double actual_volume = 0;
			Vector3f d = Vector3f::Zero();
			
			typename FaceVector::const_iterator faces_begin = _faces.begin() + TotalNumFaces(depth - 1);
			typename FaceVector::const_iterator faces_end = _faces.begin() + TotalNumFaces(depth);
			for (typename FaceVector::const_iterator i = faces_begin; i != faces_end; ++ i)
			{
				Vector3f const & a = reinterpret_cast<Vector3f const &>(_verts[i->_indices[0]]);
				Vector3f const & b = reinterpret_cast<Vector3f const &>(_verts[i->_indices[1]]);
				Vector3f const & c = reinterpret_cast<Vector3f const &>(_verts[i->_indices[2]]);
				actual_volume += TetrahedronVolume<float>(a, b, c, d);
			}
			
			double ideal_volume = SphereVolume<double, 3>(1);
			double volume_adjust = ideal_volume / actual_volume;
			double coefficient = pow(volume_adjust, 1. / 3.);
			
			return float(coefficient);
		}
		
		// variables
		unsigned const _max_depth;
		
		VertexVector & _verts;
		FaceVector & _faces;
		ScalarVector _coefficients;
	};
	
}
