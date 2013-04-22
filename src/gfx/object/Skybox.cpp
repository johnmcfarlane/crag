//
//  Skybox.cpp
//  crag
//
//  Created by John on 11/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Skybox.h"

#include "gfx/Image.h"
#include "gfx/Pov.h"
#include "gfx/Engine.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"

#include "core/Random.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// vertex helper functions

template <>
void EnableClientState<Skybox::Vertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glEnableVertexAttribArray(2));
}

template <>
void DisableClientState<Skybox::Vertex>()
{
	GL_CALL(glDisableVertexAttribArray(2));
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<Skybox::Vertex>()
{
	VertexAttribPointer<1, Skybox::Vertex, decltype(Skybox::Vertex::pos), & Skybox::Vertex::pos>();
	VertexAttribPointer<2, Skybox::Vertex, decltype(Skybox::Vertex::tex), & Skybox::Vertex::tex>();
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Skybox member definitions

Skybox::Skybox(LeafNode::Init const & init)
: LeafNode(init, gfx::Transformation::Matrix44::Identity(), Layer::background)
{
	InitVerts();

	ResourceManager & resource_manager = init.engine.GetResourceManager();
	
	Program * skybox_program = resource_manager.GetProgram(ProgramIndex::skybox);
	SetProgram(skybox_program);
}

Skybox::~Skybox()
{
	vbo.Deinit();
	
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			Texture & side = sides[axis][pole];
			side.Deinit();
		}
	}
}

void Skybox::SetSide(int axis, int pole, Image const & image)
{
	Texture & side_tex = sides[axis][pole];
	
	image.CreateTexture(side_tex);
}

void Skybox::UpdateModelViewTransformation(Transformation const & model_view)
{
	// Set model view matrix (with zero translation).
	Matrix33 rotation = model_view.GetRotation();
	SetModelViewTransformation(Transformation(Vector3::Zero(), rotation));
}

void Skybox::Render(Engine const & renderer) const
{
	auto & engine = GetEngine();
	engine.SetCurrentMesh(nullptr);

	// Note: Skybox is being drawn very tiny but with z test off. This stops writing.
	Enable(GL_TEXTURE_2D);
	Disable(GL_CULL_FACE);
	glDepthMask(false);
	
	ASSERT(! IsEnabled(GL_LIGHTING));
	ASSERT(IsEnabled(GL_TEXTURE_2D));
	ASSERT(! IsEnabled(GL_CULL_FACE));
	
	// Draw VBO
	vbo.Bind();
	vbo.Activate();
	
	int index = 0;
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			Texture const & side = sides[axis][pole];
			side.Bind();
			vbo.DrawStrip(index, 4);
			side.Unbind();
			index += 4;
		}
	}
	
	vbo.Deactivate();
	vbo.Unbind();
	
	glDepthMask(true);
	Disable(GL_TEXTURE_2D);
	Enable(GL_CULL_FACE);
}

void Skybox::InitVerts()
{
	vbo.Init();
	Vertex verts[3][2][4];
	
	for (int axis = 0; axis < 3; ++ axis)
	{
		int x_axis = TriMod(axis + 1);
		int y_axis = TriMod(axis + 2);
		int z_axis = TriMod(axis);
		
		for (int pole = 0; pole < 2; ++ pole)
		{
			Vertex * side_verts = verts[axis][pole];
			
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
	
	vbo.Bind();
	vbo.BufferData(3 * 2 * 4, verts[0][0], GL_STATIC_DRAW);
	vbo.Unbind();
}
