//
//  core/TypeId.h
//  crag
//
//  Created by John McFarlane on 2014-01-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

namespace crag
{
	namespace core
	{
		// forward-declarations
		class TypeId;
		bool operator<(TypeId const & lhs, TypeId const & rhs);
		bool operator==(TypeId const & lhs, TypeId const & rhs);
		bool operator!=(TypeId const & lhs, TypeId const & rhs);
		std::ostream & operator << (std::ostream & lhs, TypeId const & rhs);
		std::istream & operator >> (std::istream & lhs, TypeId & rhs);

		////////////////////////////////////////////////////////////////////////////////
		// TypeId class definition
		
		// identifies a particular type at run time
		class TypeId
		{
			////////////////////////////////////////////////////////////////////////////////
			// functions
			
			TypeId(int value);
		public:
			TypeId();

			template <typename Type>
			static TypeId Create();
			
			CRAG_VERIFY_INVARIANTS_DECLARE(TypeId);
		
			friend bool operator<(TypeId const & lhs, TypeId const & rhs);
			friend bool operator==(TypeId const & lhs, TypeId const & rhs);
			friend bool operator!=(TypeId const & lhs, TypeId const & rhs);
		
			friend std::ostream & operator << (std::ostream & lhs, TypeId const & rhs);
			friend std::istream & operator >> (std::istream & lhs, TypeId & rhs);
		
		private:
			////////////////////////////////////////////////////////////////////////////////
			// variables
			
			int _value;
		};

		////////////////////////////////////////////////////////////////////////////////
		// TypeId helpers

		// not thread-safe
		int GetUniqueId();
		
		// thread-safe
		template <typename Type>
		struct TypeIdGenerator
		{
			static int const _value;
		};

		template <typename Type>
		int const TypeIdGenerator<Type>::_value = GetUniqueId();

		////////////////////////////////////////////////////////////////////////////////
		// TypeId function template definitions

		template <typename Type>
		TypeId TypeId::Create() 
		{
			return TypeId(TypeIdGenerator<Type>::_value); 
		}
	}
}
