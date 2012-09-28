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

#include "glHelpers.h"

#include "core/app.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Program member definitions

gfx::Program::Program()
: _id(0)
, _lights_changed(true)
{
}

Program::~Program()
{
	ASSERT(! IsInitialized());
}

bool Program::IsInitialized() const
{
	return _id != 0;
}

bool Program::IsLinked() const
{
	assert(IsInitialized());
	
	GLint params;
	GL_CALL(glGetProgramiv(_id, GL_LINK_STATUS, & params));
	
	return params != GL_FALSE;
}

void Program::Init(char const * vert_source, char const * frag_source, Shader & light_vert_shader, Shader & light_frag_shader)
{
	assert(! IsInitialized());

	// Create the program.
	_id = glCreateProgram();

	// Create the main vert and frag shaders.
	_vert_shader.Init(vert_source, GL_VERTEX_SHADER);
	_frag_shader.Init(frag_source, GL_FRAGMENT_SHADER);
	
	glAttachShader(_id, _vert_shader._id);
	glAttachShader(_id, _frag_shader._id);
	glAttachShader(_id, light_vert_shader._id);
	glAttachShader(_id, light_frag_shader._id);
	glLinkProgram(_id);
	
	if (! IsLinked())
	{
		ERROR_MESSAGE("Failed to link program including vert shader '%s'.", vert_source);
		
#if ! defined(NDEBUG)
		std::string info_log;
		GetInfoLog(info_log);
		DEBUG_BREAK("Shader info log: \n%s", info_log.c_str());
#endif
	}
	
	Bind();
	
	InitUniforms();
	
	for (int i = 0; i < MAX_LIGHTS; ++ i)
	{
		char name[40];
		
		sprintf(name, "lights[%d].position", i);
		_light_block.lights[i].position = glGetUniformLocation(_id, name);
		
		sprintf(name, "lights[%d].color", i);
		_light_block.lights[i].color = glGetUniformLocation(_id, name);
	}
	
	Unbind();
}

void Program::Deinit(Shader & light_vert_shader, Shader & light_frag_shader)
{
	glDetachShader(_id, light_vert_shader._id);
	glDetachShader(_id, light_frag_shader._id);
	glDetachShader(_id, _frag_shader._id);
	glDetachShader(_id, _vert_shader._id);
	
	_frag_shader.Deinit();
	_vert_shader.Deinit();
	
	glDeleteProgram(_id);
	_id = 0;
}

void Program::Bind() const
{
	ASSERT(GetInt<GL_CURRENT_PROGRAM>() == 0);
	GL_CALL(glUseProgram(_id));
}

void Program::Unbind() const
{
	ASSERT(GetInt<GL_CURRENT_PROGRAM>() == static_cast<int>(_id));
	GL_CALL(glUseProgram(0));
}

void Program::OnLightsChanged() const
{
	_lights_changed = true;
}

void Program::UpdateLights(Light::List const & lights) const
{
	if (! _lights_changed)
	{
		return;
	}
	
	_lights_changed = false;
	
	LightBlock const & light_block = _light_block;
	LightBlock::Light const * uniforms = light_block.lights, * uniforms_end = uniforms + MAX_LIGHTS;
	for (Light::List::const_iterator i = lights.begin(), end = lights.end(); i != end; ++ uniforms, ++ i)
	{
		Light const & light = * i;
		if (uniforms == uniforms_end)
		{
			// We're out of light uniforms. Time to start prioritizing perhaps.
			ASSERT(false);
			break;
		}
		
		geom::Vector4f position = geom::Vector4f(0,0,0,1) * light.GetModelViewTransformation().GetOpenGlMatrix();
		Color4f const & color = light.GetColor();
		
		glUniform3f(uniforms->position, position.x, position.y, position.z);
		glUniform3f(uniforms->color, color.r, color.g, color.b);
	}
	
	// Make sure any unused light uniforms are blacked out.
	while (uniforms != uniforms_end)
	{
		glUniform3f(uniforms->color, 0, 0, 0);
		++ uniforms;
	}
}

GLint Program::GetUniformLocation(char const * name) const
{
	return glGetUniformLocation(_id, name);
}

void Program::InitUniforms()
{
}

void Program::GetInfoLog(std::string & info_log) const
{
	GLint length;
	GL_CALL(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, & length));
	
	if (length == 0)
	{
		info_log.clear();
	}
	else
	{
		info_log.resize(length);
		glGetProgramInfoLog(_id, length, nullptr, & info_log[0]);
	}
}

void Program::Verify() const
{
	if (_id == 0)
	{
		return;
	}
	
	assert(glIsProgram(_id));
}


////////////////////////////////////////////////////////////////////////////////
// SphereProgram member definitions

SphereProgram::SphereProgram()
: _color_location(-1)
, _center_location(-1)
, _radius_location(-1)
{
}

void SphereProgram::SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color) const
{
	glUniform4f(_color_location, color.r, color.g, color.b, color.a);
	
	geom::Vector4f center = geom::Vector4f(0,0,0,1) * model_view.GetOpenGlMatrix();
	glUniform3f(_center_location, center.x, center.y, center.z);
	
	float radius = static_cast<float>(CalculateRadius(model_view));
	glUniform1f(_radius_location, radius);
}

void SphereProgram::InitUniforms()
{
	_color_location = GetUniformLocation("color");
	_center_location = GetUniformLocation("center");
	_radius_location = GetUniformLocation("radius");
}

Scalar SphereProgram::CalculateRadius(geom::Transformation<float> const & transformation)
{
	Vector3 size = transformation.GetScale();
	ASSERT(NearEqual(size.x / size.y, 1, 0.0001));
	ASSERT(NearEqual(size.y / size.z, 1, 0.0001));
	ASSERT(NearEqual(size.z / size.x, 1, 0.0001));
	Scalar radius = size.x;
	return radius;
}


////////////////////////////////////////////////////////////////////////////////
// FogProgram member definitions

FogProgram::FogProgram()
: _density_location(-1)
{
}

void FogProgram::SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color, float density) const
{
	SphereProgram::SetUniforms(model_view, color);
	
	glUniform1f(_density_location, density);
}

void FogProgram::InitUniforms()
{
	_density_location = GetUniformLocation("density");
}
