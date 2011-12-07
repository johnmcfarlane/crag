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


typedef gfx::GeodesicSphere<gfx::Sphere::Vertex> Geodesic;


gfx::Sphere::Sphere()
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
	
	InitColors(verts);
}

gfx::Sphere::~Sphere()
{
	gl::DeleteBuffer(_color_vbo);
	_mesh.Deinit();
}

void gfx::Sphere::Draw(int lod) const
{
	float scale = _lod_coefficients[lod];
	gl::Scale(scale, scale, scale);
	
	// Select the correct range of indices, given the LoD.
	unsigned faces_begin = Geodesic::TotalNumFaces(lod - 1);
	unsigned faces_num = Geodesic::LodNumFaces(lod);
	unsigned indices_begin = faces_begin * 3;
	unsigned indices_num = faces_num * 3;
	
	// Perform the draw calls.
	ActivateColors();
	
	_mesh.Bind();
	_mesh.Activate();
	_mesh.Draw(GL_TRIANGLES, indices_num, indices_begin);
	_mesh.Deactivate();
	_mesh.Unbind();
	
	DeactivateColors();
}

void gfx::Sphere::InitColors(VertexVector const & vertex_positions)
{
	// Color buffer
	std::vector<Color> vert_colors;
	std::size_t num_verts = vertex_positions.size();
	vert_colors.reserve(num_verts);
	for (Geodesic::VertexVector::const_iterator i = vertex_positions.begin(); i != vertex_positions.end(); ++ i)
	{
		Vertex const & v = * i;
		
		Color c;
#if 1
		c.r = GLubyte((v.pos.x + 1.) * 127.5);
		c.g = GLubyte((v.pos.y + 1.) * 127.5);
		c.b = GLubyte((v.pos.z + 1.) * 127.5);
#else
		float b = v.pos.x * v.pos.y * v.pos.z;
		float d = pow(Abs(b), .5f) * b / Abs(b);
		GLubyte comp = (d + 1.f) * 127.5;
		c.r = c.g = c.b = comp;
		c.a = 0xff;
#endif
		vert_colors.push_back(c);
	}
	Assert(vert_colors.size() == num_verts);
	
	gl::GenBuffer(_color_vbo);
	gl::BindBuffer(_color_vbo);
	
	gl::BufferData(_color_vbo, num_verts, & vert_colors.front(), gl::STATIC_DRAW);
	gl::UnbindBuffer(_color_vbo);
}

void gfx::Sphere::ActivateColors() const
{
	gl::BindBuffer(_color_vbo);
	_color_vbo.Activate();
}

void gfx::Sphere::DeactivateColors() const
{
	gl::BindBuffer(_color_vbo);
	_color_vbo.Deactivate();
	gl::UnbindBuffer(_color_vbo);
}
