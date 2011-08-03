/*
 *  Name.h
 *  Crag
 *
 *  Created by John on 12/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/debug.h"

#include "glpp.h"

// TODO: In all the places where textures, buffer objects etc. are
// tested for not-being-bound immediately prior to binding, should
// maybe test for nothing-at-all-bounded instead?


namespace gl
{
	
	// Specializations
	template<GLenum TARGET> GLuint Init();
	template<GLenum TARGET> void Deinit(GLuint id);
	template<GLenum TARGET> void Bind(GLuint id);
	template<GLenum TARGET> GLuint GetBindingEnum();


	template<GLenum TARGET> class Name
	{
		// Class is non-copyable.
		Name(Name const & rhs);
		void operator = (Name const & rhs);
		
	public:
		Name()
		: id(0)
		{
		}
		
		~Name()
		{
			assert(! IsInitialized());
		}
		
		bool IsInitialized() const
		{
			return id != 0;
		}

	protected:
		GLuint id;
	};
	
}
