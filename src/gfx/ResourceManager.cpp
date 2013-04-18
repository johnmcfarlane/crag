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
: _light_vert_shader(nullptr)
, _light_frag_shader(nullptr)
, _cuboid(nullptr)
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
		Program & program = * _programs[program_index];
		program.Deinit(* _light_vert_shader, * _light_frag_shader);
	}
	
	for (int program_index = 0; program_index != int(ProgramIndex::size); ++ program_index)
	{
		Program * & program = _programs[program_index];
		delete program;
		program = nullptr;
	}
	
	_light_frag_shader->Deinit();
	delete _light_frag_shader;
	_light_frag_shader = nullptr;
	
	_light_vert_shader->Deinit();
	delete _light_vert_shader;
	_light_vert_shader = nullptr;
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
	_light_vert_shader = new Shader;
	_light_vert_shader->Init("glsl/light.frag", GL_VERTEX_SHADER);

	_light_frag_shader = new Shader;
	_light_frag_shader->Init("glsl/light.frag", GL_FRAGMENT_SHADER);

	auto init_program = [&] (Program * program, ProgramIndex index, char const * vert_filename, char const * frag_filename)
	{
		if (program != nullptr)
		{
			program->Init(vert_filename, frag_filename, * _light_vert_shader, * _light_frag_shader);
		}
		
		_programs[int(index)] = program;
	};

	init_program(new PolyProgram, ProgramIndex::poly, "glsl/poly.vert", "glsl/poly.frag");
	init_program(new DiskProgram, ProgramIndex::sphere, "glsl/disk.vert", "glsl/sphere.frag");
	init_program(new FogProgram, ProgramIndex::fog, "glsl/disk.vert", "glsl/fog.frag");
	init_program(new DiskProgram, ProgramIndex::disk, "glsl/disk.vert", "glsl/disk.frag");
	init_program(new TexturedProgram, ProgramIndex::skybox, "glsl/skybox.vert", "glsl/skybox.frag");
	init_program(new TexturedProgram, ProgramIndex::textured, "glsl/textured.vert", "glsl/textured.frag");

	return true;
}

bool ResourceManager::InitGeometry()
{
	_cuboid = new Cuboid;
	_sphere_quad = new Quad(-1);
	_disk_quad = new Quad(0);
	
	return true;
}
