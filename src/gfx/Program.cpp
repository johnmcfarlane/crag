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


////////////////////////////////////////////////////////////////////////////////
// Globals

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


////////////////////////////////////////////////////////////////////////////////
// gfx::Program member definitions

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
	
	InitUniforms();
	
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

void gfx::Program::InitUniforms()
{
}


////////////////////////////////////////////////////////////////////////////////
// gfx::SphereProgram member definitions

gfx::SphereProgram::SphereProgram()
: _color_location(-1)
, _center_location(-1)
, _radius_location(-1)
{
}

void gfx::SphereProgram::SetUniforms(::Transformation<float> const & model_view, Color4f const & color) const
{
	gl::Uniform<float, 4>(_color_location, & color[0]);
	
	Vector4f center = Vector4f(0,0,0,1) * model_view.GetOpenGlMatrix();
	gl::Uniform<float, 3>(_center_location, center.GetAxes());
	
	float radius = static_cast<float>(CalculateRadius(model_view));
	gl::Uniform(_radius_location, radius);
}

void gfx::SphereProgram::InitUniforms() override
{
	_color_location = gl::GetUniformLocation(* this, "color");
	_center_location = gl::GetUniformLocation(* this, "center");
	_radius_location = gl::GetUniformLocation(* this, "radius");
}

gfx::Scalar gfx::SphereProgram::CalculateRadius(::Transformation<float> const & transformation)
{
	Vector3 size = transformation.GetScale();
	Assert(NearEqual(size.x / size.y, 1, 0.0001));
	Assert(NearEqual(size.y / size.z, 1, 0.0001));
	Assert(NearEqual(size.z / size.x, 1, 0.0001));
	Scalar radius = size.x;
	return radius;
}


////////////////////////////////////////////////////////////////////////////////
// gfx::FogProgram member definitions

gfx::FogProgram::FogProgram()
: _density_location(-1)
{
}

void gfx::FogProgram::SetUniforms(::Transformation<float> const & model_view, Color4f const & color, float density) const
{
	SphereProgram::SetUniforms(model_view, color);
	
	gl::Uniform(_density_location, density);
}

void gfx::FogProgram::InitUniforms() override
{
	_density_location = gl::GetUniformLocation(* this, "density");
}
