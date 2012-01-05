//
//  Name.h
//  crag
//
//  Created by John on 12/13/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "glpp.h"


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
