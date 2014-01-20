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

#include "axes.h"

#include "core/app.h"

#if ! defined(NDEBUG)
#define DUMP_GLSL_ERRORS
#endif

using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Program member definitions

Program::Program()
: _id(0)
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
	
#if defined(DUMP_GLSL_ERRORS)
	std::string info_log;
	GetInfoLog(info_log);

	if (! info_log.empty())
	{
		PrintMessage(stderr, "Linker output of program including vert shader '%s':\n", vert_sources[0]);
		
		PrintMessage(stderr, "%s", info_log.c_str());
	}
#endif
	
	if (! IsLinked())
	{
		DEBUG_BREAK("Failed to link program including vert shader '%s'.", vert_sources[0]);
	}

	Bind();
	
	_projection_matrix_location = GetUniformLocation("projection_matrix");
	_model_view_matrix_location = GetUniformLocation("model_view_matrix");
	InitUniforms();
	
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

void Program::SetProjectionMatrix(Matrix44 const & projection_matrix) const
{
#if defined(CRAG_USE_GLES)
	GL_CALL(glUniformMatrix4fv(_projection_matrix_location, 1, GL_FALSE, geom::Transposition(projection_matrix).GetArray()));
#elif defined(CRAG_USE_GL)
	GL_CALL(glUniformMatrix4fv(_projection_matrix_location, 1, GL_TRUE, projection_matrix.GetArray()));
#endif
}

void Program::SetModelViewMatrix(Matrix44 const & model_view_matrix) const
{
#if defined(CRAG_USE_GLES)
	GL_CALL(glUniformMatrix4fv(_model_view_matrix_location, 1, GL_FALSE, geom::Transposition(ToOpenGl(model_view_matrix)).GetArray()));
#elif defined(CRAG_USE_GL)
	GL_CALL(glUniformMatrix4fv(_model_view_matrix_location, 1, GL_TRUE, ToOpenGl(model_view_matrix).GetArray()));
#endif
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
// LightProgram member definitions

void LightProgram::InitUniforms()
{
	ASSERT(IsBound());
	GL_VERIFY;
	
	Program::InitUniforms();
	
	_ambient_location = GetUniformLocation("ambient");
	_num_lights_location = glGetUniformLocation(_id, "num_lights");

	GL_VERIFY;
}

void LightProgram::SetLight(Light const & light)
{
	ASSERT(IsBound());

	SetAmbient(Color4f::Black());
	
	SetLight(light, 0);
	
	glUniform1i(_num_lights_location, 1);
}

void LightProgram::SetLights(Color4f const & ambient, Light::List const & lights, LightType filter)
{
	ASSERT(IsBound());

	SetAmbient(ambient);
	
	auto num_lights = 0;
	for (auto & light : lights)
	{
		if (filter != LightType::all)
		{
			auto type = light.GetType();
			if (type != filter)
			{
				continue;
			}
		}
		
		SetLight(light, num_lights);
		++ num_lights;
	}
	
	glUniform1i(_num_lights_location, num_lights);
}

void LightProgram::SetLight(Light const & light, int index)
{
	while (unsigned(index) >= _light_locations.size())
	{
		AddLight();
	}
	
	auto & location = _light_locations[index];

	auto & transformation = light.GetModelViewTransformation();
	geom::Vector3f position = ToOpenGl(transformation.GetTranslation());
	GL_CALL(glUniform3f(location.position, position.x, position.y, position.z));

	Color4f const & color = light.GetColor();
	GL_CALL(glUniform3f(location.color, color.r, color.g, color.b));
}

void LightProgram::AddLight()
{
	LightLocation additional;
	{
		auto i = int(_light_locations.size());
		char name[40];
	
		sprintf(name, "lights[%d].position", i);
		additional.position = glGetUniformLocation(_id, name);
	
		sprintf(name, "lights[%d].color", i);
		additional.color = glGetUniformLocation(_id, name);
	}

	_light_locations.push_back(additional);
}

void LightProgram::SetAmbient(Color4f const & ambient) const
{
	CRAG_VERIFY_EQUAL(ambient.a, 1);
	
	GL_CALL(glUniform4f(_ambient_location, ambient.r, ambient.g, ambient.b, ambient.a));
}

////////////////////////////////////////////////////////////////////////////////
// PolyProgram member definitions

PolyProgram::PolyProgram()
: _color_location(-1)
, _fragment_lighting_location(-1)
, _flat_shade_location(-1)
, _relief_enabled_location(-1)
{
}

void PolyProgram::SetUniforms(Color4f const & color, bool fragment_lighting, bool flat_shade, bool relief_enabled) const
{
	ASSERT(IsBound());
	GL_CALL(glUniform4f(_color_location, color.r, color.g, color.b, color.a));
	GL_CALL(glUniform1i(_fragment_lighting_location, fragment_lighting));
	GL_CALL(glUniform1i(_flat_shade_location, flat_shade));
	GL_CALL(glUniform1i(_relief_enabled_location, relief_enabled));
}

void PolyProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_normal"));
	GL_CALL(glBindAttribLocation(id, 3, "vertex_color"));
	GL_CALL(glBindAttribLocation(id, 4, "vertex_height"));
}

void PolyProgram::InitUniforms()
{
	ASSERT(IsBound());
	GL_VERIFY;
	
	super::InitUniforms();

	_color_location = GetUniformLocation("color");
	_fragment_lighting_location = GetUniformLocation("fragment_lighting");
	_flat_shade_location = GetUniformLocation("flat_shade");
	_relief_enabled_location = GetUniformLocation("relief_enabled");
	
	GL_VERIFY;
}

////////////////////////////////////////////////////////////////////////////////
// ShadowProgram member definitions

ShadowProgram::ShadowProgram()
{
}

void ShadowProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
}

////////////////////////////////////////////////////////////////////////////////
// ScreenProgram member definitions

ScreenProgram::ScreenProgram()
{
}

void ScreenProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
}

////////////////////////////////////////////////////////////////////////////////
// DiskProgram member definitions

DiskProgram::DiskProgram()
: _color_location(-1)
, _center_location(-1)
, _radius_location(-1)
{
}

void DiskProgram::SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const
{
	GL_CALL(glUniform4f(_color_location, color.r, color.g, color.b, color.a));
	
	geom::Vector3f center = ToOpenGl(model_view.GetTranslation());
	GL_CALL(glUniform3f(_center_location, center.x, center.y, center.z));
	
	GL_CALL(glUniform1f(_radius_location, radius));
}

void DiskProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_normal"));
}

void DiskProgram::InitUniforms()
{
	super::InitUniforms();

	_color_location = GetUniformLocation("color");
	_center_location = GetUniformLocation("center");
	_radius_location = GetUniformLocation("radius");
}

////////////////////////////////////////////////////////////////////////////////
// FogProgram member definitions

FogProgram::FogProgram()
: _density_location(-1)
{
}

void FogProgram::SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color, float radius, float density) const
{
	DiskProgram::SetUniforms(model_view, radius, color);
	
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
