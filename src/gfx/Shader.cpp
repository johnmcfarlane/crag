//
//  Shader.cpp
//  crag
//
//  Created by John McFarlane on 1/20/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Shader.h"

#include "glHelpers.h"

#include "core/app.h"


gfx::Shader::Shader() 
: _id(0)
{ 
	VerifyObject(* this);
}

gfx::Shader::~Shader()
{
	VerifyObject(* this);
	assert(! IsInitialized());
}

bool gfx::Shader::Init(char const * filename, GLenum shader_type)
{
	std::vector<char> sphere_shader_source;
	
	if (! app::LoadFile(filename, sphere_shader_source))
	{
		AssertErrno();
		return false;
	}
	
	// Create the shader.
	_id = glCreateShader(shader_type);

	char const * sphere_shader_source_string[1] = { & sphere_shader_source[0] };
	int sphere_shader_source_size[1] = { int(sphere_shader_source.size()) };
	GL_CALL(glShaderSource(_id, 1, sphere_shader_source_string, sphere_shader_source_size));

	GL_CALL(glCompileShader(_id));
	
	// Check for errors in the source code.
	if (! IsCompiled())
	{
		ERROR_MESSAGE("Failed to compile shader, '%s'.", filename);

#if ! defined(NDEBUG)
		std::string info_log;
		GetInfoLog(info_log);
		DEBUG_BREAK("Shader info log: \n%s", info_log.c_str());
#endif
		
		Deinit();
		return false;
	}
	
	return true;
}

bool gfx::Shader::IsInitialized() const
{
	VerifyObject(* this);
	return _id != 0;
}

bool gfx::Shader::IsCompiled() const
{
	assert(IsInitialized());
	
	GLint params;
	GL_CALL(glGetShaderiv(_id, GL_COMPILE_STATUS, & params));
	
	return params != GL_FALSE;
}

void gfx::Shader::GetInfoLog(std::string & info_log) const
{
	GLint length;
	GL_CALL(glGetShaderiv(_id, GL_INFO_LOG_LENGTH, & length));
	
	if (length == 0)
	{
		info_log.clear();
	}
	else
	{
		info_log.resize(length);
		glGetShaderInfoLog(_id, length, nullptr, & info_log[0]);
	}
}

void gfx::Shader::Deinit()
{
	VerifyObject(* this);
	
	GL_CALL(glDeleteShader(_id));
	_id = 0;
}

#if defined(VERIFY)
void gfx::Shader::Verify() const
{
	if (_id == 0)
	{
		return;
	}
	
	assert(glIsShader(_id));
}
#endif
