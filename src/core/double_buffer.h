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

		// general-purpose c'tor; given arguments are passed to c'tors of both elements
		template <typename ... ARGS>
		double_buffer(ARGS ... args)
		{
			new (data() + 0) value_type (args ...);
			new (data() + 1) value_type (args ...);
			front_buffer = data() + 0;
			back_buffer = data() + 1;

			VerifyObject(* this);
		}
		
		// copy c'tor
		double_buffer(double_buffer const & rhs)
		{
			VerifyObjectRef(rhs);
			
			new (data() + 0) value_type (rhs[0]);
			new (data() + 1) value_type (rhs[1]);
			copy_polarity(rhs);
			
			VerifyObject(* this);
		}
		
		// copy operator
		double_buffer & operator = (double_buffer const & rhs)
		{
			VerifyObject(* this);
			VerifyObjectRef(rhs);
			
			data()[0] = rhs[0];
			data()[1] = rhs[1];
			copy_polarity(rhs);
			
			VerifyObject(* this);
			return * this;
		}
		
		// front buffer accessors
		value_type & front()
		{
			VerifyObject(* this);
			
			return * front_buffer;
		}
		
		value_type const & front() const
		{
			VerifyObject(* this);

			return * front_buffer;
		}
		
		// back buffer accessors
		value_type & back()
		{
			VerifyObject(* this);

			return * back_buffer;
		}
		
		value_type const & back() const
		{
			VerifyObject(* this);

			return * back_buffer;
		}
		
		// subscript operator
		value_type & operator [] (int n)
		{
			VerifyObject(* this);
			ASSERT(n == 0 || n == 1);

			return data()[n];
		}
		
		value_type const & operator [] (int n) const
		{
			VerifyObject(* this);
			ASSERT(n == 0 || n == 1);

			return data()[n];
		}
		
		// raw data accessor
		value_type * data()
		{
			return reinterpret_cast<value_type *>(storage);
		}
		
		value_type const * data() const
		{
			return reinterpret_cast<value_type const *>(storage);
		}
		
		// swaps the front and back buffers
		void flip()
		{
			VerifyObject(* this);

			std::swap(front_buffer, back_buffer);
		}
		
#if defined(VERIFY)
		void Verify() const
		{
			VerifyObject(data()[0]);
			VerifyObject(data()[1]);
			VerifyArrayElement(front_buffer, data() + 0, data() + 2);
			VerifyArrayElement(back_buffer, data() + 0, data() + 2);
			VerifyTrue(front_buffer != back_buffer);
		}
#endif
		
	private:
		void copy_polarity(const double_buffer & rhs)
		{
			size_t front_buffer_index = rhs.front_buffer - rhs.data();
			front_buffer = data() + front_buffer_index;
			back_buffer = data() + (front_buffer_index ^ 1);
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables

		// enough bytes to store two objects
		// TODO: Investigate initializer_list as way to replace char with value_type.
		char storage [sizeof(value_type) * 2];
		
		// pointers to each of the two objects
		value_type * front_buffer;
		value_type * back_buffer;
	};
	
}
