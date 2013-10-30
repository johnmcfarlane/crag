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

#if ! defined(NDEBUG)
#define DUMP_GLSL_ERRORS
#endif

using namespace gfx;

namespace
{
#if defined(DUMP_GLSL_ERRORS)
	std::size_t GetNumLines(char const * source)
	{
		std::size_t num_lines = 1;
		while (true)
		{
			switch (* source)
			{
				case '\0':
					return num_lines;
					
				case '\n':
					++ num_lines;
			}
			
			++ source;
		}
	}
#endif
	
	void EraseQualifier(char * source, char const * pattern)
	{
		ASSERT(source != nullptr);
		ASSERT(pattern != nullptr);
		
		auto pattern_length = strlen(pattern);
		if (pattern_length == 0)
		{
			DEBUG_BREAK("empty pattern string");
			return;
		}
		
		while (*source != '\0')
		{
			if (strncmp(source, pattern, pattern_length) == 0)
			{
				std::fill(source, source + pattern_length, ' ');
				source += pattern_length;
			}
			else
			{
				++ source;
			}
		}
	}
}

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
	typedef std::vector<app::FileResource> BufferArray;
	
	auto filenames_end = filenames;
	while (* filenames_end)
	{
		++ filenames_end;
	}
	
	auto num_strings = filenames_end - filenames;
	
	// load individual shader files
#if defined(WIN32)
	auto string_array = static_cast<char const * *>(_malloca(num_strings * sizeof(char const *)));
#else
	char const * string_array [num_strings];
#endif
	BufferArray source_buffers(num_strings);
	for (auto index = 0; index != num_strings; ++ index)
	{
		char const * filename = filenames[index];
		
		auto source_buffer = app::LoadFile(filename, true);
		if (! source_buffer)
		{
			return false;
		}
		
#if ! defined(CRAG_USE_GLES)
		// earlier version of desktop GLSL fail to ignore these
		EraseQualifier(source_buffer->data(), "lowp");
		EraseQualifier(source_buffer->data(), "mediump");
		EraseQualifier(source_buffer->data(), "highp");
#endif
		
		string_array[index] = source_buffer->data();
		
		source_buffers[index] = std::move(source_buffer);
	}
	
	// Create the shader.
	GL_CALL(_id = glCreateShader(shader_type));
	if (_id == 0)
	{
		return false;
	}
	
	GL_CALL(glShaderSource(_id, int(num_strings), string_array, nullptr));

#if defined(WIN32)
	_freea(string_array);
#endif

	GL_CALL(glCompileShader(_id));
	
#if defined(DUMP_GLSL_ERRORS)
	std::vector<char> info_log = GetInfoLog();
	if (! info_log.empty())
	{
		auto line_start = 0;
		for (auto i = 0; i < num_strings; ++ i)
		{
			auto line_end = int(line_start + GetNumLines(string_array[i]) - 1);
			PrintMessage(stderr, "%s [%d,%d]\n", filenames[i], line_start, line_end);
			line_start = line_end;
		}

		PrintMessage(stderr, "Shader info log: %s", info_log.data());
	}
#endif
	
	// Check for errors in the source code.
	if (! IsCompiled())
	{
		DEBUG_BREAK("Failed to compile shader.");
		
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
	
	if (real_size != size + 1)
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
