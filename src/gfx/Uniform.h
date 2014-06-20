//
//  gfx/Uniform.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-20.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "glHelpers.h"

#if ! defined(NDEBUG)
#define CRAG_GFX_UNIFORMS_DEBUG
#endif

namespace gfx
{
	template <typename Type>
	class Uniform
	{
	public:
		Uniform() = default;
		
		Uniform(Uniform const & rhs) = default;
		
		Uniform(GLuint program, char const * name)
		: _location(glGetUniformLocation(program, name))
		{
			GL_VERIFY;
			
#if defined(CRAG_GFX_UNIFORMS_DEBUG)
			if (! IsInitialized())
			{
				// can be caused by shader compiler identifying 
				// that a variable is unused and optimizing it away
				DEBUG_BREAK("failed to get location of uniform, \"%s\"", name);
			}
#endif
		}
		
		bool IsInitialized() const
		{
			return _location != _invalid_location;
		}
		
		void Set(Type const & value) const
		{
			if (! IsInitialized())
			{
				return;
			}
			
			SetUnchecked(value);
		}
		
	protected:
		void SetUnchecked(Type const & value) const;

		// variables
		GLint _location = _invalid_location;
		
		// constants
		static constexpr GLint _invalid_location = -1;
	};
}
