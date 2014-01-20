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
#include "MeshResource.h"
#include "Program.h"
#include "Quad.h"
#include "Shader.h"
#include "LitVertex.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// file-local definitions

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// cuboid creation

	// a lit cuboid has additional vertices to express the normals of sharp edges
	LitMesh CreateLitCuboid()
	{
		LitVertex vertices[3][2][4];	// [axis][sign][corner]
		ElementIndex indices[3][2][2][3];
	
		ElementIndex * index = * * * indices;
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				int pole_sign = pole ? 1 : -1;
				int index_1 = TriMod(axis + TriMod(3 + pole_sign));
				int index_2 = TriMod(axis + TriMod(3 - pole_sign));
			
				LitVertex * polygon_vertices = vertices[axis][pole];
				Vector3 normal = Vector3::Zero();
				normal[axis] = float(pole_sign);
			
				LitVertex * polygon_vert = polygon_vertices;
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
	
		int num_vertices = sizeof(vertices) / sizeof(LitVertex);
		int num_indices = sizeof(indices) / sizeof(ElementIndex);
		
		LitMesh cuboid(num_vertices, num_indices);
		
		std::copy(* * vertices, * * vertices + num_vertices, cuboid.GetVertices().data());
		std::copy(* * * indices, * * * indices + num_indices, cuboid.GetIndices().data());
		
		return cuboid;
	}

	ShadowVolumeMesh CreateCuboid()
	{
		PlainVertex vertices[2][2][2];	// [z][y][x]
		ElementIndex indices[3][2][2][3];
		
		Vector3 position;
		for (int z = 0; z < 2; ++ z)
		{
			position.z = z ? .5f : -.5f;

			for (int y = 0; y < 2; ++ y)
			{
				position.y = y ? .5f : -.5f;
	
				for (int x = 0; x < 2; ++ x)
				{
					position.x = x ? .5f : -.5f;
					
					vertices[z][y][x].pos = position;
				}
			}
		}
		
		ElementIndex * index = * * * indices;
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				int index_1 = TriMod(axis + 1);
				int index_2 = TriMod(axis + 2);
				
				auto get_index = [&] (int u, int v)
				{
					int poles[3];
					poles[axis] = pole;
					poles[index_1] = u ^ pole;
					poles[index_2] = v;
					
					auto & vertex = vertices[poles[2]][poles[1]][poles[0]];
					return & vertex - * * vertices;
				};
				
				* (index ++) = get_index(0, 0);
				* (index ++) = get_index(1, 0);
				* (index ++) = get_index(0, 1);
			
				* (index ++) = get_index(0, 1);
				* (index ++) = get_index(1, 0);
				* (index ++) = get_index(1, 1);
			}
		}
		
		CRAG_VERIFY_EQUAL(index, & indices[3][0][0][0]);
	
		int num_vertices = sizeof(vertices) / sizeof(PlainVertex);
		int num_indices = sizeof(indices) / sizeof(ElementIndex);
		
		ShadowVolumeMesh cuboid(num_vertices, num_indices);
		
		std::copy(* * vertices, * * vertices + num_vertices, cuboid.GetVertices().data());
		std::copy(* * * indices, * * * indices + num_indices, cuboid.GetIndices().data());
		
		return cuboid;
	}
}

////////////////////////////////////////////////////////////////////////////////
// gfx::ResourceManager

ResourceManager::ResourceManager()
{
	if (! InitModels())
	{
		ASSERT(false);
	}
	
	if (! InitShaders())
	{
		ASSERT(false);
	}
	
	if (! InitVbos())
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
		delete program;
		program = nullptr;
	}
}

Model & ResourceManager::GetModel(ModelIndex index)
{
	ASSERT(index >= ModelIndex(0) && index < ModelIndex::size);
	return ref(_models[int(index)]);
}

Model const & ResourceManager::GetModel(ModelIndex index) const
{
	ASSERT(index >= ModelIndex(0) && index < ModelIndex::size);
	return ref(_models[int(index)]);
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

bool ResourceManager::InitModels()
{
	_models[int(ModelIndex::cuboid)] = new MeshResource<PlainVertex> (std::move(CreateCuboid()));
	_models[int(ModelIndex::lit_cuboid)] = new MeshResource<LitVertex> (std::move(CreateLitCuboid()));
	
	return true;
}

bool ResourceManager::InitShaders()
{
#if defined(CRAG_USE_GL)
	char const * flat_shader_filename = "assets/glsl/flat_enabled.glsl";
#elif defined(CRAG_USE_GLES)
	// performs a no-op on its inputs to avoid flat shading;
	// necessary shader, GL_OES_standard_derivatives, is available on some GLES2
	// systems but as they often don't have an 'F' key, it's never enabled anyway
	char const * flat_shader_filename = "assets/glsl/flat_disabled.glsl";
#endif

	static char const * light_shader_filename = "assets/glsl/light.glsl";

	_programs[int(ProgramIndex::poly)] = new PolyProgram(
		{ "assets/glsl/poly.vert", light_shader_filename },
		{ "assets/glsl/poly.frag", flat_shader_filename, light_shader_filename });

	_programs[int(ProgramIndex::shadow)] = new ShadowProgram(
		{ "assets/glsl/shadow.vert", light_shader_filename },
		{ "assets/glsl/shadow.frag", light_shader_filename });

	_programs[int(ProgramIndex::screen)] = new ScreenProgram(
		{ "assets/glsl/screen.vert", light_shader_filename },
		{ "assets/glsl/screen.frag", light_shader_filename });

	_programs[int(ProgramIndex::sphere)] = new DiskProgram(
		{ "assets/glsl/disk.vert", light_shader_filename },
		{ "assets/glsl/sphere.frag", light_shader_filename });

	_programs[int(ProgramIndex::disk)] = new DiskProgram(
		{ "assets/glsl/disk.vert", light_shader_filename },
		{ "assets/glsl/disk.frag", light_shader_filename });

	_programs[int(ProgramIndex::skybox)] = new TexturedProgram(
		{ "assets/glsl/skybox.vert", light_shader_filename },
		{ "assets/glsl/skybox.frag", light_shader_filename });

	_programs[int(ProgramIndex::sprite)] = new SpriteProgram(
		{ "assets/glsl/sprite.vert", light_shader_filename },
		{ "assets/glsl/sprite.frag", light_shader_filename });

	for (auto program : _programs)
	{
		if (! program->IsInitialized())
		{
			return false;
		}
	}
	
	return true;
}

bool ResourceManager::InitVbos()
{
	typedef IndexedVboResource<LitVertex, GL_STATIC_DRAW> Cuboid;

	auto init_vbo = [&] (VboResource * vbo, VboIndex index)
	{
		ASSERT(vbo);
		_vbos[static_cast<int>(index)] = vbo;
	};
	
	auto & lit_cuboid_mesh = static_cast<MeshResource<LitVertex> const &>(ref(_models[int(ModelIndex::lit_cuboid)]));
	init_vbo(new Cuboid(lit_cuboid_mesh.GetMesh()), VboIndex::cuboid);
	init_vbo(new Quad(-1), VboIndex::sphere_quad);
	init_vbo(new Quad(0), VboIndex::disk_quad);
	
	return true;
}
