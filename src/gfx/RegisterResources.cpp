//
//  gfx/ResourceManager.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RegisterResources.h"

#include "IndexedVboResource.h"
#include "LitVertex.h"
#include "Mesh.h"
#include "PlainVertex.h"
#include "Program.h"
#include "Quad.h"
#include "Shader.h"
#include "LitVertex.h"

#include "core/ResourceManager.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// file-local definitions

namespace
{
	using crag::core::HashString;
	
	// the resources that must be created and destroyed on gfx thread
	HashString gl_resources[] = 
	{
		HashString("PolyProgram"),
		HashString("ShadowProgram"),
		HashString("ScreenProgram"),
		HashString("SphereProgram"),
		HashString("DiskProgram"),
		HashString("SkyboxProgram"),
		HashString("SpriteProgram"),
		HashString("CuboidVbo"),
		HashString("CuboidLitMesh"),
		HashString("SphereQuadVbo"),
		HashString("QuadVbo")
	};
	
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
		
		CRAG_VERIFY(cuboid);
		return cuboid;
	}

	PlainMesh CreatePlainCuboid()
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
		
		PlainMesh cuboid(num_vertices, num_indices);
		
		std::copy(* * vertices, * * vertices + num_vertices, cuboid.GetVertices().data());
		std::copy(* * * indices, * * * indices + num_indices, cuboid.GetIndices().data());
		
		CRAG_VERIFY(cuboid);
		return cuboid;
	}

	void RegisterModels()
	{
		auto & manager = crag::core::ResourceManager::Get();
	
		manager.Register<PlainMesh>("CuboidPlainMesh", CreatePlainCuboid);
		manager.Register<LitMesh>("CuboidLitMesh", CreateLitCuboid);
	}

	void RegisterShaders()
	{
		auto & manager = crag::core::ResourceManager::Get();
	
		static char const * common_shader_filename = "assets/glsl/common.glsl";
		static char const * light_common_shader_filename = "assets/glsl/light_common.glsl";
		static char const * light_fg_solid_filename = "assets/glsl/light_fg_solid.glsl";
		static char const * light_fg_soft_filename = "assets/glsl/light_fg_soft.glsl";
		static char const * light_bg_filename = "assets/glsl/light_bg.glsl";

		manager.Register<PolyProgram>("PolyProgram", []()
		{
			return MakeProgram<PolyProgram>(
				{ common_shader_filename, light_common_shader_filename, light_fg_solid_filename, "assets/glsl/poly.vert" },
				{ common_shader_filename, light_common_shader_filename, light_fg_solid_filename, "assets/glsl/poly.frag" });
		});

		manager.Register<ShadowProgram>("ShadowProgram", [] ()
		{
			return MakeProgram<ShadowProgram>(
				{ common_shader_filename, "assets/glsl/shadow.vert" },
				{ common_shader_filename, "assets/glsl/shadow.frag" });
		});

		manager.Register<ScreenProgram>("ScreenProgram", [] ()
		{
			return MakeProgram<ScreenProgram>(
				{ common_shader_filename, "assets/glsl/screen.vert" },
				{ common_shader_filename, "assets/glsl/screen.frag" });
		});

		manager.Register<DiskProgram>("SphereProgram", [] ()
		{
			return MakeProgram<DiskProgram>(
				{ common_shader_filename, light_common_shader_filename, light_fg_solid_filename, "assets/glsl/sphere.vert" },
				{ common_shader_filename, light_common_shader_filename, light_fg_solid_filename, "assets/glsl/sphere.frag" });
		});

		manager.Register<DiskProgram>("DiskProgram", [] ()
		{
			return MakeProgram<DiskProgram>(
				{ common_shader_filename, light_common_shader_filename, light_fg_soft_filename, "assets/glsl/disk.vert" },
				{ common_shader_filename, light_common_shader_filename, light_fg_soft_filename, "assets/glsl/disk.frag" });
		});

		manager.Register<TexturedProgram>("SkyboxProgram", [] ()
		{
			return MakeProgram<TexturedProgram>(
				{ common_shader_filename, light_common_shader_filename, light_bg_filename, "assets/glsl/skybox.vert" },
				{ common_shader_filename, light_common_shader_filename, light_bg_filename, "assets/glsl/skybox.frag" });
		});

		manager.Register<SpriteProgram>("SpriteProgram", [] ()
		{
			return MakeProgram<SpriteProgram>(
				{ common_shader_filename, "assets/glsl/sprite.vert" },
				{ common_shader_filename, "assets/glsl/sprite.frag" });
		});
	}
	
	bool RegisterVbos()
	{
		auto & manager = crag::core::ResourceManager::Get();

		manager.Register<LitVboResource>("CuboidVbo", [] ()
		{
			auto & manager = crag::core::ResourceManager::Get();
			auto const & lit_cuboid_mesh = * manager.GetHandle<LitMesh>("CuboidLitMesh");
			return LitVboResource(lit_cuboid_mesh);
		});
	
		manager.Register<Quad>("SphereQuadVbo", [] ()
		{
			return Quad(-1);
		});
	
		manager.Register<Quad>("QuadVbo", [] ()
		{
			return Quad(0);
		});
	
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////
// external definitions

void gfx::RegisterResources()
{
	RegisterModels();
	RegisterShaders();
	RegisterVbos();
	
}

void gfx::LoadResources()
{
	// if programs and VBOs are not made now,
	// their binding will clash with Engine's management of binding
	auto & manager = crag::core::ResourceManager::Get();
	for (auto const & key : gl_resources)
	{
		manager.Load(key);
	}
}
	
void gfx::UnloadResources()
{
	// if programs and VBOs are not made now,
	// their binding will clash with Engine's management of binding
	auto & manager = crag::core::ResourceManager::Get();
	for (auto const & key : gl_resources)
	{
		manager.Unload(key);
	}
}
