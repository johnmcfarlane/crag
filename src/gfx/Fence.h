//
//  Fence.h
//  crag
//
//  Created by John McFarlane on 7/26/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once


namespace gfx
{
	////////////////////////////////////////////////////////////////////////////////
	// Fence class

	class Fence
	{
	public:
		Fence();
		~Fence();
		
		bool IsInitialized() const;
		
		bool Init();
		void Deinit();
		
		void Set();
		bool Test();
		void Finish();
		
	private:
		GLuint _id;
	};
}
