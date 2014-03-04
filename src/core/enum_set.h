//
//  enum_set.h
//  crag
//
//  Created by John McFarlane on 2014-03-01.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <bitset>

namespace core
{
	// a bit set with an enum as the key
	template <typename EnumType, EnumType value_limit>
	class enum_set
	{
		// types
		using bitset = std::bitset<std::size_t(value_limit)>;
	public:
		using reference = typename bitset::reference;
		
		// functions
	private:
		enum_set(bitset const & bits) : _bits(bits) { }
	public:
		enum_set() = default;
		enum_set(enum_set const &) = default;
		enum_set(EnumType pos)
		{
			set(pos);
		}

		enum_set & operator=(enum_set const &) = default;
		
		friend enum_set operator | (enum_set const & lhs, enum_set const & rhs)
		{
			return enum_set(lhs._bits | rhs._bits);
		}
		
		friend enum_set operator & (enum_set const & lhs, enum_set const & rhs)
		{
			return enum_set(lhs._bits & rhs._bits);
		}
		
		// sets a bit
		enum_set & set(EnumType pos, bool value = true)
		{
			auto index = ::std::size_t(pos);
			ASSERT(index >= 0);

			_bits.set(index, value);

			return * this;
		}
		
		// sets all bits to true
		enum_set & set()
		{
			_bits.set();
			return * this;
		}
		
		// sets all bits to false
		enum_set & reset()
		{
			_bits.reset();
			return * this;
		}
		
		bool operator[] (EnumType pos) const
		{
			ASSERT(pos < value_limit);

			auto index = ::std::size_t(pos);
			ASSERT(index >= 0);

			return _bits[index];
		}
		
		reference operator[] (EnumType pos)
		{
			ASSERT(pos < value_limit);

			auto index = ::std::size_t(pos);
			ASSERT(index >= 0);

			return _bits[index];
		}
		
	private:
		// variables
		bitset _bits;
	};
}
