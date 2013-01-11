//
//  double_buffer.h
//  crag
//
//  Created by John on 9/17/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace core
{
	
	// Super-simple stl-style implementation of double-buffering.
	// Really double-buffering is a subset of what this class can do.
	// But the technique is best known for its application to buffers.
	
	template <typename T> class double_buffer
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef T value_type;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions

		double_buffer()
			: _front_buffer(& _buffer[0])
			, _back_buffer(& _buffer[1])
		{
			VerifyObject(* this);
		}
		
		// general-purpose c'tor; given arguments are passed to c'tors of both elements
		template <typename ... ARGS>
		double_buffer(std::initializer_list<ARGS ...> args)
			: _buffer({args, args})
			, _front_buffer(& _buffer[0])
			, _back_buffer(& _buffer[1])
		{
			double_buffer();

			VerifyObject(* this);
		}
		
		// copy c'tor
		double_buffer(double_buffer const & rhs)
			: _buffer(rhs._buffer)
		{
			VerifyObjectRef(rhs);
			
			copy_polarity(rhs);
			
			VerifyObject(* this);
		}
		
		// copy operator
		double_buffer & operator = (double_buffer const & rhs)
		{
			VerifyObject(* this);
			VerifyObjectRef(rhs);
			
			_buffer[0] = rhs[0];
			_buffer[1] = rhs[1];
			copy_polarity(rhs);
			
			VerifyObject(* this);
			return * this;
		}
		
		// front buffer accessors
		value_type & front()
		{
			VerifyObject(* this);
			
			return * _front_buffer;
		}
		
		value_type const & front() const
		{
			VerifyObject(* this);

			return * _front_buffer;
		}
		
		// back buffer accessors
		value_type & back()
		{
			VerifyObject(* this);

			return * _back_buffer;
		}
		
		value_type const & back() const
		{
			VerifyObject(* this);

			return * _back_buffer;
		}
		
		// subscript operator
		value_type & operator [] (int n)
		{
			VerifyObject(* this);
			ASSERT(n == 0 || n == 1);

			return _buffer[n];
		}
		
		value_type const & operator [] (int n) const
		{
			VerifyObject(* this);
			ASSERT(n == 0 || n == 1);

			return _buffer[n];
		}
		
		// swaps the front and back buffers
		void flip()
		{
			VerifyObject(* this);

			std::swap(_front_buffer, _back_buffer);
		}
		
#if defined(VERIFY)
		void Verify() const
		{
			VerifyObject(_buffer[0]);
			VerifyObject(_buffer[1]);
			VerifyArrayElement(_front_buffer, &_buffer[0], &_buffer[0]+2);
			VerifyArrayElement(_back_buffer, &_buffer[0], &_buffer[0]+2);
			VerifyTrue(_front_buffer != _back_buffer);
		}
#endif
		
	private:
		void copy_polarity(const double_buffer & rhs)
		{
			size_t front_buffer_index = rhs._front_buffer - rhs._buffer;
			_front_buffer = _buffer + front_buffer_index;
			_back_buffer = _buffer + (front_buffer_index ^ 1);
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables

		std::array<value_type, 2> _buffer;
		
		// pointers to each of the two objects in _buffer
		value_type * _front_buffer;
		value_type * _back_buffer;
	};
	
}
