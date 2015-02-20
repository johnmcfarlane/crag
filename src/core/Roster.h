//
//  Roster.h
//  crag
//
//  Created by John on 2013-03-08.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Ordering.h"

namespace crag
{
	namespace core
	{
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

			// the function to be called for an object;
			class Function
			{
				////////////////////////////////////////////////////////
				// types

				using VoidFunctionPointerParameter = void *;
				using VoidFunctionPointer = void (*) (VoidFunctionPointerParameter);

				template <typename CLASS>
				using TypedFunctionPointerParameter = CLASS *;

				template <typename CLASS>
				using TypedFunctionPointer = void (*) (TypedFunctionPointerParameter<CLASS>);

			public:
				Function() = default;
				Function(Function const & rhs) = default;

				template <typename CLASS>
				Function(TypedFunctionPointer<CLASS> typed_function)
				{
					Set<CLASS>(typed_function);
				}

				operator bool () const
				{
					return _function_pointer != nullptr;
				}

				friend bool operator == (Function lhs, Function rhs)
				{
					return lhs._function_pointer == rhs._function_pointer;
				}

				friend bool operator != (Function lhs, Function rhs)
				{
					return lhs._function_pointer != rhs._function_pointer;
				}

				void operator() (void * object) const
				{
					// undefined behaviour
					_function_pointer(object);
				}

			private:
				template <typename CLASS>
				void Set(TypedFunctionPointer<CLASS> typed_function_pointer)
				{
					static_assert(
							sizeof(TypedFunctionPointerParameter<CLASS>) == sizeof(VoidFunctionPointerParameter),
							"bad assumption about ability to cast between typed and untyped functions");
					static_assert(
							sizeof(TypedFunctionPointer<CLASS>) == sizeof(VoidFunctionPointer),
							"bad assumption about ability to cast between typed and untyped functions");

					_function_pointer = reinterpret_cast<VoidFunctionPointer>(typed_function_pointer);
				}

				VoidFunctionPointer _function_pointer = nullptr;
			};

			using Ordering = crag::core::Ordering<Function>;
			using FunctionIndex = Ordering::Index;
			struct Command
			{
				void * object;
				FunctionIndex function_index;

				bool operator == (Command rhs) const
				{
					return object == rhs.object
							&& function_index == rhs.function_index;
				}

				bool operator != (Command rhs) const
				{
					return object != rhs.object
							|| function_index != rhs.function_index;
				}
				
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
