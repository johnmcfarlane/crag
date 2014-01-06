//
//  ResourceManager.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ResourceManager.h"

#include "IndexedVboResource.h"
#include "Program.h"
#include "Quad.h"
#include "Shader.h"
#include "Vertex.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// file-local definitions

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// Cuboid type definitions

	typedef IndexedVboResource<Vertex, GL_STATIC_DRAW> Cuboid;
	
	////////////////////////////////////////////////////////////////////////////////
	// cuboid creation

	Cuboid CreateCuboid()
	{
		Cuboid::Vertex vertices[3][2][4];
		ElementIndex indices[3][2][2][3];
	
		ElementIndex * index = * * * indices;
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				int pole_sign = pole ? 1 : -1;
				int index_1 = TriMod(axis + TriMod(3 + pole_sign));
				int index_2 = TriMod(axis + TriMod(3 - pole_sign));
			
				Cuboid::Vertex * polygon_vertices = vertices[axis][pole];
				Vector3 normal = Vector3::Zero();
				normal[axis] = float(pole_sign);
			
				Cuboid::Vertex * polygon_vert = polygon_vertices;
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
						polygon_vert->color = Color4f::White();
					}
				}
			
				ElementIndex index_base = polygon_vertices - vertices[0][0];
				* (index ++) = index_base + 0;
				* (index ++) = index_base + 1;
				* (index ++) = index_base + 2;
			
				* (index ++) = index_base + 3;
				* (index ++) = index_base + 2;
				* (index ++) = index_base + 1;
			}
		}
	
		int num_vertices = sizeof(vertices) / sizeof(Cuboid::Vertex);
		int num_indices = sizeof(indices) / sizeof(ElementIndex);
		
		Cuboid cuboid(* * vertices, * * vertices + num_vertices, * * * indices, * * * indices + num_indices);
		
		return cuboid;
	}
}

////////////////////////////////////////////////////////////////////////////////
// gfx::ResourceManager

ResourceManager::ResourceManager()
{
	if (! InitShaders())
	{
		ASSERT(false);
	}
	
	if (! InitGeometry())
	{
		ASSERT(false);
	}
}

ResourceManager::~ResourceManager()
{
	for (auto & vbo : _vbos)
	{
		delete vbo;
		vbo = nullptr;
	}
	
	for (auto & program : _programs)
	{
		program->Deinit();
		delete program;
		program = nullptr;
	}
}

Program * ResourceManager::GetProgram(ProgramIndex index)
{
	ASSERT(index >= ProgramIndex(0) && index < ProgramIndex::size);
	return _programs[int(index)];
}

Program const * ResourceManager::GetProgram(ProgramIndex index) const
{
	ASSERT(index >= ProgramIndex(0) && index < ProgramIndex::size);
	return _programs[int(index)];
}

VboResource & ResourceManager::GetVbo(VboIndex index)
{
	ASSERT(index >= static_cast<VboIndex>(0) && index < VboIndex::size);
	return ref(_vbos[static_cast<std::size_t>(index)]);
}

VboResource const & ResourceManager::GetVbo(VboIndex index) const
{
	ASSERT(index >= static_cast<VboIndex>(0) && index < VboIndex::size);
	return ref(_vbos[static_cast<std::size_t>(index)]);
}

bool ResourceManager::InitShaders()
{
	auto init_program = [&] (Program * program, ProgramIndex index, char const * vert_filename, char const * frag_filename1, char const * frag_filename2)
	{
		ASSERT(program);
		if (program)
		{
			static char const * light_shader_filename = "assets/glsl/light.glsl";
			char const * vert_filenames[] = { vert_filename, light_shader_filename, nullptr };
			char const * frag_filenames[] = { frag_filename1, light_shader_filename, frag_filename2, nullptr };
			program->Init(vert_filenames, frag_filenames);
		}
		
		_programs[int(index)] = program;
	};

#if defined(CRAG_USE_GL)
	char const * flat_shader_filename = "assets/glsl/flat_enabled.glsl";
#elif defined(CRAG_USE_GLES)
	// performs a no-op on its inputs to avoid flat shading;
	// necessary shader, GL_OES_standard_derivatives, is available on some GLES2
	// systems but as they often don't have an 'F' key, it's never enabled anyway
	char const * flat_shader_filename = "assets/glsl/flat_disabled.glsl";
#endif

	init_program(new PolyProgram, ProgramIndex::poly, "assets/glsl/poly.vert", "assets/glsl/poly.frag", flat_shader_filename);
	init_program(new ShadowProgram, ProgramIndex::shadow, "assets/glsl/shadow.vert", "assets/glsl/shadow.frag", nullptr);
	init_program(new ScreenProgram, ProgramIndex::screen, "assets/glsl/screen.vert", "assets/glsl/screen.frag", nullptr);
	init_program(new DiskProgram, ProgramIndex::sphere, "assets/glsl/disk.vert", "assets/glsl/sphere.frag", nullptr);
	init_program(new FogProgram, ProgramIndex::fog, "assets/glsl/disk.vert", "assets/glsl/fog.frag", nullptr);
	init_program(new DiskProgram, ProgramIndex::disk, "assets/glsl/disk.vert", "assets/glsl/disk.frag", nullptr);
	init_program(new TexturedProgram, ProgramIndex::skybox, "assets/glsl/skybox.vert", "assets/glsl/skybox.frag", nullptr);
	init_program(new SpriteProgram, ProgramIndex::sprite, "assets/glsl/sprite.vert", "assets/glsl/sprite.frag", nullptr);

	return true;
}

bool ResourceManager::InitGeometry()
{
	auto init_vbo = [&] (VboResource * vbo, VboIndex index)
	{
		ASSERT(vbo);
		_vbos[static_cast<int>(index)] = vbo;
	};
	
	init_vbo(new Cuboid(CreateCuboid()), VboIndex::cuboid_mesh);
	init_vbo(new Quad(-1), VboIndex::sphere_quad);
	init_vbo(new Quad(0), VboIndex::disk_quad);
	
	return true;
}
