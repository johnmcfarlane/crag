//
//  Shader.h
//  crag
//
//  Created by John McFarlane on 2011/12/28.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace gfx
{
	// Wrapper for an OpenGL shader object.
	class Shader
	{
		OBJECT_NO_COPY(Shader);
		
		friend class Program;
		
		// functions
	public:
		Shader();
		~Shader();
		
		bool Init(char const * filename, GLenum shader_type);
		void Deinit();
		
		bool IsInitialized() const;
		bool IsCompiled() const;
		
	private:
		void GetInfoLog(std::string & info_log) const;		
		
#if defined(VERIFY)
		void Verify() const;
#endif
	
		// variables
		GLuint _id;
	};
}
