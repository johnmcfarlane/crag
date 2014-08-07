//
//  Shader.h
//  crag
//
//  Created by John McFarlane on 2011/12/28.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/counted_object.h"

namespace gfx
{
	// Wrapper for an OpenGL shader object.
	class Shader : private crag::counted_object<Shader>
	{
		OBJECT_NO_COPY(Shader);
		
		friend class Program;
		
		// functions
	public:
		Shader();
		Shader(Shader && rhs);
		~Shader();
		
		bool Init(std::initializer_list<char const *> filenames, GLenum shader_type);
		void Deinit();
		
		bool IsInitialized() const;
		bool IsCompiled() const;
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Shader);
		
	private:
		std::vector<char> GetInfoLog() const;		
		
		// variables
		GLuint _id;
	};
}
