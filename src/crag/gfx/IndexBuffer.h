/*
` *  IndexBuffer.h
 *  Crag
 *
 *  Created by john on 5/3/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */
 
#pragma once

#include "glpp/glpp.h"

#include <vector>


class IndexBuffer : private std::vector<GLuint>
{
public:
	IndexBuffer(int capacity);

	// number of indices
	int GetSize() const 
	{ 
		return size(); 
	}
	
	int GetCapacity() const
	{
		return capacity();
	}

	// number of spare faces
	int GetSlack() const 
	{ 
		return capacity() - size(); 
	}
	
	void Clear();
	
	void PushBack(value_type index)
	{
		push_back(index);
	}
	
	value_type const * GetArray() const;
};

