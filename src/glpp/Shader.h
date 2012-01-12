//
//  Shader.h
//  crag
//
//  Created by John McFarlane on 2011/12/28.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "glpp.h"


namespace gl
{
	////////////////////////////////////////////////////////////////////////////////
	// class declarations
	
	class Program;
	class Shader;
	
	////////////////////////////////////////////////////////////////////////////////
	// helper function declarations
	
	void Create(Shader & shader, GLenum shader_type);
	void Delete(Shader & shader);
	void Source(Shader & shader, char const * string, GLint * length = nullptr);
	void Compile(Shader & shader);
	
	void Create(Program & program);
	void Delete(Program & program);
	void BindAttribLocation(Program & program, GLuint index, GLchar const * name);
	void Use(Program const & program);
	void Disuse(Program const & program);
	void Link(Program & program);
	GLint GetUniformLocation(Program & program, char const * name);

	void Attach(Program & program, Shader & shader);
	void Detach(Program & program, Shader & shader);
	
	////////////////////////////////////////////////////////////////////////////////
	// class definitions
	
	// Shader class
	class Shader
	{
		OBJECT_NO_COPY(Shader);
		
		friend void Create(Shader & shader, GLenum shader_type);
		friend void Delete(Shader & shader);
		friend void Source(Shader & shader, char const * string, GLint * length);
		friend void Compile(Shader & shader);
		friend void Attach(Program & program, Shader & shader);
		friend void Detach(Program & program, Shader & shader);
		
	public:
		Shader() 
		: _id(0)
		{ 
			Verify();
		}
		
		~Shader()
		{
			Verify();
			assert(! IsInitialized());
		}
		
		bool IsInitialized() const
		{
			Verify();
			return _id != 0;
		}
		
		bool IsCompiled() const
		{
			assert(IsInitialized());
			
			GLint params;
			GLPP_CALL(glGetShaderiv(_id, GL_COMPILE_STATUS, & params));
			
			return params != GL_FALSE;
		}
		
		void GetInfoLog(std::string & info_log) const
		{
			GLint length;
			GLPP_CALL(glGetShaderiv(_id, GL_INFO_LOG_LENGTH, & length));
			
			if (length == 0)
			{
				info_log.clear();
			}
			else
			{
				info_log.resize(length);
				glGetShaderInfoLog(_id, length, NULL, & info_log[0]);
			}
		}
		
	private:
		void Verify() const
		{
			if (_id == 0)
			{
				return;
			}
			
			assert(glIsShader(_id));
		}
		
		GLuint _id;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// Program
	
	class Program
	{
		friend void Create(Program & program);
		friend void Delete(Program & program);
		friend void BindAttribLocation(Program & program, GLuint index, GLchar const * name);
		friend void Use(Program const & program);
		friend void Disuse(Program const & program);
		friend void Link(Program & program);
		friend GLint GetUniformLocation(Program & program, char const * name);
		friend void Attach(Program & program, Shader & shader);
		friend void Detach(Program & program, Shader & shader);
		
	public:
		Program()
		: _id(0)
		{
		}
		
		~Program()
		{
			Verify();
			assert(! IsInitialized());
		}

		bool IsInitialized() const
		{
			Verify();
			return _id != 0;
		}

		bool IsLinked() const
		{
			assert(IsInitialized());
			
			GLint params;
			GLPP_CALL(glGetProgramiv(_id, GL_LINK_STATUS, & params));
			
			return params != GL_FALSE;
		}
		
		bool IsInUse() const
		{
			assert(IsInitialized());
			
			GLuint used_id = GetInt<GL_CURRENT_PROGRAM>();
			
			return used_id == _id;
		}
		
		void GetInfoLog(std::string & info_log) const
		{
			GLint length;
			GLPP_CALL(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, & length));
			
			if (length == 0)
			{
				info_log.clear();
			}
			else
			{
				info_log.resize(length);
				glGetProgramInfoLog(_id, length, NULL, & info_log[0]);
			}
		}

	private:
		void Verify() const
		{
			if (_id == 0)
			{
				return;
			}
			
			assert(glIsProgram(_id));
		}

		// variables
		GLuint _id;
	};
	
	////////////////////////////////////////////////////////////////////////////////
	// helper function definitions
	
	inline void Create(Shader & shader, GLenum shader_type)
	{
		assert(! shader.IsInitialized());
		
		switch (shader_type)
		{
			default:
				assert(false);
			case GL_FRAGMENT_SHADER:
			case GL_VERTEX_SHADER:
				break;
		}
		
		shader._id = glCreateShader(shader_type);
		
		GLPP_VERIFY;
	}
	
	inline void Delete(Shader & shader)
	{
		shader.Verify();
		
		GLPP_CALL(glDeleteShader(shader._id));
		shader._id = 0;
	}
	
	inline void Source(Shader & shader, char const * string, GLint * length)
	{
		assert(shader.IsInitialized());
		
		GLPP_CALL(glShaderSource(shader._id, 1, & string, length));
	}
	
	inline void Compile(Shader & shader)
	{
		assert(shader.IsInitialized());
		
		GLPP_CALL(glCompileShader(shader._id));
	}
	
	inline void Create(Program & program)
	{
		assert(! program.IsInitialized());
		
		GLPP_CALL(program._id = glCreateProgram());
	}
	
	inline void Delete(Program & program)
	{
		assert(program.IsInitialized());
		
		GLPP_CALL(glDeleteProgram(program._id));
		program._id = 0;
	}
	
	inline void BindAttribLocation(Program & program, GLuint index, GLchar const * name)
	{
		GLPP_CALL(glBindAttribLocation(program._id, index, name));
	}
	
	inline void Use(Program const & program)
	{
		Assert(program.IsInitialized());
		GLPP_CALL(glUseProgram(program._id));
	}
	
	inline void Disuse(Program const & program)
	{
		Assert(program.IsInUse());
		GLPP_CALL(glUseProgram(0));
	}
	
	inline void Link(Program & program)
	{
		Assert(program.IsInitialized());
		GLPP_CALL(glLinkProgram(program._id));
	}
	
	inline GLint GetUniformLocation(Program & program, char const * name)
	{
		Assert(program.IsInitialized());
		GLint location = glGetUniformLocation(program._id, name);
		GLPP_VERIFY;
		return location;
	}
	
	inline void Attach(Program & program, Shader & shader)
	{
		GLPP_CALL(glAttachShader(program._id, shader._id));
	}
	
	inline void Detach(Program & program, Shader & shader)
	{
		GLPP_CALL(glDetachShader(program._id, shader._id));
	}
	
	inline void Uniform(GLint location, float v0)
	{
		GLPP_CALL(glUniform1f(location, v0));
	}
	inline void Uniform(GLint location, float v0, float v1)
	{
		GLPP_CALL(glUniform2f(location, v0, v1));
	}
	inline void Uniform(GLint location, float v0, float v1, float v2)
	{
		GLPP_CALL(glUniform3f(location, v0, v1, v2));
	}
	inline void Uniform(GLint location, float v0, float v1, float v2, float v3)
	{
		GLPP_CALL(glUniform4f(location, v0, v1, v2, v3));
	}

	template <typename S, GLint N>
	void inline Uniform(GLint location, S const * value, GLint count = 1);
	
	template <> 
	inline void Uniform<float, 3>(GLint location, float const * value, GLint count)
	{
		GLPP_CALL(glUniform3fv(location, count, value));
	}
	
	template <> 
	inline void Uniform<float, 4>(GLint location, float const * value, GLint count)
	{
		GLPP_CALL(glUniform4fv(location, count, value));
	}
}
