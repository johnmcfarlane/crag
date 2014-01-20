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
		}
		
		bool IsInitialized() const
		{
			return _location != _invalid_location;
		}
		
		void Set(Type const & value) const;
		
	protected:
		// variables
		GLint _location = _invalid_location;
		
		// constants
		static constexpr GLint _invalid_location = -1;
	};
}
