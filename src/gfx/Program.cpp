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

Program::Program()
: _id(0)
, _uniforms_valid(false)
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

bool Program::IsBound() const
{
	return unsigned(GetInt<GL_CURRENT_PROGRAM>()) == _id;
}

bool Program::Init(char const * const * vert_sources, char const * const * frag_sources)
{
	assert(! IsInitialized());

	// Create the program.
	_id = glCreateProgram();

	// Create the main vert and frag shaders.
	if (! _vert_shader.Init(vert_sources, GL_VERTEX_SHADER))
	{
		return false;
	}
	
	if (! _frag_shader.Init(frag_sources, GL_FRAGMENT_SHADER))
	{
		return false;
	}
	
	GL_CALL(glAttachShader(_id, _vert_shader._id));
	GL_CALL(glAttachShader(_id, _frag_shader._id));
	
	InitAttribs(_id);
	
	glLinkProgram(_id);
	
	if (! IsLinked())
	{
		ERROR_MESSAGE("Failed to link program including vert shader '%s'.", vert_sources[0]);
		
#if ! defined(NDEBUG)
		std::string info_log;
		GetInfoLog(info_log);
		DEBUG_BREAK("Shader info log: \n%s", info_log.c_str());
#endif
	}
	
	Bind();
	
	_projection_matrix_location = GetUniformLocation("projection_matrix");
	_model_view_matrix_location = GetUniformLocation("model_view_matrix");
	InitUniforms();
	
	for (auto i = 0; i < MAX_LIGHTS; ++ i)
	{
		char name[40];
		
		sprintf(name, "lights[%d].position", i);
		_light_block.lights[i].position = glGetUniformLocation(_id, name);
		
		sprintf(name, "lights[%d].color", i);
		_light_block.lights[i].color = glGetUniformLocation(_id, name);
	}
	
	Unbind();
	return true;
}

void Program::Deinit()
{
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
	ASSERT(IsBound());
	GL_CALL(glUseProgram(0));
}

void Program::SetUniformsValid(bool uniforms_valid)
{
	_uniforms_valid = uniforms_valid;
}

bool Program::GetUniformsValid() const
{
	return _uniforms_valid;
}

void Program::UpdateLights(Light::List const & lights) const
{
	ASSERT(IsBound());

	LightBlock const & light_block = _light_block;
	LightBlock::Light const * uniforms = light_block.lights, * uniforms_end = uniforms + MAX_LIGHTS;
	for (Light::List::const_iterator i = lights.begin(), end = lights.end(); i != end; ++ uniforms, ++ i)
	{
		Light const & light = * i;
		if (uniforms == uniforms_end)
		{
			// We're out of light uniforms. Time to start prioritizing perhaps.
			DEBUG_BREAK("light limit, %d, exceeded", MAX_LIGHTS);
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

void Program::SetProjectionMatrix(Matrix44 const & projection_matrix) const
{
	GL_CALL(glUniformMatrix4fv(_projection_matrix_location, 1, GL_FALSE, projection_matrix.GetArray()));
}

void Program::SetModelViewMatrix(Matrix44 const & model_view_matrix) const
{
	GL_CALL(glUniformMatrix4fv(_model_view_matrix_location, 1, GL_FALSE, model_view_matrix.GetArray()));
}

GLint Program::GetUniformLocation(char const * name) const
{
	return glGetUniformLocation(_id, name);
}

void Program::InitAttribs(GLuint /*id*/)
{
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
// PolyProgram member definitions

PolyProgram::PolyProgram()
: _fragment_lighting_location(-1)
, _flat_shade_location(-1)
{
}

void PolyProgram::SetUniforms(Color4f const & color, bool fragment_lighting, bool flat_shade) const
{
	ASSERT(IsBound());
	GL_CALL(glUniform4f(_color_location, color.r, color.g, color.b, color.a));
	GL_CALL(glUniform1i(_fragment_lighting_location, fragment_lighting));
	GL_CALL(glUniform1i(_flat_shade_location, flat_shade));
}

void PolyProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_normal"));
}

void PolyProgram::InitUniforms()
{
	ASSERT(IsBound());
	GL_VERIFY;

	_color_location = GetUniformLocation("color");
	_fragment_lighting_location = GetUniformLocation("fragment_lighting");
	_flat_shade_location = GetUniformLocation("flat_shade");

	GL_VERIFY;
}

////////////////////////////////////////////////////////////////////////////////
// DiskProgram member definitions

DiskProgram::DiskProgram()
: _color_location(-1)
, _center_location(-1)
, _radius_location(-1)
{
}

void DiskProgram::SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color) const
{
	GL_CALL(glUniform4f(_color_location, color.r, color.g, color.b, color.a));
	
	geom::Vector4f center = geom::Vector4f(0,0,0,1) * model_view.GetOpenGlMatrix();
	GL_CALL(glUniform3f(_center_location, center.x, center.y, center.z));
	
	float radius = static_cast<float>(CalculateRadius(model_view));
	GL_CALL(glUniform1f(_radius_location, radius));
}

void DiskProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_normal"));
}

void DiskProgram::InitUniforms()
{
	_color_location = GetUniformLocation("color");
	_center_location = GetUniformLocation("center");
	_radius_location = GetUniformLocation("radius");
}

Scalar DiskProgram::CalculateRadius(geom::Transformation<float> const & transformation)
{
	Vector3 size = transformation.GetScale();
	ASSERT(NearEqual(size.x / size.y, 1, Scalar(0.0001)));
	ASSERT(NearEqual(size.y / size.z, 1, Scalar(0.0001)));
	ASSERT(NearEqual(size.z / size.x, 1, Scalar(0.0001)));
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
	DiskProgram::SetUniforms(model_view, color);
	
	glUniform1f(_density_location, density);
}

void FogProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_normal"));
}

void FogProgram::InitUniforms()
{
	_density_location = GetUniformLocation("density");
}

////////////////////////////////////////////////////////////////////////////////
// TexturedProgram member definitions

void TexturedProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_tex_coord"));
}

////////////////////////////////////////////////////////////////////////////////
// SpriteProgram member definitions

void SpriteProgram::SetUniforms(geom::Vector2i const & resolution) const
{
	auto resolution_f = geom::Cast<float>(resolution);
	GL_CALL(glUniform2f(_position_scale_location, 1.f / resolution_f.x, - 1.f / resolution_f.y));
	GL_CALL(glUniform2f(_position_offset_location, -.5f, .5f));
}

void SpriteProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_tex_coord"));
}

void SpriteProgram::InitUniforms()
{
	_position_scale_location = GetUniformLocation("position_scale");
	_position_offset_location = GetUniformLocation("position_offset");
}
