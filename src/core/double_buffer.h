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
			CRAG_VERIFY(* this);
		}
		
		// general-purpose c'tor; given arguments are passed to c'tors of both elements
		template <typename ... ARGS>
		double_buffer(std::initializer_list<ARGS ...> args)
			: _buffer({args, args})
			, _front_buffer(& _buffer[0])
			, _back_buffer(& _buffer[1])
		{
			double_buffer();

			CRAG_VERIFY(* this);
		}
		
		// copy c'tor
		double_buffer(double_buffer const & rhs)
			: _buffer(rhs._buffer)
		{
			CRAG_VERIFY(rhs);
			
			copy_polarity(rhs);
			
			CRAG_VERIFY(* this);
		}
		
		// copy operator
		double_buffer & operator = (double_buffer const & rhs)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
			
			_buffer[0] = rhs[0];
			_buffer[1] = rhs[1];
			copy_polarity(rhs);
			
			CRAG_VERIFY(* this);
			return * this;
		}
		
		// front buffer accessors
		value_type & front()
		{
			CRAG_VERIFY(* this);
			
			return * _front_buffer;
		}
		
		value_type const & front() const
		{
			CRAG_VERIFY(* this);

			return * _front_buffer;
		}
		
		// back buffer accessors
		value_type & back()
		{
			CRAG_VERIFY(* this);

			return * _back_buffer;
		}
		
		value_type const & back() const
		{
			CRAG_VERIFY(* this);

			return * _back_buffer;
		}
		
		value_type * begin()
		{
			CRAG_VERIFY(* this);
			
			return std::begin(_buffer);
		}
		
		value_type const * begin() const
		{
			CRAG_VERIFY(* this);
			
			return std::begin(_buffer);
		}
		
		value_type * end()
		{
			CRAG_VERIFY(* this);
			
			return std::end(_buffer);
		}
		
		value_type const * end() const
		{
			CRAG_VERIFY(* this);
			
			return std::end(_buffer);
		}
		
		// subscript operator
		value_type & operator [] (int n)
		{
			CRAG_VERIFY(* this);
			ASSERT(n == 0 || n == 1);

			return _buffer[n];
		}
		
		value_type const & operator [] (int n) const
		{
			CRAG_VERIFY(* this);
			ASSERT(n == 0 || n == 1);

			return _buffer[n];
		}
		
		// swaps the front and back buffers
		void flip()
		{
			CRAG_VERIFY(* this);

			std::swap(_front_buffer, _back_buffer);
		}
		
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(double_buffer, self)
			CRAG_VERIFY(self._buffer[0]);
			CRAG_VERIFY(self._buffer[1]);
			CRAG_VERIFY_ARRAY_ELEMENT(self._front_buffer, &self._buffer[0], &self._buffer[0]+2);
			CRAG_VERIFY_ARRAY_ELEMENT(self._back_buffer, &self._buffer[0], &self._buffer[0]+2);
			CRAG_VERIFY_TRUE(self._front_buffer != self._back_buffer);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
		
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
