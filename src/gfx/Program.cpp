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

Program::Program(Program && rhs)
: _id(rhs._id)
, _vert_shader(std::move(rhs._vert_shader))
, _frag_shader(std::move(rhs._frag_shader))
{
	rhs._id = 0;
}

Program::Program(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: _id(glCreateProgram())	// Create the program.
{
	// Create the main vert and frag shaders.
	if (! _vert_shader.Init(vert_sources, GL_VERTEX_SHADER))
	{
		DEBUG_BREAK("fatal vertex shader error");
		return;
	}

	if (! _frag_shader.Init(frag_sources, GL_FRAGMENT_SHADER))
	{
		DEBUG_BREAK("fatal fragment shader error");
		return;
	}

	GL_CALL(glAttachShader(_id, _vert_shader._id));
	GL_CALL(glAttachShader(_id, _frag_shader._id));
	
#if defined(DUMP_GLSL_ERRORS)
	std::string info_log;
	GetInfoLog(info_log);

	if (info_log.size() > 1)
	{
		for (auto source : vert_sources)
		{
			PrintMessage(stderr, "Linker output of program including vert shader '%s':\n", source);
		}
		
		PrintMessage(stderr, "%s", info_log.c_str());
	}
#endif
}

Program::~Program()
{
	if (_id == 0)
	{
		return;
	}
	
	glDetachShader(_id, _frag_shader._id);
	glDetachShader(_id, _vert_shader._id);
	
	_frag_shader.Deinit();
	_vert_shader.Deinit();
	
	glDeleteProgram(_id);
	_id = 0;

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

void Program::InitUniforms()
{
}

void Program::BindAttribLocation(int index, char const * name) const
{
	GL_CALL(glBindAttribLocation(_id, index, name));
}

template <typename Type>
void Program::InitUniformLocation(Uniform<Type> & uniform, char const * name) const
{
	uniform = Uniform<Type>(_id, name);
}

void Program::Finalize()
{
	glLinkProgram(_id);
	ASSERT(IsLinked());
	
	Bind();
	InitUniforms();
	Unbind();
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

Program3d::Program3d(Program3d && rhs)
: Program(std::move(rhs))
, _projection_matrix(std::move(rhs._projection_matrix))
, _model_view_matrix(std::move(rhs._model_view_matrix))
{
}

Program3d::Program3d(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program(vert_sources, frag_sources)
{
}

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
	
	InitUniformLocation(_projection_matrix, "projection_matrix");
	InitUniformLocation(_model_view_matrix, "model_view_matrix");
}

////////////////////////////////////////////////////////////////////////////////
// LightProgram member definitions

LightProgram::LightProgram(LightProgram && rhs)
: Program3d(std::move(rhs))
, _ambient(std::move(rhs._ambient))
, _num_point_lights(std::move(rhs._num_point_lights))
, _num_beam_lights(std::move(rhs._num_beam_lights))
, _lights(std::move(rhs._lights))
{
}

LightProgram::LightProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program3d(vert_sources, frag_sources)
{
}

void LightProgram::InitUniforms()
{
	super::InitUniforms();
	
	InitUniformLocation(_ambient, "ambient");
	InitUniformLocation(_num_point_lights, "num_point_lights");
	InitUniformLocation(_num_beam_lights, "num_beam_lights");
	
	for (auto index = 0u; index != _lights.size(); ++ index)
	{
		auto & light_uniforms = _lights[index];

		char name[40];
	
		sprintf(name, "lights[%d].position", index);
		InitUniformLocation(light_uniforms.position, name);
	
		sprintf(name, "lights[%d].direction", index);
		InitUniformLocation(light_uniforms.direction, name);
	
		sprintf(name, "lights[%d].color", index);
		InitUniformLocation(light_uniforms.color, name);
	}
}

void LightProgram::SetLight(Light const & light) const
{
	ASSERT(light.GetIsLuminant());
	ASSERT(IsBound());

	SetLight(light, 0);
	
	_ambient.Set(Color4f::Black());
	
	auto type = light.GetType();
	switch (type)
	{
	default:
		DEBUG_BREAK("bad enum value, %d", int(type));
		
	case LightType::point:
	case LightType::shadow:
		_num_point_lights.Set(1);
		_num_beam_lights.Set(0);
		break;
		
	case LightType::beam:
		_num_point_lights.Set(0);
		_num_beam_lights.Set(1);
		break;
	}
}

void LightProgram::SetLights(Color4f const & ambient, Light::List const & lights, LightTypeSet filter) const
{
	ASSERT(IsBound());
	CRAG_VERIFY_EQUAL(ambient.a, 1);
	
	auto num_point_lights = 0;
	auto num_beam_lights = 0;
	auto num_lights = 0;
	
	auto populate_lights = [&] (LightType pass_type)
	{
		if (! filter[pass_type])
		{
			return 0;
		}
		
		auto num_type_lights = 0;

		for (auto & light : lights)
		{
			auto type = light.GetType();
			if (pass_type != type)
			{
				continue;
			}

			if (! light.GetIsLuminant())
			{
				continue;
			}
			
			SetLight(light, num_lights);
			++ num_lights;
			++ num_type_lights;
		}
		
		return num_type_lights;
	};
	
	// add point lights first
	num_point_lights += populate_lights(LightType::point);
	num_point_lights += populate_lights(LightType::shadow);
	
	// then beam
	num_beam_lights += populate_lights(LightType::beam);
	
	ASSERT(num_beam_lights + num_point_lights == num_lights);
	
	_ambient.Set(ambient);
	_num_point_lights.Set(num_point_lights);
	_num_beam_lights.Set(num_beam_lights);
}

void LightProgram::SetLight(Light const & light, int index) const
{
	if (unsigned(index) >= _lights.size())
	{
		DEBUG_BREAK("too many lights");
		return;
	}
	
	auto & light_uniforms = _lights[index];

	auto & transformation = light.GetModelViewTransformation();
	auto position = transformation.GetTranslation();
	light_uniforms.position.Set(position);
	
	auto direction = GetAxis(transformation.GetRotation(), Direction::forward);
	light_uniforms.direction.Set(direction);

	Color4f const & color = light.GetColor();
	light_uniforms.color.Set(color);
}

////////////////////////////////////////////////////////////////////////////////
// PolyProgram member definitions

PolyProgram::PolyProgram(PolyProgram && rhs)
: LightProgram(std::move(rhs))
, _color(std::move(rhs._color))
{
}

PolyProgram::PolyProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: LightProgram(vert_sources, frag_sources)
{
	// attribute locations
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_normal");
	BindAttribLocation(3, "vertex_color");
	BindAttribLocation(4, "vertex_height");

	Finalize();
}

void PolyProgram::SetUniforms(Color4f const & color) const
{
	ASSERT(IsBound());
	_color.Set(color);
}

void PolyProgram::InitUniforms()
{
	super::InitUniforms();

	InitUniformLocation(_color, "color");
}

////////////////////////////////////////////////////////////////////////////////
// ShadowProgram member definitions

ShadowProgram::ShadowProgram(ShadowProgram && rhs)
: Program3d(std::move(rhs))
{
}

ShadowProgram::ShadowProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program3d(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	
	Finalize();
}

////////////////////////////////////////////////////////////////////////////////
// ScreenProgram member definitions

ScreenProgram::ScreenProgram(ScreenProgram && rhs)
: Program(std::move(rhs))
{
}

ScreenProgram::ScreenProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	
	Finalize();
}

////////////////////////////////////////////////////////////////////////////////
// DiskProgram member definitions

DiskProgram::DiskProgram(DiskProgram && rhs)
: LightProgram(std::move(rhs))
, _color(std::move(rhs._color))
, _center(std::move(rhs._center))
, _radius(std::move(rhs._radius))
{
}

DiskProgram::DiskProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: LightProgram(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_normal");
	
	Finalize();
}

void DiskProgram::SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const
{
	_color.Set(color);
	_center.Set(model_view.GetTranslation());
	_radius.Set(radius);
}

void DiskProgram::InitUniforms()
{
	super::InitUniforms();

	InitUniformLocation(_color, "color");
	InitUniformLocation(_center, "center");
	InitUniformLocation(_radius, "radius");
}

////////////////////////////////////////////////////////////////////////////////
// TexturedProgram member definitions

TexturedProgram::TexturedProgram(TexturedProgram && rhs)
: Program3d(std::move(rhs))
{
}

TexturedProgram::TexturedProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program3d(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_tex_coord");

	Finalize();
}

////////////////////////////////////////////////////////////////////////////////
// SpriteProgram member definitions

SpriteProgram::SpriteProgram(SpriteProgram && rhs)
: Program(std::move(rhs))
, _position_scale(std::move(rhs._position_scale))
, _position_offset(std::move(rhs._position_offset))
{
}

SpriteProgram::SpriteProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_tex_coord");
	
	Finalize();
}

void SpriteProgram::SetUniforms(geom::Vector2i const & resolution) const
{
	auto resolution_f = geom::Cast<float>(resolution);
	_position_scale.Set(Vector2(1.f / resolution_f.x, - 1.f / resolution_f.y));
	_position_offset.Set(Vector2(-.5f, .5f));
}

void SpriteProgram::InitUniforms()
{
	InitUniformLocation(_position_scale, "position_scale");
	InitUniformLocation(_position_offset, "position_offset");
}
