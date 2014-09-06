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
//#define CRAG_GFX_UNIFORMS_DEBUG
#endif

// fewer redundant GL calls; more CPU load / memory
//#define CRAG_GFX_UNIFORMS_LAZY

namespace gfx
{
	template <typename Type>
	class Uniform
	{
		OBJECT_NO_COPY(Uniform);

	public:
		Uniform() = default;
		
		Uniform(Uniform && rhs)
			: _location(rhs._location)
		{
			rhs._location = _invalid_location;
#if defined(CRAG_GFX_UNIFORMS_LAZY)
			std::swap(_written, rhs._written);
#endif
		}
		
		Uniform(GLuint program, char const * name)
		: _location(glGetUniformLocation(program, name))
		{
			GL_VERIFY;
			
			if (! IsInitialized())
			{
#if defined(CRAG_GFX_UNIFORMS_DEBUG)
				// can be caused by shader compiler identifying 
				// that a variable is unused and optimizing it away
				DEBUG_BREAK("failed to get location of uniform, \"%s\"", name);
#endif

				return;
			}

#if defined(CRAG_GFX_UNIFORMS_LAZY)
			SetUnchecked(_written);
#endif
		}

		Uniform & operator=(Uniform && rhs)
		{
			std::swap(_location, rhs._location);
#if defined(CRAG_GFX_UNIFORMS_LAZY)
			std::swap(_written, rhs._written);
#endif
			return * this;
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
			
#if defined(CRAG_GFX_UNIFORMS_LAZY)
			if (value == _written)
			{
				return;
			}
#endif

			SetUnchecked(value);

#if defined(CRAG_GFX_UNIFORMS_LAZY)
			_written = value;
#endif
		}
		
	protected:
		void SetUnchecked(Type const & value) const;

		// variables
		GLint _location = _invalid_location;
#if defined(CRAG_GFX_UNIFORMS_LAZY)
		mutable Type _written = Type();
#endif
		
		// constants
		static constexpr GLint _invalid_location = -1;
	};
}
