//
//  core/HashString.h
//  crag
//
//  Created by John McFarlane on 2014-01-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace crag
{
	namespace core
	{
		// simple string-to-hash class used to identify resources
		class HashString
		{
			////////////////////////////////////////////////////////////////////////////////
			// types
		
			typedef std::size_t ValueType;

			friend struct std::hash<HashString>;
		
			////////////////////////////////////////////////////////////////////////////////
			// functions
		
		public:
			constexpr HashString(char const * string)
			: _value(Calculate(string))
#if defined(CRAG_DEBUG)
			, _debug_string(string)
#endif
			{
			}
		
			friend bool operator<(HashString const & lhs, HashString const & rhs)
			{
				return lhs._value < rhs._value;
			}

			friend bool operator==(HashString const & lhs, HashString const & rhs)
			{
				return lhs._value == rhs._value;
			}

			friend bool operator!=(HashString const & lhs, HashString const & rhs)
			{
				return lhs._value != rhs._value;
			}

			friend std::ostream & operator << (std::ostream & lhs, HashString const & rhs)
			{
#if defined(CRAG_DEBUG)
				return lhs << rhs._value << '(' << rhs._debug_string << ')';
#else
				return lhs << rhs._value;
#endif
			}

		private:
			static constexpr ValueType Calculate(ValueType hash, char const * str)
			{
				return ( !*str ? hash :
				Calculate(((hash << 5) + hash) + *str, str + 1));
			}

			static constexpr ValueType Calculate(char const * str)
			{
				return ( !str ? 0 :
				Calculate(5381, str));
			}
		
			////////////////////////////////////////////////////////////////////////////////
			// variables
		
			ValueType _value;
#if defined(CRAG_DEBUG)
			char const * _debug_string;
#endif
		};
	}
}

namespace std
{
	template <>
	struct hash<crag::core::HashString>
	{
		size_t operator()(::crag::core::HashString const & key) const
		{
			return key._value;
		}
	};
}
