//
//  Fence.h
//  crag
//
//  Created by John McFarlane on 7/26/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "glpp.h"


namespace gl
{
	// forward-declarations
	class Fence;
	
	void GenFence(Fence & f);
	void DeleteFence(Fence & f);
	void SetFence(Fence const & f);
	bool TestFence(Fence const & f);
	void FinishFence(Fence const & f);
	
	// fence class
	class Fence
	{
	public:
		Fence() 
		: _id(0) 
		{ 
		}
		
		~Fence() 
		{
			assert(! IsInitialized());
		}
		
		bool IsInitialized() const
		{
			assert(_id == 0 || glIsFenceAPPLE(_id));
			return _id != 0;
		}
		
		friend void GenFence(Fence & f)
		{
			assert(! f.IsInitialized());
			GLPP_CALL(glGenFencesAPPLE(1, & f._id));
		}
		friend void DeleteFence(Fence & f)
		{
			assert(f.IsInitialized());
			GLPP_CALL(glDeleteFencesAPPLE(1, & f._id));
			f._id = 0;
		}
		
		friend void SetFence(Fence const & f)
		{
			assert(f.IsInitialized());
			GLPP_CALL(glSetFenceAPPLE(f._id));
		}
		friend bool TestFence(Fence const & f)
		{
			assert(f.IsInitialized());
			bool result = glTestFenceAPPLE(f._id) != GL_FALSE;
			GLPP_VERIFY;
			return result;
		}
		friend void FinishFence(Fence const & f)
		{
			assert(f.IsInitialized());
			GLPP_CALL(glFinishFenceAPPLE(f._id));
		}
		
	private:
		GLuint _id;
	};
}
