//
//  RenderBuffer.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RenderBuffer.h"

#include "glHelpers.h"


using namespace gfx;


RenderBuffer::RenderBuffer()
: _id(0)
{
}

RenderBuffer::~RenderBuffer()
{
	ASSERT(! IsInitialized());
}

bool RenderBuffer::IsInitialized() const
{
	return _id != 0;
}

bool RenderBuffer::IsBound() const
{
	ASSERT(IsInitialized());
	return GetBinding<GL_RENDERBUFFER>() == _id;
}

void RenderBuffer::Init()
{
	GL_CALL(glGenRenderbuffers(1, & _id));
}

void RenderBuffer::Deinit()
{
	assert(_id != 0);
	GL_CALL(glDeleteRenderbuffers(1, & _id));
}

void RenderBuffer::Bind()
{
	GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, _id));
}

void RenderBuffer::ResizeForDepth(GLsizei width, GLsizei height)
{
	assert(IsBound());
	GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
}

//void AttachToFrameBuffer(RenderBuffer * render_buffer) 
//{
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer->id);
//}
