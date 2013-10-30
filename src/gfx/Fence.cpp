//
//  Fence.cpp
//  crag
//
//  Created by John McFarlane on 1/19/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fence.h"

#include "glHelpers.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// fence class helper functions

Fence::Fence() 
: _id(0) 
{ 
}

Fence::~Fence() 
{
	assert(! IsInitialized());
}

bool Fence::IsInitialized() const
{
	return _id != 0;
}

bool Fence::Init()
{
	assert(! IsInitialized());

#if defined(__APPLE__)
	glGenFencesAPPLE(1, & _id);
#elif defined(CRAG_USE_GLES)
	DEBUG_BREAK("Not supported");
#else
	glGenFencesNV(1, & _id);
#endif

	return true;
}

void Fence::Deinit()
{
	assert(IsInitialized());

#if defined(__APPLE__)
	glDeleteFencesAPPLE(1, & _id);
#elif defined(CRAG_USE_GLES)
	DEBUG_BREAK("Not supported");
#else
	glDeleteFencesNV(1, & _id);
#endif
	
	_id = 0;
}

void Fence::Set()
{
	assert(IsInitialized());

#if defined(__APPLE__)
	glSetFenceAPPLE(_id);
#elif defined(CRAG_USE_GLES)
	DEBUG_BREAK("Not supported");
#else
	glSetFenceNV(_id, GL_ALL_COMPLETED_NV);
#endif
}

bool Fence::Test()
{
	assert(IsInitialized());

#if defined(__APPLE__)
	bool result = glTestFenceAPPLE(_id) != GL_FALSE;
#elif defined(CRAG_USE_GLES)
	DEBUG_BREAK("Not supported");
	bool result = true;
#else
	bool result = glTestFenceNV(_id) != GL_FALSE;
#endif

	GL_VERIFY;
	return result;
}

void Fence::Finish()
{
	assert(IsInitialized());

#if defined(__APPLE__)
	glFinishFenceAPPLE(_id);
#elif defined(CRAG_USE_GLES)
	DEBUG_BREAK("Not supported");
#else
	glFinishFenceNV(_id);
#endif
}
