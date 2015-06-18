//
//  Roster.cpp
//  crag
//
//  Created by John on 2013-03-08.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Roster.h"

using namespace crag::core;

////////////////////////////////////////////////////////////////////////////////
// crag::core::Roster member definitions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Roster, self)
	CRAG_VERIFY(self._ordering);

	for (auto & command: self._commands)
	{
		// all iterators should be initialized
		auto i = command.ordering_iterator;
		CRAG_VERIFY_TRUE(* i);
	}

	auto begin = std::begin(self._commands);
	auto end = std::end(self._commands);

	// verify order
	CRAG_VERIFY_TRUE(std::is_sorted(begin, end, [& self] (Command lhs, Command rhs)
	{
		return self._ordering.less_than(lhs.ordering_iterator, rhs.ordering_iterator);
	}));

#if 1	// expensive
	// exhaustive comparison ensures complete integrity of ordering
	for (auto rhs = begin; rhs != end; ++ rhs)
	{
		auto const & rhs_command = * rhs;
		for (auto lhs = begin; lhs != rhs; ++ lhs)
		{
			auto const & lhs_command = * lhs;
			CRAG_VERIFY_TRUE(! self._ordering.less_than(rhs_command.ordering_iterator, lhs_command.ordering_iterator));
		}
	}
#endif
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Roster::AddCommand(key_type key, value_type function) noexcept
{
	// make sure same key hasn't already been added
	ASSERT(std::find_if(std::begin(_commands), std::end(_commands), [key] (Command const & command)
	{
		return * command.ordering_iterator == key;
	}) == std::end(_commands));

	// insert the key into the ordering and get an iterator back
	auto insertion = _ordering.insert(key);

	// use it to populate a new element of _commands
	_commands.push_back({ std::move(function), insertion.first });

	// if the key was NOT inserted into ordering,
	if (! insertion.second)
	{
		// then presumably AddOrdering was previous added to AddOrdering
		// and the position of the new command might already matter
		Sort();
	}

	CRAG_VERIFY(* this);
}

void Roster::AddOrdering(Roster::key_type lhs, Roster::key_type rhs) noexcept
{
	CRAG_VERIFY(* this);

	auto lhs_iterator = _ordering.insert(lhs).first;
	auto rhs_iterator = _ordering.insert(rhs).first;
	ASSERT(lhs_iterator != rhs_iterator);

	// if _ordering is mutated
	if (_ordering.set_less_than(lhs_iterator, rhs_iterator))
	{
		// re-sort commands
		Sort();
	}

	CRAG_VERIFY(* this);
}

// call the commands
void Roster::Call() noexcept
{
	CRAG_VERIFY(* this);

	for (auto & command : _commands)
	{
		command.function();
	}
}

// sort _commands based on _ordering
void Roster::Sort() noexcept
{
	std::sort(std::begin(_commands), std::end(_commands), [=] (Command const & lhs, Command const & rhs)
	{
		return _ordering.less_than(lhs.ordering_iterator, rhs.ordering_iterator);
	});
}
