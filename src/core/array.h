//
//  array.h
//  crag
//
//  Created by John McFarlane on 10/13/11.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace core
{
	// This class is holding the place of std::array until crag upgrades to c++11.
	template <typename ELEMENT, int SIZE>
	class array
	{
	public:
		// types
		typedef ELEMENT * iterator;
		typedef ELEMENT const * const_iterator;
		
		// functions
		iterator begin() { return _buffer; }
		const_iterator begin() const { return _buffer; }
		iterator end() { return _buffer + SIZE; }
		const_iterator end() const { return _buffer + SIZE; }

	private:
		// variables
		ELEMENT _buffer[SIZE];
	};
}