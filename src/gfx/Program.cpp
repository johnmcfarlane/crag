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
, _needs_matrix_update(rhs._needs_matrix_update)
, _needs_lights_update(rhs._needs_lights_update)
{
	rhs._id = 0;
	rhs._needs_matrix_update = false;
	rhs._needs_lights_update = false;
}

Program::Program(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: _id(glCreateProgram())	// Create the program.
, _needs_matrix_update(true)
, _needs_lights_update(true)
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
	
	if (DumpInfoLog())
	{
		for (auto source : vert_sources)
		{
			PrintMessage(stderr, "Linker output of program including vert shader '%s':\n", source);
		}
	}
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
	ASSERT(IsLinked());
	ASSERT(GetInt<GL_CURRENT_PROGRAM>() == 0);

	GL_CALL(glUseProgram(_id));

	ASSERT(IsBound());
}

void Program::Unbind() const
{
	ASSERT(IsLinked());
	ASSERT(IsBound());

	GL_CALL(glUseProgram(0));
}

void Program::SetNeedsMatrixUpdate(bool needs_matrix_update) const
{
	_needs_matrix_update = needs_matrix_update;
}

bool Program::NeedsMatrixUpdate() const
{
	return _needs_matrix_update;
}

void Program::SetNeedsLightsUpdate(bool needs_lights_update) const
{
	_needs_lights_update = needs_lights_update;
}

bool Program::NeedsLightsUpdate() const
{
	return _needs_lights_update;
}

void Program::SetProjectionMatrix(Matrix44 const &) const
{
}

void Program::SetModelViewMatrix(Matrix44 const &) const
{
}

void Program::SetLights(Color4f const &, Light::List const &, LightFilter const &) const
{
}

void Program::InitUniforms()
{
	ASSERT(IsBound());
}

void Program::BindAttribLocation(int index, char const * name) const
{
	GL_CALL(glBindAttribLocation(_id, index, name));
}

template <typename Type>
void Program::InitUniformLocation(Uniform<Type> & uniform, char const * name) const
{
	uniform = std::move(Uniform<Type>(_id, name));
}

void Program::Link()
{
	ASSERT(! IsLinked());

	GL_CALL(glLinkProgram(_id));
	if (! IsLinked())
	{
		DumpInfoLog();
	}
	
	Bind();
	InitUniforms();
	Unbind();
}

bool Program::DumpInfoLog() const
{
#if defined(DUMP_GLSL_ERRORS)
	GLint length;
	GL_CALL(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, & length));
	if (length == 0)
	{
		return false;
	}

	// TODO: Consider glValidateProgram
	std::string info_log;
	info_log.resize(length);
	glGetProgramInfoLog(_id, length, nullptr, & info_log[0]);
	if (info_log.size() <= 1)
	{
		return false;
	}
	
	PrintMessage(stderr, "%s", info_log.c_str());
	return true;
#else
	return false;
#endif
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
	super::InitUniforms();
	
	InitUniformLocation(_projection_matrix, "projection_matrix");
	InitUniformLocation(_model_view_matrix, "model_view_matrix");
}

////////////////////////////////////////////////////////////////////////////////
// LightProgram::LightUniforms member definitions

LightProgram::LightUniforms::LightUniforms(LightUniforms && rhs)
: position(std::move(rhs.position))
, direction(std::move(rhs.direction))
, color(std::move(rhs.color))
, angle(std::move(rhs.angle))
, type(std::move(rhs.type))
{
}

LightProgram::LightUniforms & LightProgram::LightUniforms::operator = (LightUniforms && rhs)
{
	std::swap(position, rhs.position);
	std::swap(direction, rhs.direction);
	std::swap(color, rhs.color);
	std::swap(angle, rhs.angle);
	std::swap(type, rhs.type);

	return * this;
}

////////////////////////////////////////////////////////////////////////////////
// LightProgram member definitions

LightProgram::LightProgram(LightProgram && rhs)
: Program3d(std::move(rhs))
, _vertex_lights(std::move(rhs._vertex_lights))
, _fragment_lights(std::move(rhs._fragment_lights))
{
}

LightProgram::LightProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program3d(vert_sources, frag_sources)
, _vertex_lights(max_vertex_lights)
, _fragment_lights(max_fragment_lights)
{
}

void LightProgram::InitUniforms()
{
	super::InitUniforms();	

	auto init_lights = [this](LightUniformArray & array, char const * array_name)
	{
		auto index = 0;
		for (auto & light_uniforms : array)
		{
			auto & light_uniforms = array[index];

			int constexpr name_size = 64;
			char name[name_size];

			snprintf(name, name_size, "%s[%d].position", array_name, index);
			InitUniformLocation(light_uniforms.position, name);

			snprintf(name, name_size, "%s[%d].direction", array_name, index);
			InitUniformLocation(light_uniforms.direction, name);

			snprintf(name, name_size, "%s[%d].color", array_name, index);
			InitUniformLocation(light_uniforms.color, name);

			snprintf(name, name_size, "%s[%d].angle", array_name, index);
			InitUniformLocation(light_uniforms.angle, name);

			snprintf(name, name_size, "%s[%d].type", array_name, index);
			InitUniformLocation(light_uniforms.type, name);

			++ index;
		}
	};
	
	init_lights(_vertex_lights, "vertex_lights");
	init_lights(_fragment_lights, "fragment_lights");
}

void LightProgram::SetLights(Color4f const &, Light::List const & lights, LightFilter const & filter) const
{
	ASSERT(_vertex_lights.size() == max_vertex_lights);
	ASSERT(_fragment_lights.size() == max_fragment_lights);
	ASSERT(IsBound());

	auto set_lights = [& lights, & filter](LightUniformArray const & light_uniforms, LightResolution resolution)
	{
		auto light_uniforms_iterator = std::begin(light_uniforms);

		for (auto const & light : lights)
		{
			if (! filter(light))
			{
				// otherwise filtered out
				continue;
			}

			if (! light.GetIsLuminant())
			{
				// not doing any lighting right now
				continue;
			}

			auto attributes = light.GetAttributes();
			if (attributes.resolution != resolution)
			{
				continue;
			}
			
			if (light_uniforms_iterator == std::end(light_uniforms))
			{
				if (CRAG_DEBUG_ONCE)
				{
					DEBUG_BREAK("too many lights");
				}

				return;
			}

			auto const & transformation = light.GetModelViewTransformation();
			auto position = transformation.GetTranslation();
			light_uniforms_iterator->position.Set(position);

			auto const & direction = GetAxis(transformation.GetRotation(), Direction::forward);
			light_uniforms_iterator->direction.Set(direction);

			auto const & color = light.GetColor();
			light_uniforms_iterator->color.Set(color);

			auto const & angle = light.GetAngle();
			light_uniforms_iterator->angle.Set(angle);
		
			light_uniforms_iterator->type.Set(attributes.type != LightType::point);

			++ light_uniforms_iterator;
		}
	
		for (; light_uniforms_iterator != std::end(light_uniforms); ++ light_uniforms_iterator)
		{
			light_uniforms_iterator->type.Set(-1);
		}
	};

	set_lights(_vertex_lights, LightResolution::vertex);
	set_lights(_fragment_lights, LightResolution::fragment);
}

////////////////////////////////////////////////////////////////////////////////
// ForegroundProgram member definitions

ForegroundProgram::ForegroundProgram(ForegroundProgram && rhs)
: LightProgram(std::move(rhs))
, _ambient(std::move(rhs._ambient))
, _color(std::move(rhs._color))
{
}

ForegroundProgram::ForegroundProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: LightProgram(vert_sources, frag_sources)
{
}

void ForegroundProgram::SetUniforms(Color4f const & color) const
{
	ASSERT(IsBound());
	_color.Set(color);
}

void ForegroundProgram::InitUniforms()
{
	super::InitUniforms();

	InitUniformLocation(_ambient, "ambient");
	InitUniformLocation(_color, "color");
}

void ForegroundProgram::SetLights(Color4f const & ambient, Light::List const & lights, LightFilter const & filter) const
{
	CRAG_VERIFY_EQUAL(ambient.a, 1);
	
	_ambient.Set(ambient);

	LightProgram::SetLights(ambient, lights, filter);
}

////////////////////////////////////////////////////////////////////////////////
// PolyProgram member definitions

PolyProgram::PolyProgram(PolyProgram && rhs)
: ForegroundProgram(std::move(rhs))
{
}

PolyProgram::PolyProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: ForegroundProgram(vert_sources, frag_sources)
{
	// attribute locations
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_normal");
	BindAttribLocation(3, "vertex_color");
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
}

////////////////////////////////////////////////////////////////////////////////
// DiskProgram member definitions

DiskProgram::DiskProgram(DiskProgram && rhs)
: super(std::move(rhs))
, _center(std::move(rhs._center))
, _radius(std::move(rhs._radius))
{
}

DiskProgram::DiskProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: super(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_normal");
}

void DiskProgram::SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const
{
	super::SetUniforms(color);

	_center.Set(model_view.GetTranslation());
	_radius.Set(radius);
}

void DiskProgram::InitUniforms()
{
	super::InitUniforms();

	InitUniformLocation(_center, "center");
	InitUniformLocation(_radius, "radius");
}

////////////////////////////////////////////////////////////////////////////////
// TexturedProgram member definitions

TexturedProgram::TexturedProgram(TexturedProgram && rhs)
: LightProgram(std::move(rhs))
{
}

TexturedProgram::TexturedProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: LightProgram(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_tex_coord");
}

////////////////////////////////////////////////////////////////////////////////
// SpriteProgram member definitions

SpriteProgram::SpriteProgram(SpriteProgram && rhs)
: super(std::move(rhs))
, _position_scale(std::move(rhs._position_scale))
, _position_offset(std::move(rhs._position_offset))
{
}

SpriteProgram::SpriteProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: super(vert_sources, frag_sources)
{
	BindAttribLocation(1, "vertex_position");
	BindAttribLocation(2, "vertex_tex_coord");
}

void SpriteProgram::SetUniforms(geom::Vector2i const & resolution) const
{
	auto resolution_f = geom::Cast<float>(resolution);
	_position_scale.Set(Vector2(1.f / resolution_f.x, - 1.f / resolution_f.y));
	_position_offset.Set(Vector2(-.5f, .5f));
}

void SpriteProgram::InitUniforms()
{
	super::InitUniforms();

	InitUniformLocation(_position_scale, "position_scale");
	InitUniformLocation(_position_offset, "position_offset");
}
