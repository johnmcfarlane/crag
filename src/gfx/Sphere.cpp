//
//  Sphere.cpp
//  crag
//
//  Created by John McFarlane on 7/9/11.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "Sphere.h"

#include "GeodesicSphere.h"

#include "geom/Transformation.h"


namespace gfx
{
	typedef GeodesicSphere<Sphere::Vertex> Geodesic;
	
	
	Sphere::Sphere()
	{
		VertexVector verts;
		Geodesic::FaceVector faces;
		
		Geodesic source(verts, faces, num_sphere_lods);
		
		_lod_coefficients = source.GetCoefficients();
		
		int num_verts = verts.size();
		int num_faces = faces.size();
		
		_mesh.Init();
		_mesh.Bind();
		
		_mesh.Resize(num_verts, num_faces * 3, gl::STATIC_DRAW);
		_mesh.SetIbo(num_faces * 3, faces[0]._indices);
		_mesh.SetVbo(num_verts, & verts[0]);
		
		_mesh.Unbind();
	}
	
	Sphere::~Sphere()
	{
		_mesh.Deinit();
	}
	
	Scalar Sphere::CalculateRadius(Transformation const & transformation)
	{
		Vector3 size = transformation.GetScale();
		Scalar radius = Length(size) * .5;
		return radius;
	}
	
	void Sphere::Draw(Transformation const & transformation) const
	{
		// Calculate the inverse distnce)
		Scalar radius = CalculateRadius(transformation);
		Vector3 relative_position = transformation.GetTranslation();
		Scalar inv_distance = InvSqrt(LengthSq(relative_position));
		int lod = CalculateLod(radius, inv_distance);
		
		Draw(lod);
	}
	
	void Sphere::Draw(int lod) const
	{
		float scale = _lod_coefficients[lod];
		gl::Scale(scale, scale, scale);
		
		// Select the correct range of indices, given the LoD.
		unsigned faces_begin = Geodesic::TotalNumFaces(lod - 1);
		unsigned faces_num = Geodesic::LodNumFaces(lod);
		unsigned indices_begin = faces_begin * 3;
		unsigned indices_num = faces_num * 3;
		
		// Perform the draw calls.
		_mesh.Bind();
		_mesh.Activate();
		_mesh.Draw(GL_TRIANGLES, indices_num, indices_begin);
		_mesh.Deactivate();
		_mesh.Unbind();
		
		GLPP_VERIFY;
	}
	
	unsigned Sphere::CalculateLod(Scalar radius, Scalar inv_distance_to_camera) const
	{
		Scalar mn1 = 1500;
		int lod = int(pow(mn1 * radius * inv_distance_to_camera, .3));
		Clamp(lod, 1, 5);
		-- lod;
		
		return lod;
	}
	
}
