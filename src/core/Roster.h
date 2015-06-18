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
		// container of explicitly ordered functions, (a.k.a. commands);
		// used to execute complex sequences of components in deterministic order
		class Roster
		{
		public:
			////////////////////////////////////////////////////////////////////////////////
			// public types

			using key_type = std::uintptr_t;
			using value_type = std::function<void()>;

			////////////////////////////////////////////////////////////////////////////////
			// functions

			Roster() = default;
			OBJECT_NO_COPY(Roster);

			CRAG_VERIFY_INVARIANTS_DECLARE(Roster);

			// add a command to be called
			void AddCommand(key_type key, value_type function) noexcept;

			// stipulate that one command is to be called before another
			void AddOrdering(key_type lhs, key_type rhs) noexcept;

			// call all the commands
			void Call() noexcept;

		private:
			void Sort() noexcept;

			////////////////////////////////////////////////////////////////////
			// private types
			using Ordering = crag::core::Ordering<key_type>;
			using OrderingIterationType = Ordering::const_iterator;

			struct Command
			{
				// the function called from Roster::Call()
				value_type function;

				// key_type iterator; our handle into ordering
				OrderingIterationType ordering_iterator;
			};

			using CommandVector = std::vector<Command>;

			////////////////////////////////////////////////////////////////////
			// variables

			// array of commands sorted as specified _ordering
			CommandVector _commands;

			// the pairs of functions which dictate the orderings;
			// cannot contain circular references
			Ordering _ordering;
		};
	}
}
