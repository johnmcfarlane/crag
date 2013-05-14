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

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Shader member definitions

Shader::Shader() 
: _id(0)
{ 
	VerifyObject(* this);
}

Shader::~Shader()
{
	VerifyObject(* this);
	assert(! IsInitialized());
}

bool Shader::Init(char const * const * filenames, GLenum shader_type)
{
	typedef app::FileResource::element_type Buffer;
	typedef std::vector<app::FileResource> BufferArray;
	
	auto filenames_end = filenames;
	while (* filenames_end)
	{
		++ filenames_end;
	}
	
	auto num_strings = filenames_end - filenames;
	
	// load individual shader files
	char const * string_array [num_strings];
	BufferArray source_buffers(num_strings);
	for (auto index = 0; index != num_strings; ++ index)
	{
		char const * filename = filenames[index];
		
		auto source_buffer = app::LoadFile(filename, true);
		if (! source_buffer)
		{
			return false;
		}
		
		string_array[index] = source_buffer->data();
		
		source_buffers[index] = std::move(source_buffer);
	}
	
	// Create the shader.
	GL_CALL(_id = glCreateShader(shader_type));
	if (_id == 0)
	{
		return false;
	}
	
	GL_CALL(glShaderSource(_id, num_strings, string_array, nullptr));
	GL_CALL(glCompileShader(_id));
	
	// Check for errors in the source code.
	if (! IsCompiled())
	{
		ERROR_MESSAGE("Failed to compile shaders, '%s'.", filenames[0]);

#if ! defined(NDEBUG)
		std::vector<char> info_log = GetInfoLog();
		DEBUG_BREAK("Shader info log: %s", info_log.data());
#endif
		
		Deinit();
		return false;
	}
	
	return true;
}

bool Shader::IsInitialized() const
{
	VerifyObject(* this);
	return _id != 0;
}

bool Shader::IsCompiled() const
{
	assert(IsInitialized());
	
	GLint params;
	GL_CALL(glGetShaderiv(_id, GL_COMPILE_STATUS, & params));
	
	return params != GL_FALSE;
}

std::vector<char> Shader::GetInfoLog() const
{
	std::vector<char> log_buffer;
	
	GLint size;
	GL_CALL(glGetShaderiv(_id, GL_INFO_LOG_LENGTH, & size));

	log_buffer.resize(size);
	GLint length;
	glGetShaderInfoLog(_id, size, & length, & log_buffer.front());
	GLint real_size = length + 1;
	
	if (real_size != size)
	{
		DEBUG_MESSAGE("glGetShaderInfoLog buffer is %d bytes - not %d bytes", real_size, size);
		size = real_size;
		log_buffer.resize(size);
		glGetShaderInfoLog(_id, size, & length, & log_buffer.front());
		DEBUG_MESSAGE("second attempt: %d bytes", length + 1);
	}
	
	ASSERT(log_buffer.empty() || log_buffer.back() == '\0');
	
	return log_buffer;
}

void Shader::Deinit()
{
	VerifyObject(* this);
	
	GL_CALL(glDeleteShader(_id));
	_id = 0;
}

#if defined(VERIFY)
void Shader::Verify() const
{
	if (_id == 0)
	{
		return;
	}
	
	assert(glIsShader(_id));
}
#endif
