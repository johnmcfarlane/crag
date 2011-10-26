/*
 *  gfx/object/Skybox.cpp
 *  Crag
 *
 *  Created by John on 11/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Skybox.h"

#include "gfx/Image.h"
#include "gfx/Pov.h"
#include "gfx/Scene.h"

#include "core/Random.h"

#include "geom/Transformation.h"


using namespace gfx;


void Skybox::Init()
{
	InitVerts();
}

void Skybox::Deinit()
{
	gl::DeleteBuffer(vbo);
	
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			Texture & side = sides[axis][pole];
			gl::DeleteTexture(side);
		}
	}
}

void Skybox::SetSide(int axis, int pole, Image const & image)
{
	Texture & side_tex = sides[axis][pole];
	
	image.CreateTexture(side_tex);
}

void Skybox::Render(Layer::type layer, Scene const & scene) const
{
	Pov const & pov = scene.GetPov();
	
	// Set projection matrix within relatively tight bounds.
	Frustum skybox_frustum = pov.GetFrustum();
	skybox_frustum.near_z = .1f;
	skybox_frustum.far_z = 10.f;
	skybox_frustum.SetProjectionMatrix();
	
	// Set model view matrix (with zero translation).
	pov.SetModelView(Transformation(pov.GetPosition()));
	
	// Note: Skybox is being drawn very tiny but with z test off. This stops writing.
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	gl::Enable(GL_TEXTURE_2D);
	gl::Disable(GL_CULL_FACE);
	gl::SetDepthMask(false);
	
	Assert(! gl::IsEnabled(GL_LIGHTING));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	Assert(! gl::IsEnabled(GL_LIGHT0));
	Assert(gl::IsEnabled(GL_TEXTURE_2D));
	Assert(! gl::IsEnabled(GL_CULL_FACE));
	Assert(! gl::IsEnabled(GL_DEPTH_TEST));
	
	// Draw VBO
	gl::BindBuffer(vbo);
	vbo.Activate();
	
	int index = 0;
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			Texture const & side = sides[axis][pole];
			gl::BindTexture(side);
			vbo.DrawStrip(index, 4);
			gl::UnbindTexture(side);
			index += 4;
		}
	}
	
	vbo.Deactivate();
	gl::UnbindBuffer(vbo);
	
	gl::SetDepthMask(true);
	gl::Disable(GL_TEXTURE_2D);
	gl::Enable(GL_CULL_FACE);
}

bool Skybox::IsInLayer(Layer::type layer) const
{
	return layer == Layer::background;
}

void Skybox::InitVerts()
{
	gl::GenBuffer(vbo);
	gl::Vertex3dTex verts[3][2][4];
	
	for (int axis = 0; axis < 3; ++ axis)
	{
		int x_axis = TriMod(axis + 1);
		int y_axis = TriMod(axis + 2);
		int z_axis = TriMod(axis);
		
		for (int pole = 0; pole < 2; ++ pole)
		{
			gl::Vertex3dTex * side_verts = verts[axis][pole];
			
			for (int u = 0; u < 2; ++ u)
			{
				for (int v = 0; v < 2; ++ v)
				{
					float * axes = side_verts->pos.GetAxes();
					axes[x_axis] = (static_cast<float>(u) - .5f) * 2;
					axes[y_axis] = (static_cast<float>(v) - .5f) * 2;
					axes[z_axis] = (static_cast<float>(pole) - .5f) * 2;
					side_verts->tex.x = static_cast<float>(u);
					side_verts->tex.y = static_cast<float>(v);
					++ side_verts;
				}
			}
		}
	}
	
	gl::BindBuffer(vbo);
	gl::BufferData(vbo, 3 * 2 * 4, verts[0][0], gl::STATIC_DRAW);
	gl::UnbindBuffer(vbo);
}