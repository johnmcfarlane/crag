//
//  FrameBuffer.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "FrameBuffer.h"
#include "glHelpers.h"


using namespace gfx;


FrameBuffer::FrameBuffer()
: _id(0)
{
}

FrameBuffer::~FrameBuffer()
{
	Assert(! IsInitialized());
}

bool FrameBuffer::IsInitialized() const
{
	return _id != 0;
}

bool FrameBuffer::IsBound() const
{
	return GetBinding<GL_FRAMEBUFFER>() == _id;
}

void FrameBuffer::Init()
{
	GL_CALL(glGenFramebuffers(1, & _id));
}

void FrameBuffer::Deinit()
{
	assert(IsInitialized());
	GL_CALL(glDeleteFramebuffers(1, & _id));
	_id = 0;
}

void FrameBuffer::Bind(GLuint id)
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, id));
}
