/*
 *  DoubleBuffer.h
 *  crag
 *
 *  Created by John on 9/17/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace core
{
	
	// Super-simple stl-style implementation of double-buffering.
	// Really double-buffering is a subset of what this class can do.
	// But the technique is best known for its application to buffers.
	
	template <typename T> class double_buffer
	{
	public:
		// c'tor
		double_buffer()
		: front_buffer(buffers + 0)
		, back_buffer(buffers + 1)
		{
		}
		
		double_buffer(double_buffer const & rhs)
		: buffers(rhs.buffers)
		{
			set_pointers(rhs);
		}
		
		// copy
		double_buffer & operator = (double_buffer const & rhs)
		{
			buffers[0] = rhs.buffers[0];
			buffers[1] = rhs.buffers[1];
			
			set_pointers(rhs);
			
			return * this;
		}
		
		// front buffer accessors
		T & front()
		{
			return * front_buffer;
		}
		
		T const & front() const
		{
			return * front_buffer;
		}
		
		// back buffer accessors
		T & back()
		{
			return * back_buffer;
		}
		
		T const & back() const
		{
			return * back_buffer;
		}
		
		// index accessors
		T & operator [] (int n)
		{
			Assert(n == 0 || n == 1);
			return buffers [n];
		}
		
		T const & operator [] (int n) const
		{
			Assert(n == 0 || n == 1);
			return buffers [n];
		}
		
		// flip the front and back buffers
		void flip()
		{
			std::swap(front_buffer, back_buffer);
		}
		
	private:
		
		void set_pointers(double_buffer const & that)
		{
			int front_index = that.buffers - that.front_buffer;			
			front_buffer = buffers + front_index;
			back_buffer = buffers + (front_index ^ 1);
		}
		
		T buffers[2];
		T * front_buffer;
		T * back_buffer;
	};
	
}
