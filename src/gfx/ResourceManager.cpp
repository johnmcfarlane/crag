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

#include "Cuboid.h"
#include "Program.h"
#include "Quad.h"
#include "Shader.h"


using namespace gfx;


ResourceManager::ResourceManager()
: _cuboid(nullptr)
, _sphere_quad(nullptr)
, _disk_quad(nullptr)
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
	delete _disk_quad;
	_disk_quad = nullptr;
	
	delete _sphere_quad;
	_sphere_quad = nullptr;
	
	delete _cuboid;
	_cuboid = nullptr;
	
	for (int program_index = 0; program_index != int(ProgramIndex::size); ++ program_index)
	{
		auto & program = * _programs[program_index];
		program.Deinit();
		delete _programs[program_index];
		_programs[program_index] = nullptr;
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

Cuboid const & ResourceManager::GetCuboid() const
{
	return ref(_cuboid);
}

Quad const & ResourceManager::GetSphereQuad() const
{
	return ref(_sphere_quad);
}

Quad const & ResourceManager::GetDiskQuad() const
{
	return ref(_disk_quad);
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
	init_program(new DiskProgram, ProgramIndex::sphere, "assets/glsl/disk.vert", "assets/glsl/sphere.frag", nullptr);
	init_program(new FogProgram, ProgramIndex::fog, "assets/glsl/disk.vert", "assets/glsl/fog.frag", nullptr);
	init_program(new DiskProgram, ProgramIndex::disk, "assets/glsl/disk.vert", "assets/glsl/disk.frag", nullptr);
	init_program(new TexturedProgram, ProgramIndex::skybox, "assets/glsl/skybox.vert", "assets/glsl/skybox.frag", nullptr);
	init_program(new SpriteProgram, ProgramIndex::sprite, "assets/glsl/sprite.vert", "assets/glsl/sprite.frag", nullptr);

	return true;
}

bool ResourceManager::InitGeometry()
{
	_cuboid = new Cuboid;
	_sphere_quad = new Quad(-1);
	_disk_quad = new Quad(0);
	
	return true;
}
