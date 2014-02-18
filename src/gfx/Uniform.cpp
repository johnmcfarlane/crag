//
//  gfx/Uniform.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-20.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "axes.h"
#include "Color.h"
#include "glHelpers.h"
#include "Uniform.h"

////////////////////////////////////////////////////////////////////////////////
// gfx::Uniform::Set specializations

namespace gfx
{
	template <>
	void gfx::Uniform<bool>::Set(bool const & value) const
	{
		ASSERT(IsInitialized());
		GL_CALL(glUniform1i(_location, value));
	}

	template <>
	void Uniform<int>::Set(int const & value) const
	{
		ASSERT(IsInitialized());
		GL_CALL(glUniform1i(_location, value));
	}

	template <>
	void Uniform<float>::Set(float const & value) const
	{
		ASSERT(IsInitialized());
		GL_CALL(glUniform1f(_location, value));
	}

	template <>
	void Uniform<geom::Vector2f>::Set(geom::Vector2f const & value) const
	{
		ASSERT(IsInitialized());
		GL_CALL(glUniform2f(_location, value.x, value.y));
	}

	template <>
	void Uniform<geom::Vector3f>::Set(geom::Vector3f const & value) const
	{
		ASSERT(IsInitialized());
		auto flipped = ToOpenGl(value);
		GL_CALL(glUniform3f(_location, flipped.x, flipped.y, flipped.z));
	}

	template <>
	void Uniform<Color4f>::Set(Color4f const & value) const
	{
		ASSERT(IsInitialized());
		GL_CALL(glUniform4f(_location, value.r, value.g, value.b, value.a));
	}

	template <>
	void Uniform<geom::Matrix44f>::Set(geom::Matrix44f const & value) const
	{
		ASSERT(IsInitialized());
	#if defined(CRAG_USE_GLES)
		GL_CALL(glUniformMatrix4fv(_location, 1, GL_FALSE, geom::Transposition(value).GetArray()));
	#elif defined(CRAG_USE_GL)
		GL_CALL(glUniformMatrix4fv(_location, 1, GL_TRUE, value.GetArray()));
	#endif
	}
}
