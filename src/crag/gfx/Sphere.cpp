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

#include "glpp/glpp.h"


typedef gfx::GeodesicSphere<gfx::Sphere::Vertex> Geodesic;


gfx::Sphere::Sphere()
{
	Geodesic source(num_sphere_lods);
	
	_lod_coefficients = source.GetCoefficients();
	
	Geodesic::VertexVector & verts = source.GetVerts();
	for (Geodesic::VertexVector::iterator v = verts.begin(); v != verts.end(); ++ v)
	{
		v->norm = v->pos;
	}
	
	Geodesic::FaceVector & faces = source.GetFaces();
	
	_mesh.Init();
	_mesh.Bind();
	
	_mesh.Resize(verts.size(), faces.size() * 3);
	_mesh.SetIbo(faces.size() * 3, faces[0]._indices);
	_mesh.SetVbo(verts.size(), & verts[0]);
	
	_mesh.Unbind();
}

gfx::Sphere::~Sphere()
{
	_mesh.Deinit();
}

void gfx::Sphere::Draw(float radius, int lod) const
{
	float scale = _lod_coefficients[lod] * radius;
	gl::Scale(scale, scale, scale);
	
	// Select the correct range of indices, given the LoD.
	unsigned faces_begin = lod ? Geodesic::TotalNumFaces(lod - 1) : 0;
	unsigned faces_num = Geodesic::LodNumFaces(lod);
	unsigned indices_begin = faces_begin * 3;
	unsigned indices_num = faces_num * 3;
	
	// Perform the draw calls.
	_mesh.Bind();
	_mesh.Activate();
	_mesh.Draw(GL_TRIANGLES, indices_num, indices_begin);
	_mesh.Deactivate();
	_mesh.Unbind();
}
