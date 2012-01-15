//
//  Program.cpp
//  crag
//
//  Created by John McFarlane on 2011-01-09.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Program.h"

#include "core/app.h"


bool gfx::InitShader(gl::Shader & shader, char const * filename, GLenum shader_type)
{
	std::vector<char> sphere_shader_source;

	if (! app::LoadFile(filename, sphere_shader_source))
	{
		AssertErrno();
		return false;
	}

	int sphere_shader_source_size = sphere_shader_source.size();
	
	// Create the shader.
	gl::Create(shader, shader_type);
	gl::Source(shader, & sphere_shader_source[0], & sphere_shader_source_size);
	gl::Compile(shader);
	
	// Check for errors in the source code.
#if ! defined(NDEBUG)
	if (! shader.IsCompiled())
	{
		std::cerr << "Failed to compile shader, \"" << filename << "\":" << std::endl;
		
		std::string info_log;
		shader.GetInfoLog(info_log);
		std::cerr << info_log;
		assert(false);
	}
#endif
	
	return true;
}


void gfx::Program::Init(char const * vert_source, char const * frag_source, gl::Shader & light_shader)
{
	InitShader(_vert_shader, vert_source, GL_VERTEX_SHADER);
	InitShader(_frag_shader, frag_source, GL_FRAGMENT_SHADER);

	// Create the program.
	gl::Create(* this);
	gl::Attach(* this, _vert_shader);
	gl::Attach(* this, _frag_shader);
	gl::Attach(* this, light_shader);
	gl::Link(* this);
	
#if ! defined(NDEBUG)
	if (! IsLinked())
	{
		std::cerr << "Failed to link program including vert shader " << vert_source << '.' << std::endl;
		
		std::string info_log;
		GetInfoLog(info_log);
		std::cerr << info_log;
		assert(false);
	}
#endif
	
	gl::Use(* this);
	
	for (int i = 0; i < MAX_LIGHTS; ++ i)
	{
		char name[40];
		
		sprintf(name, "lights[%d].position", i);
		_light_block.lights[i].position = gl::GetUniformLocation(* this, name);
		
		sprintf(name, "lights[%d].color", i);
		_light_block.lights[i].color = gl::GetUniformLocation(* this, name);
	}
	
	gl::Disuse(* this);
}

void gfx::Program::Deinit(gl::Shader & light_shader)
{
	gl::Detach(* this, light_shader);
	gl::Detach(* this, _frag_shader);
	gl::Detach(* this, _vert_shader);
	gl::Delete(* this);
	gl::Delete(_vert_shader);
	gl::Delete(_frag_shader);
}

void gfx::Program::Use() const
{
	gl::Use(* this);
}

void gfx::Program::Disuse() const
{
	gl::Disuse(* this);
}

void gfx::Program::UpdateLights(Light::List const & lights) const
{
	LightBlock const & light_block = _light_block;
	LightBlock::Light const * uniforms = light_block.lights, * uniforms_end = uniforms + MAX_LIGHTS;
	for (Light::List::const_iterator i = lights.begin(), end = lights.end(); i != end; ++ uniforms, ++ i)
	{
		Light const & light = * i;
		if (uniforms == uniforms_end)
		{
			// We're out of light uniforms. Time to start prioritizing perhaps.
			Assert(false);
			break;
		}
		
		Vector4f position = Vector4f(0,0,0,1) * light.GetModelViewTransformation().GetOpenGlMatrix();
		Color4f const & color = light.GetColor();
		
		gl::Uniform<float, 3>(uniforms->position, position.GetAxes());
		gl::Uniform<float, 3>(uniforms->color, color.GetArray());
	}
	
	// Make sure any unused light uniforms are blacked out.
	while (uniforms != uniforms_end)
	{
		gl::Uniform<float, 3>(uniforms->color, Color4f::Black().GetArray());
		++ uniforms;
	}
}
