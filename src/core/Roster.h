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
		// the member function to be called for an object;
		// WARNING: not particularly compliant
		class Function
		{
		public:
			Function()
			: _function(nullptr)
			{
			}

			Function(Function const & rhs)
			: _function(rhs._function)
			{
			}

			template <typename CLASS>
			Function(void (CLASS::* member_function)())
			{
				static_assert(sizeof(member_function) == sizeof(void *), "this ain't gonna work...");
				union
				{
					void (CLASS::* mf)();
					void * vp;
				} u;
				u.mf = member_function;
				_function = u.vp;
			}

			bool operator == (std::nullptr_t) const
			{
				return _function == nullptr;
			}

			bool operator != (std::nullptr_t) const
			{
				return _function != nullptr;
			}

			friend bool operator == (Function lhs, Function rhs)
			{
				return lhs._function == rhs._function;
			}

			friend bool operator != (Function lhs, Function rhs)
			{
				return lhs._function != rhs._function;
			}

			friend bool operator < (Function lhs, Function rhs)
			{
				return lhs._function < rhs._function;
			}

			void operator() (void * object) const
			{
				VerifyObject(* this);

				// the choice of Function here is entirely arbitrary
				typedef Function ArbitraryClass;
				typedef void (ArbitraryClass::* MemberFunctionType)();
				static_assert(sizeof(MemberFunctionType) == sizeof(void *), "this ain't gonna work...");

				union
				{
					MemberFunctionType mf;
					void * vp;
				} u;

				u.vp = _function;
				ArbitraryClass & surrogate = * static_cast<ArbitraryClass *>(object);
				(surrogate.*u.mf)();
			}

#if defined(VERIFY)
			void Verify() const
			{
				VerifyTrue(_function != nullptr);
			}
#endif

		private:
			template <typename CLASS, void (CLASS::*FUNCTION)()>
			static void CallMemberFunction(CLASS & object)
			{
				(object.*FUNCTION)();
			}

			void * _function;
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

			void SetComparison(FunctionIndex lhs, FunctionIndex rhs, Comparison comparison);
			Comparison GetComparison(FunctionIndex lhs, FunctionIndex rhs) const;

			FunctionIndex GetFunctionIndex(Function function);
			Function GetFunction(FunctionIndex function_index) const;

#if defined(VERIFY)
			void Verify() const;
#endif
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

			typedef Ordering::FunctionIndex FunctionIndex;
			struct Command
			{
				void * object;
				FunctionIndex function_index;
			};

			typedef std::vector<Command> CommandVector;
		public:
			Roster();
			~Roster();

#if defined(VERIFY)
			void Verify() const;
#endif

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

			void Sort();

			FunctionIndex GetFunctionIndex(Function function);
			Function GetFunction(FunctionIndex function_index) const;

			////////////////////////////////////////////////////////////////////
			// variables

			// ordered array of commands
			CommandVector _commands;

			// the pairs of functions which dictate the orderings;
			// cannot contain circular references
			Ordering _ordering;

			// true iff _commands is no longer ordered
			bool _is_ordered;
		};
	}
}
