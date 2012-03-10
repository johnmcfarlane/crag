//
//  StackFrameBuffer.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "StackFrameBuffer.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::StackFrameBuffer member definitions

StackFrameBuffer::StackFrameBuffer()
: _range(null, null)
{
}

StackFrameBuffer::~StackFrameBuffer()
{
	VerifyObject(* this);
	
#if ! defined(NDEBUG) && 0
	std::cout << "~StackFrameBuffer: " 
		<< (_range[1] - _range[0]) << '/' 
		<< _buffer.size() << '/' 
		<< _buffer.capacity() << std::endl;
#endif
}

void StackFrameBuffer::SetBottom(char * bottom)
{
	VerifyObject(* this);
	ASSERT(_range[top_index] == null);
	
	_range[bottom_index] = bottom;
	VerifyObject(* this);
}

void StackFrameBuffer::SetTop(char * top)
{
	VerifyObject(* this);
	ASSERT(_range[bottom_index] != null);
	
	_range[top_index] = top;
	VerifyObject(* this);
}

// The master class in harmful programming techniques 
// reaches it's logical conclusion inside this function.
char * StackFrameBuffer::CalculateTop()
{
	char marker;
	char * top = & marker;
	return top;
}

// Take a snapshot of the section of the stack given by the range.
void StackFrameBuffer::Copy()
{
	VerifyObject(* this);
	
	std::size_t buffer_size = GetRangeBytes();
	_buffer.resize(buffer_size);
	ASSERT(_buffer.size() == buffer_size);
	
	std::memcpy(& _buffer.front(), _range[0], buffer_size);
}

// Copy the stored snapshot back to its source.
void StackFrameBuffer::Restore() const
{
	VerifyObject(* this);
	
	std::size_t buffer_size = _buffer.size();
	ASSERT(buffer_size == GetRangeBytes());
	
	std::memcpy(_range[0], & _buffer.front(), buffer_size);
}

bool StackFrameBuffer::IsPersistent(ConstPointerRange const & data) const
{
	ASSERT(data[0] < data[1]);
	
	VerifyObject(* this);
	ASSERT(_range[0] != nullptr);
	ASSERT(_range[1] != nullptr);

	if (_range[0] >= data[1])
	{
		// data is fully before the section of stack used by the fiber
		return true;
	}
	
	if (_range[1] <= data[0])
	{
		// data is fully after the section of stack used by the fiber
		return true;
	}
	
	return false;
}

#if defined(VERIFY)
void StackFrameBuffer::Verify() const
{
	if (_range[0] != nullptr && _range[1] != nullptr)
	{
		VerifyTrue(GetRangeBytes() > 0);
	}
}
#endif

std::size_t StackFrameBuffer::GetRangeBytes() const
{
	ASSERT(_range[0] != nullptr);
	char const * minimum = static_cast<char const *>(_range[0]);
	
	ASSERT(_range[1] != nullptr);
	char const * maximum = static_cast<char const *>(_range[1]);
	
	std::ptrdiff_t difference = maximum - minimum;
	ASSERT(difference > 0);
	
	return static_cast<std::size_t>(difference);
}

char * const StackFrameBuffer::null = nullptr;
