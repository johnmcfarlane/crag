//
//  enum_bitset.h
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
	class enum_bitset
	{
		// types
		using bitset = std::bitset<std::size_t(value_limit)>;
	public:
		using reference = typename bitset::reference;
		
		// functions
	private:
		enum_bitset(bitset const & bits) : _bits(bits) { }
	public:
		enum_bitset() = default;
		enum_bitset(enum_bitset const &) = default;
		enum_bitset(EnumType pos)
		{
			set(pos);
		}

		enum_bitset & operator=(enum_bitset const &) = default;
		
		friend enum_bitset operator | (enum_bitset const & lhs, enum_bitset const & rhs)
		{
			return enum_bitset(lhs._bits | rhs._bits);
		}
		
		friend enum_bitset operator & (enum_bitset const & lhs, enum_bitset const & rhs)
		{
			return enum_bitset(lhs._bits & rhs._bits);
		}
		
		friend enum_bitset operator &= (enum_bitset & lhs, enum_bitset const & rhs)
		{
			lhs._bits &= rhs._bits;
			return rhs._bits;
		}
		
		bool any() const
		{
			return _bits.any();
		}
		
		// sets a bit
		enum_bitset & set(EnumType pos, bool value = true)
		{
			auto index = ::std::size_t(pos);
			ASSERT(index < _bits.size());

			_bits.set(index, value);

			return * this;
		}
		
		// sets all bits to true
		enum_bitset & set()
		{
			_bits.set();
			return * this;
		}
		
		// sets all bits to false
		enum_bitset & reset()
		{
			_bits.reset();
			return * this;
		}
		
		bool operator[] (EnumType pos) const
		{
			ASSERT(pos < value_limit);

			auto index = ::std::size_t(pos);
			ASSERT(index < _bits.size());

			return _bits[index];
		}
		
		reference operator[] (EnumType pos)
		{
			ASSERT(pos < value_limit);

			auto index = ::std::size_t(pos);
			ASSERT(index < _bits.size());

			return _bits[index];
		}
		
	private:
		// variables
		bitset _bits;
	};
}
