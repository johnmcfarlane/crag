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

int Program::SetLights(Color4f const &, Light::List const &, LightFilter const &) const
{
	return 0;
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
, used(std::move(rhs.used))
, fragment(std::move(rhs.fragment))
, search(std::move(rhs.search))
{
}

LightProgram::LightUniforms & LightProgram::LightUniforms::operator = (LightUniforms && rhs)
{
	std::swap(position, rhs.position);
	std::swap(direction, rhs.direction);
	std::swap(color, rhs.color);
	std::swap(angle, rhs.angle);
	std::swap(used, rhs.used);
	std::swap(fragment, rhs.fragment);
	std::swap(search, rhs.search);

	return * this;
}

////////////////////////////////////////////////////////////////////////////////
// LightProgram member definitions

LightProgram::LightProgram(LightProgram && rhs)
: Program3d(std::move(rhs))
{
	for (auto i = 0; i != max_lights; ++ i)
	{
		std::swap(_lights[i], rhs._lights[i]);
	}
}

LightProgram::LightProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
: Program3d(vert_sources, frag_sources)
{
}

void LightProgram::InitUniforms()
{
	super::InitUniforms();	

	for (auto index = 0u; index != _lights.size(); ++ index)
	{
		auto & light = _lights[index];

		int constexpr name_size = 64;
		char name[name_size];

		snprintf(name, name_size, "lights[%d].position", index);
		InitUniformLocation(light.position, name);

		snprintf(name, name_size, "lights[%d].direction", index);
		InitUniformLocation(light.direction, name);

		snprintf(name, name_size, "lights[%d].color", index);
		InitUniformLocation(light.color, name);

		snprintf(name, name_size, "lights[%d].angle", index);
		InitUniformLocation(light.angle, name);

		snprintf(name, name_size, "lights[%d].used", index);
		InitUniformLocation(light.used, name);

		snprintf(name, name_size, "lights[%d].fragment", index);
		InitUniformLocation(light.fragment, name);

		snprintf(name, name_size, "lights[%d].search", index);
		InitUniformLocation(light.search, name);
	}
}

int LightProgram::SetLights(Color4f const &, Light::List const & lights, LightFilter const & filter) const
{
	ASSERT(_lights.size() == max_lights);
	ASSERT(IsBound());

	auto total_lights = 0;

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

		if (unsigned(total_lights) == _lights.size())
		{
			if (CRAG_DEBUG_ONCE)
			{
				DEBUG_MESSAGE(
					"too many lights [%u>%u]", unsigned(total_lights), _lights.size());
			}

			break;
		}

		auto const & light_uniforms = _lights[total_lights];

		auto const & transformation = light.GetModelViewTransformation();
		auto position = transformation.GetTranslation();
		light_uniforms.position.Set(position);

		auto const & direction = GetAxis(transformation.GetRotation(), Direction::forward);
		light_uniforms.direction.Set(direction);

		auto const & color = light.GetColor();
		light_uniforms.color.Set(color);

		auto const & angle = light.GetAngle();
		light_uniforms.angle.Set(angle);
		
		auto attributes = light.GetAttributes();
		light_uniforms.used.Set(true);
		light_uniforms.fragment.Set(attributes.resolution == LightResolution::fragment);
		light_uniforms.search.Set(attributes.type == LightType::search);

		++ total_lights;
	}
	
	for (auto unused = unsigned(total_lights); unused != _lights.size(); ++ unused)
	{
		auto const & light_uniforms = _lights[unused];
		light_uniforms.used.Set(false);
	}

	// base class override is a stub
	ASSERT(super::SetLights(Color4f(), lights, filter) == 0);
	
	return total_lights;
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

int ForegroundProgram::SetLights(Color4f const & ambient, Light::List const & lights, LightFilter const & filter) const
{
	CRAG_VERIFY_EQUAL(ambient.a, 1);
	
	_ambient.Set(ambient);

	return LightProgram::SetLights(ambient, lights, filter);
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
