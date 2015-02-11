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

void Roster::Command::Dump() const
{
	std::cout << function_index << ' ' << object << std::endl;
}

Roster::Roster()
{
	CRAG_VERIFY(* this);
}

Roster::~Roster()
{
	CRAG_VERIFY(* this);

	if (! _commands.empty())
	{
		DEBUG_BREAK(SIZE_T_FORMAT_SPEC " commands remaining", _commands.size());
	}
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Roster, self)
	CRAG_VERIFY(self._ordering);
	
	for (auto & command: self._commands)
	{
		auto f = command.function_index;
		CRAG_VERIFY_TRUE(self._ordering.GetValue(f));
	}
	
	auto begin = std::begin(self._commands);
	auto end = std::end(self._commands);

	// verify order
	CRAG_VERIFY_TRUE(std::is_sorted(begin, end, [& self] (Command lhs, Command rhs)
	{
		return self.LessThan(lhs, rhs);
	}));

#if 1	// expensive
	// exhaustive comparison ensures complete integrity of ordering
	for (auto rhs = begin; rhs != end; ++ rhs)
	{
		auto const & rhs_command = * rhs;
		for (auto lhs = begin; lhs != rhs; ++ lhs)
		{
			auto const & lhs_command = * lhs;
			CRAG_VERIFY_TRUE(! self.LessThan(rhs_command, lhs_command));
		}
	}
#endif
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Roster::AddOrdering(Function lhs_function, Function rhs_function)
{
	CRAG_VERIFY(* this);
	ASSERT(lhs_function != rhs_function);

	auto lhs_index = GetFunctionIndex(lhs_function);
	auto rhs_index = GetFunctionIndex(rhs_function);
	ASSERT(lhs_index != rhs_index);

	if (_ordering.SetComparison(lhs_index, rhs_index, -1))
	{
		Sort();
	}
	
	CRAG_VERIFY(* this);
}

// TODO: separate sorting into essential sorting and incremental sorting;
// essential sorting would ensure function order; incremental sorting could
// bubble sort for locality of reference during Call loop
void Roster::Call()
{
	CRAG_VERIFY(* this);

	// perform all calls
	for (auto command : _commands)
	{
		auto object = command.object;
		auto function = GetFunction(command.function_index);
		function(object);
	}

	CRAG_VERIFY(* this);
}

void Roster::AddCommand(Command command)
{
	CRAG_VERIFY(* this);
	ASSERT(Find(command) == std::end(_commands));

	auto insertion_position = Search(command);
	ASSERT(insertion_position == std::end(_commands) || * insertion_position != command);
	ASSERT(insertion_position == std::end(_commands) || ! LessThan(* insertion_position, command));

	_commands.insert(insertion_position, command);

	CRAG_VERIFY(* this);
}

void Roster::RemoveCommand(Command command)
{
	auto found = Find(command);
	
	if (found == std::end(_commands))
	{
		DEBUG_BREAK("command not found");
		return;
	}

	_commands.erase(found);

	CRAG_VERIFY(* this);
}

Roster::CommandVector::iterator Roster::Search(Command command)
{
	CRAG_VERIFY(* this);

	auto found = std::lower_bound(_commands.begin(), _commands.end(), command, [=] (Command lhs, Command rhs) {
		return LessThan(lhs, rhs);
	});

	return found;
}

Roster::CommandVector::iterator Roster::Find(Command command)
{
	Roster::CommandVector::iterator found = Search(command);

	Roster::CommandVector::iterator end = std::end(_commands);
	if (found != end)
	{
		Command f = * found;
		if (f != command)
		{
			found = end;
		}
	}

#if defined(CRAG_DEBUG)
	auto slow_found = std::find(std::begin(_commands), std::end(_commands), command);
	ASSERT(found == slow_found);
#endif

	return found;
}

void Roster::Sort()
{
	std::sort(std::begin(_commands), std::end(_commands), [=] (Command lhs, Command rhs) {
		return LessThan(lhs, rhs);
	});
}

// establishes Command order
bool Roster::LessThan(Command lhs, Command rhs) const
{
		auto lhs_index = lhs.function_index;
		auto rhs_index = rhs.function_index;
		if (lhs_index == rhs_index)
		{
			return lhs.object < rhs.object;
		}
		
		auto comparison = _ordering.GetComparison(lhs_index, rhs_index);
		switch (comparison)
		{
		case -1:
			return true;
		case 1:
			return false;
		default:
			DEBUG_BREAK("invaid value, %d, returned by GetComparison", int(comparison));
		case 0:
			return lhs_index < rhs_index;
		}
}

Roster::FunctionIndex Roster::GetFunctionIndex(Function function)
{
	return _ordering.GetIndex(function);
}

Roster::Function Roster::GetFunction(FunctionIndex function_index) const
{
	return _ordering.GetValue(function_index);
}

void Roster::Dump() const
{
	_ordering.Dump();
	
	for (auto const & command : _commands)
	{
		command.Dump();
	}
}
