//
//  Roster.h
//  crag
//
//  Created by John on 2013-03-08.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace core
{
	namespace locality
	{
		class Function;
		bool operator == (Function lhs, Function rhs);
		bool operator != (Function lhs, Function rhs);
		bool operator < (Function lhs, Function rhs);

		// the member function to be called for an object;
		// WARNING: not particularly compliant
		class Function
		{
			////////////////////////////////////////////////////////
			// types

			// the choice of Function here is entirely arbitrary;
			// (it's the closest class to hand)
#if ! defined(WIN32)
			typedef Function ArbitraryClass;
#else
			struct ArbitraryBaseClass1 { virtual ~ArbitraryBaseClass1() { } };
			struct ArbitraryBaseClass2 { virtual ~ArbitraryBaseClass2() { } };
			class ArbitraryClass : public ArbitraryBaseClass1, ArbitraryBaseClass2 { };
#endif
			typedef void (ArbitraryClass::* ArbitraryMemberFunctionType)();

			struct FunctionPointerBuffer
			{
				void * array[2];
			};

			template <typename MemberFunctionType>
			union FunctionPointerTransmogrifier
			{
				MemberFunctionType member_function;
				FunctionPointerBuffer buffer;
			};
		public:
			Function();
			Function(Function const & rhs);

			template <typename CLASS>
			Function(void (CLASS::* member_function)())
			{
				Set<CLASS, decltype(member_function)>(member_function);
			}

			template <typename CLASS>
			Function(void (CLASS::* member_function)() const)
			{
				Set<CLASS, decltype(member_function)>(member_function);
			}

			bool operator == (std::nullptr_t) const;
			bool operator != (std::nullptr_t) const;

			friend bool operator == (Function lhs, Function rhs);
			friend bool operator != (Function lhs, Function rhs);

			friend bool operator < (Function lhs, Function rhs);
			void operator() (void * object) const;

			void Dump() const;

		private:
			template <typename CLASS, typename FUNCTION>
			void Set(FUNCTION member_function)
			{
				static_assert(sizeof(FunctionPointerBuffer) >= sizeof(member_function), "FunctionPointerBuffer isn't big enough to store given function pointer");
				static_assert(sizeof(ArbitraryMemberFunctionType) >= sizeof(member_function), "ArbitraryMemberFunctionType is not the same size as non-arbitrary equivalent");
				FunctionPointerTransmogrifier<decltype(member_function)> transmogrifier;
				transmogrifier.buffer.array[0] = transmogrifier.buffer.array[1] = nullptr;
				transmogrifier.member_function = member_function;
				_function = transmogrifier.buffer;
			}

			template <typename CLASS, void (CLASS::*FUNCTION)()>
			static void CallMemberFunction(CLASS & object)
			{
				(object.*FUNCTION)();
			}

			FunctionPointerBuffer _function;
		};

		// remembers the relative order in which a collection of Functions should be 
		// called; infers orderings; asserts if contradictory orderings are given
		class Ordering
		{
			/////////////////////////////////////////////////////////////////////////////
			// types

			typedef signed char Comparison;
			typedef std::vector<Comparison> ComparisonVector;

			// how a function compares against all the others
			struct FunctionComparisons
			{
				ComparisonVector comparisons;
				Function function;
			};
			typedef std::vector<FunctionComparisons> ComparisonTable;
		public:
			typedef std::size_t FunctionIndex;

			/////////////////////////////////////////////////////////////////////////////
			// functions

			// returns true iff this changed
			bool SetComparison(FunctionIndex lhs, FunctionIndex rhs, Comparison comparison);
			Comparison GetComparison(FunctionIndex lhs, FunctionIndex rhs) const;

			FunctionIndex GetFunctionIndex(Function function);
			Function GetFunction(FunctionIndex function_index) const;

			CRAG_VERIFY_INVARIANTS_DECLARE(Ordering);

			void Dump() const;
		private:

			ComparisonTable _table;
		};

		// collects together pairs of objects and functions to be called
		// on those object; accepts ordering constraints based on function
		// to ensure all pairs with a certain function are called before
		// all pairs with a certain other function; otherwise attempts to
		// exploit spatial locality as much as possible with calling order
		class Roster
		{
			OBJECT_NO_COPY(Roster);

			////////////////////////////////////////////////////////////////////////////////
			// types

			typedef Ordering::FunctionIndex FunctionIndex;
			struct Command
			{
				void * object;
				FunctionIndex function_index;

				bool operator==(Command rhs) const;
				bool operator!=(Command rhs) const;
				
				void Dump() const;
			};

			typedef std::vector<Command> CommandVector;

			////////////////////////////////////////////////////////////////////////////////
			// functions
			
		public:
			Roster();
			~Roster();

			CRAG_VERIFY_INVARIANTS_DECLARE(Roster);

			void Call();

			// tells this Roster that commands of lhs_function 
			// must all be called before rhs_function
			void AddOrdering(Function lhs_function, Function rhs_function);

			template <typename Class>
			void AddCommand(Class & object, Function function)
			{
				AddCommand(CreateCommand(object, function));
			}

			template <typename Class>
			void RemoveCommand(Class & object, Function function)
			{
				RemoveCommand(CreateCommand(object, function));
			}

		private:
			template <typename Class>
			Command CreateCommand(Class & object, Function function)
			{
				Command command = 
				{
					static_cast<void *>(& object),
					GetFunctionIndex(function)
				};
				return command;
			}

			void AddCommand(Command command);
			void RemoveCommand(Command command);

			CommandVector::iterator Search(Command command);
			CommandVector::iterator Find(Command command);

			void Sort();
			bool LessThan(Command lhs, Command rhs) const;

			FunctionIndex GetFunctionIndex(Function function);
			Function GetFunction(FunctionIndex function_index) const;
			
			void Dump() const;

			////////////////////////////////////////////////////////////////////
			// variables

			// the pairs of functions which dictate the orderings;
			// cannot contain circular references
			Ordering _ordering;

			// ordered array of commands
			CommandVector _commands;
		};
	}
}
