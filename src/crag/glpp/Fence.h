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

	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations

	class Fence;
	
	bool GenFence(Fence & f);
	void DeleteFence(Fence & f);
	void SetFence(Fence const & f);
	bool TestFence(Fence const & f);
	void FinishFence(Fence const & f);
	

	////////////////////////////////////////////////////////////////////////////////
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
		
		friend bool GenFence(Fence & f);
		friend void DeleteFence(Fence & f);
		
		friend void SetFence(Fence const & f);
		friend bool TestFence(Fence const & f);
		friend void FinishFence(Fence const & f);
		
	private:
		GLuint _id;
	};


	////////////////////////////////////////////////////////////////////////////////
	// fence class helper functions

#if defined(__APPLE__)
	inline bool GenFence(Fence & f)
	{
		assert(! f.IsInitialized());
		GLPP_CALL(glGenFencesAPPLE(1, & f._id));
		return true;
	}
	inline void DeleteFence(Fence & f)
	{
		assert(f.IsInitialized());
		GLPP_CALL(glDeleteFencesAPPLE(1, & f._id));
		f._id = 0;
	}
	
	inline void SetFence(Fence const & f)
	{
		assert(f.IsInitialized());
		GLPP_CALL(glSetFenceAPPLE(f._id));
	}
	inline bool TestFence(Fence const & f)
	{
		assert(f.IsInitialized());
		bool result = glTestFenceAPPLE(f._id) != GL_FALSE;
		GLPP_VERIFY;
		return result;
	}
	inline void FinishFence(Fence const & f)
	{
		assert(f.IsInitialized());
		GLPP_CALL(glFinishFenceAPPLE(f._id));
	}
#elif defined(WIN32)
	inline bool GenFence(Fence & f)
	{
		assert(! f.IsInitialized());

		if (! GLEW_NV_fence)
		{
			return false;
		}

		GLPP_CALL(glGenFencesNV(1, & f._id));
		return true;
	}
	inline void DeleteFence(Fence & f)
	{
		assert(f.IsInitialized());
		GLPP_CALL(glDeleteFencesNV(1, & f._id));
		f._id = 0;
	}
	
	inline void SetFence(Fence const & f)
	{
		assert(f.IsInitialized());
		GLPP_CALL(glSetFenceNV(f._id, GL_ALL_COMPLETED_NV));
	}
	inline bool TestFence(Fence const & f)
	{
		assert(f.IsInitialized());
		bool result = glTestFenceNV(f._id) != GL_FALSE;
		GLPP_VERIFY;
		return result;
	}
	inline void FinishFence(Fence const & f)
	{
		assert(f.IsInitialized());
		GLPP_CALL(glFinishFenceNV(f._id));
	}
#endif

}
