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
: _light_frag_shader(nullptr)
, _current_program(nullptr)
, _cuboid(nullptr)
, _sphere_quad(nullptr)
, _disk_quad(nullptr)
{
	if (! InitShaders())
	{
		Assert(false);
	}
	
	if (! InitGeometry())
	{
		Assert(false);
	}
}

ResourceManager::~ResourceManager()
{
	SetCurrentProgram(nullptr);

	delete _disk_quad;
	_disk_quad = nullptr;
	
	delete _sphere_quad;
	_sphere_quad = nullptr;
	
	delete _cuboid;
	_cuboid = nullptr;
	
	for (int program_index = 0; program_index != ProgramIndex::max_shader; ++ program_index)
	{
		Program * & program = _programs[program_index];
		program->Deinit(* _light_frag_shader);
		delete program;
		program = nullptr;
	}
	
	_light_frag_shader->Deinit();
	delete _light_frag_shader;
	_light_frag_shader = nullptr;
}

Program * ResourceManager::GetProgram(ProgramIndex::type index)
{
	Assert(index >= 0 && index < ProgramIndex::max_index);
	return _programs[index];
}

Program const * ResourceManager::GetProgram(ProgramIndex::type index) const
{
	Assert(index >= 0 && index < ProgramIndex::max_shader);
	return _programs[index];
}

void ResourceManager::SetCurrentProgram(Program * program)
{
	if (program == _current_program)
	{
		return;
	}
	
	if (_current_program != nullptr)
	{
		_current_program->Unbind();
	}
	
	_current_program = program;
	
	if (_current_program != nullptr)
	{
		_current_program->Bind();
	}
}

Program const * ResourceManager::GetCurrentProgram() const
{
	return _current_program;
}

Cuboid const & ResourceManager::GetCuboid() const
{
	return ref(_cuboid);
}

gfx::Quad const & ResourceManager::GetSphereQuad() const
{
	return ref(_sphere_quad);
}

gfx::Quad const & ResourceManager::GetDiskQuad() const
{
	return ref(_disk_quad);
}

bool ResourceManager::InitShaders()
{
	_light_frag_shader = new Shader;
	_light_frag_shader->Init("glsl/light.frag", GL_FRAGMENT_SHADER);
	
	_programs[ProgramIndex::poly] = new Program;
	_programs[ProgramIndex::poly]->Init("glsl/poly.vert", "glsl/poly.frag", * _light_frag_shader);
	
	_programs[ProgramIndex::sphere] = new SphereProgram;
	_programs[ProgramIndex::sphere]->Init("glsl/sphere.vert", "glsl/sphere.frag", * _light_frag_shader);
	
	_programs[ProgramIndex::fog] = new FogProgram;
	_programs[ProgramIndex::fog]->Init("glsl/sphere.vert", "glsl/fog.frag", * _light_frag_shader);
	
	_programs[ProgramIndex::disk] = new SphereProgram;
	_programs[ProgramIndex::disk]->Init("glsl/disk.vert", "glsl/disk.frag", * _light_frag_shader);
	
	_programs[ProgramIndex::fixed] = nullptr;
	
	return true;
}

bool ResourceManager::InitGeometry()
{
	_cuboid = new Cuboid;
	_sphere_quad = new Quad(-1);
	_disk_quad = new Quad(0);
	
	return true;
}
