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

	// load individual shader files and count total number of bytes
	BufferArray source_buffers;
	auto total_bytes = 0;
	for (auto filename_iterator = filenames; * filename_iterator != nullptr; ++ filename_iterator)
	{
		auto source_buffer = app::LoadFile(* filename_iterator);
		source_buffers.emplace_back(source_buffer);
		total_bytes += source_buffers.back()->size();
	}
	
	// combine buffers into one buffer of source code
	Buffer source_buffer(total_bytes + 1);
	{
		auto destination_iterator = source_buffer.begin();
		for (auto source : source_buffers)
		{
			destination_iterator = std::copy(source->begin(), source->end(), destination_iterator);
		}
		ASSERT(& (* destination_iterator) == & source_buffer.back());

		(* destination_iterator) = '\0';
		ASSERT(source_buffer.size() == strlen(& source_buffer.front()) + 1);
	}
	
	// Create the shader.
	_id = glCreateShader(shader_type);

	char const * source_buffer_string[1] = { source_buffer.data() };
	int source_buffer_size[1] = { int(total_bytes) };
	GL_CALL(glShaderSource(_id, 1, source_buffer_string, source_buffer_size));

	GL_CALL(glCompileShader(_id));
	
	// Check for errors in the source code.
	if (! IsCompiled())
	{
		ERROR_MESSAGE("Failed to compile shaders, '%s'.", filenames[0]);

#if ! defined(NDEBUG)
		std::vector<char> info_log = GetInfoLog();
		DEBUG_BREAK("Shader info log: %s", & info_log.front());
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
