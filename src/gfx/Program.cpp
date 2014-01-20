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

void Program::SetProjectionMatrix(Matrix44 const &) const
{
}

void Program::SetModelViewMatrix(Matrix44 const &) const
{
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
// Program3d member definitions

void Program3d::SetProjectionMatrix(Matrix44 const & projection_matrix) const
{
	_projection_matrix.Set(projection_matrix);
}

void Program3d::SetModelViewMatrix(Matrix44 const & model_view_matrix) const
{
	_model_view_matrix.Set(ToOpenGl(model_view_matrix));
}

void Program3d::InitUniforms()
{
	ASSERT(IsBound());
	GL_VERIFY;
	
	super::InitUniforms();
	
	_projection_matrix = Uniform<Matrix44>(_id, "projection_matrix");
	_model_view_matrix = Uniform<Matrix44>(_id, "model_view_matrix");

	GL_VERIFY;
}

////////////////////////////////////////////////////////////////////////////////
// LightProgram member definitions

void LightProgram::InitUniforms()
{
	ASSERT(IsBound());
	GL_VERIFY;
	
	super::InitUniforms();
	
	if (! _ambient.IsInitialized())
	{
		_ambient = Uniform<Color4f>(_id, "ambient");
	}
	
	if (! _num_lights.IsInitialized())
	{
		_num_lights = Uniform<int>(_id, "num_lights");
	}
	
	GL_VERIFY;
}

void LightProgram::SetLight(Light const & light)
{
	ASSERT(IsBound());

	SetLight(light, 0);
	
	_ambient.Set(Color4f::Black());
	_num_lights.Set(1);
}

void LightProgram::SetLights(Color4f const & ambient, Light::List const & lights, LightType filter)
{
	ASSERT(IsBound());
	CRAG_VERIFY_EQUAL(ambient.a, 1);

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
	
	_ambient.Set(ambient);
	_num_lights.Set(num_lights);
}

void LightProgram::SetLight(Light const & light, int index)
{
	while (unsigned(index) >= _lights.size())
	{
		AddLight();
	}
	
	auto & light_uniforms = _lights[index];

	auto & transformation = light.GetModelViewTransformation();
	auto position = transformation.GetTranslation();
	light_uniforms.position.Set(position);

	Color4f const & color = light.GetColor();
	light_uniforms.color.Set(color);
}

void LightProgram::AddLight()
{
	LightUniforms additional;
	{
		auto i = int(_lights.size());
		char name[40];
	
		sprintf(name, "lights[%d].position", i);
		additional.position = Uniform<Vector3>(_id, name);
	
		sprintf(name, "lights[%d].color", i);
		additional.color = Uniform<Color4f>(_id, name);
	}

	_lights.push_back(additional);
}

////////////////////////////////////////////////////////////////////////////////
// PolyProgram member definitions

PolyProgram::PolyProgram()
{
}

void PolyProgram::SetUniforms(Color4f const & color, bool fragment_lighting, bool flat_shade, bool relief_enabled) const
{
	ASSERT(IsBound());
	_color.Set(color);
	_fragment_lighting.Set(fragment_lighting);
	_flat_shade.Set(flat_shade);
	_relief_enabled.Set(relief_enabled);
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

	_color = Uniform<Color4f>(_id, "color");
	_fragment_lighting = Uniform<bool>(_id, "fragment_lighting");
	_flat_shade = Uniform<bool>(_id, "flat_shade");
	_relief_enabled = Uniform<bool>(_id, "relief_enabled");
	
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
{
}

void DiskProgram::SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const
{
	_color.Set(color);
	_center.Set(model_view.GetTranslation());
	_radius.Set(radius);
}

void DiskProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_normal"));
}

void DiskProgram::InitUniforms()
{
	super::InitUniforms();

	_color = Uniform<Color4f>(_id, "color");
	_center = Uniform<Vector3>(_id, "center");
	_radius = Uniform<float>(_id, "radius");
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
	_position_scale.Set(Vector2(1.f / resolution_f.x, - 1.f / resolution_f.y));
	_position_offset.Set(Vector2(-.5f, .5f));
}

void SpriteProgram::InitAttribs(GLuint id)
{
	GL_CALL(glBindAttribLocation(id, 1, "vertex_position"));
	GL_CALL(glBindAttribLocation(id, 2, "vertex_tex_coord"));
}

void SpriteProgram::InitUniforms()
{
	_position_scale = Uniform<Vector2>(_id, "position_scale");
	_position_offset = Uniform<Vector2>(_id, "position_offset");
}
