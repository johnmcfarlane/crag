//
//  Cuboid.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Cuboid.h"


gfx::Cuboid::Cuboid()
{
	Vertex verts[3][2][4];
	GLuint indices[3][2][2][3];
	
	GLuint * index = * * * indices;
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			int pole_sign = pole ? 1 : -1;
			int index_1 = TriMod(axis + TriMod(3 + pole_sign));
			int index_2 = TriMod(axis + TriMod(3 - pole_sign));
			
			Vertex * polygon_verts = verts[axis][pole];
			Vector3 normal = Vector3::Zero();
			normal[axis] = float(pole_sign);
			
			Vertex * polygon_vert = polygon_verts;
			Vector3 position;
			position[axis] = .5f * pole_sign;
			for (int p = 0; p < 2; ++ p)
			{
				position[index_1] = (p) ? -.5f : .5f;
				for (int q = 0; q < 2; ++ polygon_vert, ++ q)
				{
					position[index_2] = (q) ? -.5f : .5f;
					polygon_vert->pos = position;
					polygon_vert->norm = normal;
				}
			}
			
			GLuint index_base = polygon_verts - verts[0][0];
			* (index ++) = index_base + 0;
			* (index ++) = index_base + 1;
			* (index ++) = index_base + 2;
			
			* (index ++) = index_base + 3;
			* (index ++) = index_base + 2;
			* (index ++) = index_base + 1;
		}
	}
	
	int num_verts = sizeof(verts) / sizeof(Vertex);
	int num_indices = sizeof(indices) / sizeof(GLuint);
	
	_mesh.Init();
	_mesh.Bind();
	
	_mesh.Resize(num_verts, num_indices, gl::STATIC_DRAW);
	_mesh.SetIbo(num_indices, * * * indices);
	_mesh.SetVbo(num_verts, * * verts);
	
	_mesh.Unbind();
}

gfx::Cuboid::~Cuboid()
{
	//gl::DeleteBuffer(_color_vbo);
	_mesh.Deinit();
}

void gfx::Cuboid::Draw() const
{
	// Perform the draw calls.
	_mesh.Bind();
	_mesh.Activate();
	_mesh.Draw(GL_TRIANGLES, 48, 0);
	_mesh.Deactivate();
	_mesh.Unbind();
}
