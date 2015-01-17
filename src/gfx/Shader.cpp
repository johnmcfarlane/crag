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

#if defined(CRAG_DEBUG)
#define DUMP_GLSL_ERRORS
//#define DUMP_GLSL_ERROR_FILES
#define DUMP_GLSL_ACCUMULATE_LINE_NUMBERS
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
	
#if ! defined(GRAG_GLES)
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
#endif

	std::vector<app::FileResource> ReadFileBuffers(std::initializer_list<char const *> filenames)
	{
		return core::make_transform(filenames, [](char const * filename)
		{
			if (!filename)
			{
				return app::FileResource();
			}

			auto source_buffer = app::LoadFile(filename);

			if (!source_buffer.empty())
			{
#if ! defined(GRAG_GLES)
				// earlier version of desktop GLSL fail to ignore these
				EraseQualifier(source_buffer.data(), "lowp");
				EraseQualifier(source_buffer.data(), "mediump");
				EraseQualifier(source_buffer.data(), "highp");
#endif
			}

			return source_buffer;
		});
	}
}

////////////////////////////////////////////////////////////////////////////////
// gfx::Shader member definitions

Shader::Shader() 
: _id(0)
{ 
	CRAG_VERIFY(* this);
}

Shader::Shader(Shader && rhs)
: _id(rhs._id)
{
	rhs._id = 0;
	CRAG_VERIFY(* this);
}
 
Shader::~Shader()
{
	CRAG_VERIFY(* this);
	ASSERT(! IsInitialized());
}

bool Shader::Init(std::initializer_list<char const *> filenames, GLenum shader_type)
{
	// Create the shader.
	GL_CALL(_id = glCreateShader(shader_type));
	if (_id == 0)
	{
#if defined(CRAG_DEBUG)
		for (auto filename : filenames)
		{
			DEBUG_MESSAGE("Failed to create shader that includes %s", filename);
		}
#endif

		return false;
	}

	// load individual shader files
	auto file_buffers = ReadFileBuffers(filenames);
	int num_strings = file_buffers.size();

	// generate array of pointers for GL
	auto string_array = core::make_transform(file_buffers, [](app::FileResource const & lhs)
	{
		return lhs.data();
	});

	// generate array of sizes for GL
	auto length_array = core::make_transform(file_buffers, [](app::FileResource const & lhs)
	{
		return GLint(lhs.size());
	});

	// pass to GL
	GL_CALL(glShaderSource(_id, num_strings, string_array.data(), length_array.data()));
	GL_CALL(glCompileShader(_id));
	
#if defined(DUMP_GLSL_ERRORS)
	std::vector<char> info_log = GetInfoLog();
	if (info_log.size() > 1)
	{
		auto line_start = 0;
		auto filename_iterator = std::begin(filenames);
		for (auto i = 0; i < num_strings; ++ filename_iterator, ++ i)
		{
#if defined(DUMP_GLSL_ERROR_FILES)
			auto const & buffer = string_array[i];
#if defined(DUMP_GLSL_ACCUMULATE_LINE_NUMBERS)
			line_start = 0;
#endif
			for (auto c = buffer; * c; )
			{
				auto line_end = std::strchr(c, '\n');
				auto line_length = line_end - c;
				PrintMessage(stderr, "%d:%3d %.*s", i, ++ line_start, line_length, c);
				c += line_length + (* line_end == '\n');
			}
#else
			auto line_end = int(line_start + GetNumLines(string_array[i]) - 1);
			PrintMessage(stderr, "%s [%d,%d]\n", * filename_iterator, line_start, line_end);
			line_start = line_end;
#endif
		}

		PrintMessage(stderr, "Shader info log:\n%s", info_log.data());
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
	CRAG_VERIFY(* this);
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
	
	if (real_size != size && real_size != size + 1)
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
	CRAG_VERIFY(* this);
	
	GL_CALL(glDeleteShader(_id));
	_id = 0;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Shader, self)
	if (self._id == 0)
	{
		return;
	}
	
	CRAG_VERIFY_TRUE(glIsShader(self._id));
CRAG_VERIFY_INVARIANTS_DEFINE_END
