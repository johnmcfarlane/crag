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

#include "gfx/Engine.h"
#include "gfx/Image.h"
#include "gfx/Scene.h"
#include "gfx/Program.h"

#include "core/ResourceManager.h"

using namespace gfx;

namespace
{
	// Types
	struct Vertex
	{
		geom::Vector3f pos;
		geom::Vector2f tex;
	};
	
	//typedef VertexBufferObject<Vertex> Vbo;
	using SkyboxVboResource = NonIndexedVboResource<Vertex, GL_STATIC_DRAW>;
	
	char const * vbo_key = "SkyboxVbo";

	// TODO: Cube map
	SkyboxVboResource CreateVbo()
	{
		Vertex verts[3][2][6];
	
		for (auto axis = 0; axis < 3; ++ axis)
		{
			auto x_axis = TriMod(axis + 1);
			auto y_axis = TriMod(axis + 2);
			auto z_axis = TriMod(axis);
		
			for (auto pole = 0; pole < 2; ++ pole)
			{
				//Vertex * side_verts = verts[axis][pole];
				
				Vertex side_verts[2][2];
			
				for (auto u = 0; u < 2; ++ u)
				{
					for (auto v = 0; v < 2; ++ v)
					{
						auto & vert = side_verts[u][v];
						
						float * axes = vert.pos.GetAxes();
						axes[x_axis] = (static_cast<float>(pole ^ u) - .5f) * 2;
						axes[y_axis] = (static_cast<float>(v) - .5f) * 2;
						axes[z_axis] = (static_cast<float>(pole) - .5f) * 2;
						vert.tex.x = static_cast<float>(u);
						vert.tex.y = static_cast<float>(v);
					}
				}
				
				auto verts_ptr = verts[axis][pole];
				verts_ptr[0] = side_verts[0][0];
				verts_ptr[1] = side_verts[0][1];
				verts_ptr[2] = side_verts[1][0];
				verts_ptr[3] = side_verts[1][1];
				verts_ptr[4] = side_verts[1][0];
				verts_ptr[5] = side_verts[0][1];
			}
		}
		
		auto size = 3 * 2 * 6;
		auto begin = verts[0][0];
		auto end = begin + size;
		ASSERT(end == & verts[3][0][0]);
		
		SkyboxVboResource vbo;
		vbo.Set(begin, end);

		return vbo;
	}
}

////////////////////////////////////////////////////////////////////////////////
// vertex helper functions

template <>
void EnableClientState<Vertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glEnableVertexAttribArray(2));
}

template <>
void DisableClientState<Vertex>()
{
	GL_CALL(glDisableVertexAttribArray(2));
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<Vertex>()
{
	VertexAttribPointer<1, Vertex, decltype(Vertex::pos), & Vertex::pos>();
	VertexAttribPointer<2, Vertex, decltype(Vertex::tex), & Vertex::tex>();
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Skybox member definitions

Skybox::Skybox(Engine & engine)
: Object(engine, gfx::Transformation::Matrix44::Identity(), Layer::background)
{
	auto & resource_manager = engine.GetResourceManager();
	
	// VBO
	resource_manager.Register<SkyboxVboResource>(vbo_key, [&] () {
		return CreateVbo();
	});
	auto vbo_handle = resource_manager.GetHandle<SkyboxVboResource>(vbo_key);
	SetVboResource(vbo_handle);

	// shader
	auto skybox_program = resource_manager.GetHandle<TexturedProgram>("SkyboxProgram");
	SetProgram(skybox_program);
}

void Skybox::SetSide(int axis, int pole, std::shared_ptr<Image> const & image)
{
	char name[20];
	sprintf(name, "skybox%d%d", axis, pole);
	ResourceKey key(name);
	
	auto & resource_manager = GetEngine().GetResourceManager();
	resource_manager.Register<Texture>(key, [image] () {
		return image->CreateTexture();
	});
	
	sides[axis][pole] = resource_manager.GetHandle<Texture>(key);
}

void Skybox::UpdateModelViewTransformation(Transformation const & model_view)
{
	// Set model view matrix (with zero translation).
	Matrix33 rotation = model_view.GetRotation();
	SetModelViewTransformation(Transformation(Vector3::Zero(), rotation));
}

void Skybox::Render(Engine const &) const
{
	// Note: Skybox is being drawn very tiny but with z test off. This stops writing.
	glDepthMask(GL_FALSE);

	auto const & vbo = core::StaticCast<SkyboxVboResource const &>(* GetVboResource());
	
	int index = 0;
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			auto const & side = * sides[axis][pole];
			side.Bind();
			vbo.DrawTris(index, 6);
			side.Unbind();
			index += 6;
		}
	}
	
	glDepthMask(GL_TRUE);
}
