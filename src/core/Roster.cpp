//
//  Roster.cpp
//  crag
//
//  Created by John on 2013-03-08.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "pch.h"

#include "Roster.h"

using namespace core::locality;

////////////////////////////////////////////////////////////////////////////////
// Ordering member definitions

void Ordering::SetComparison(FunctionIndex lhs, FunctionIndex rhs, Comparison comparison)
{
	VerifyObject(* this);

	// get table size
	auto size = _table.size();

	// check input
	ASSERT(lhs != rhs);
	ASSERT(comparison == -1 || comparison == 1);
	ASSERT(lhs < size);
	ASSERT(rhs < size);

	auto & _comparison = _table[lhs].comparisons[rhs];
	if (_comparison == comparison)
	{
		// fully expect duplicate calls
		return;
	}

	// this pair should not already have been set to something different;
	// it means there are conflicting ordering pairs
	ASSERT(_comparison == 0);

	// set the 1st order comparison
	_comparison = comparison;

	// set the mirror value
	_table[rhs].comparisons[lhs] = - comparison;

	// propagate the ordering to other pairs
	auto& rhs_comparisons = _table[rhs].comparisons;
	for (auto function_index = 0; function_index != size; ++ function_index)
	{
		if (function_index != lhs)
		{
			auto comparison2 = rhs_comparisons[function_index];
			if (comparison2 * comparison >= 0)
			{
				SetComparison(lhs, function_index, comparison);
			}
		}
	}

	SetComparison(rhs, lhs, - comparison);

	VerifyObject(* this);
}

Ordering::Comparison Ordering::GetComparison(FunctionIndex lhs, FunctionIndex rhs) const
{
	VerifyObject(* this);
	return _table[lhs].comparisons[rhs];
}

Ordering::FunctionIndex Ordering::GetFunctionIndex(Function function)
{
	auto found = std::find_if(_table.begin(), _table.end(), [function] (FunctionComparisons row) 
	{
		return row.function == function;
	});

	if (found == _table.end())
	{
		auto new_size = _table.size() + 1;
		for (auto & row : _table)
		{
			row.comparisons.resize(new_size);
		}

		_table.emplace_back();
		found = _table.end();
		-- found;

		found->comparisons.resize(new_size);
		found->function = function;

		VerifyObject(* this);
	}
	
	std::size_t index = found - _table.begin();
	ASSERT(index < _table.size());

	return index;
}

Function Ordering::GetFunction(FunctionIndex function_index) const
{
	VerifyObject(* this);
	ASSERT(function_index < _table.size());
	return _table[function_index].function;
}

#if defined(VERIFY)
void Ordering::Verify() const
{
	auto size = _table.size();
	for (FunctionIndex row_index = 0; row_index != size; ++ row_index)
	{
		auto & row = _table[row_index].comparisons;
		VerifyEqual(_table.size(), row.size());

		for (auto column_index = 0; column_index != size; ++ column_index)
		{
			auto cell = row[column_index];
			if (row_index == column_index)
			{
				VerifyEqual(cell, 0);
				continue;
			}

			VerifyOp(cell, >=, -1);
			VerifyOp(cell, <=, 1);
			VerifyEqual(int(cell), int(- _table[column_index].comparisons[row_index]));
		}
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Roster member definitions

Roster::Roster()
: _is_ordered(true)
{
	VerifyObject(* this);
}

Roster::~Roster()
{
	VerifyObject(* this);
	ASSERT(_commands.empty());
}

#if defined(VERIFY)
void Roster::Verify() const
{
	// verify order
	if (_is_ordered)
	{
		auto end = std::end(_commands);
		for (auto lhs = std::begin(_commands); lhs != end; ++ lhs)
		{
			for (auto rhs = lhs; ++ rhs != end; )
			{
				VerifyTrue(_ordering.GetComparison(rhs->function_index, lhs->function_index) <= 0);
			}
		};
	}

	_ordering.Verify();
}
#endif

void Roster::AddOrdering(Function lhs_function, Function rhs_function)
{
	ASSERT(lhs_function != rhs_function);

	auto lhs_index = GetFunctionIndex(lhs_function);
	auto rhs_index = GetFunctionIndex(rhs_function);
	ASSERT(lhs_index != rhs_index);

	_ordering.SetComparison(lhs_index, rhs_index, -1);
}

void Roster::Call()
{
	// make sure calls are ordered
	if (! _is_ordered)
	{
		Sort();
		_is_ordered = true;
	}

	// perform all calls
	for (auto command : _commands)
	{
		auto object = command.object;
		auto function = GetFunction(command.function_index);
		function(object);
	}
}

void Roster::AddCommand(Command command)
{
	_commands.push_back(command);
	_is_ordered = false;
}

void Roster::RemoveCommand(Command command)
{
	auto found = std::find_if(_commands.begin(), _commands.end(), [command] (Command candidate) {
		return command.object == candidate.object
			&& command.function_index == candidate.function_index;
	});
	
	if (found == _commands.end())
	{
		DEBUG_BREAK("command not found");
		return;
	}

	_commands.erase(found);
}

void Roster::Sort()
{
	VerifyObject(* this);

	std::sort(_commands.begin(), _commands.end(), [this] (Command lhs, Command rhs) {
		auto lhs_index = lhs.function_index;
		auto rhs_index = rhs.function_index;
		auto comparison = _ordering.GetComparison(lhs_index, rhs_index);
		return comparison < 0;
	});

	VerifyObject(* this);
}

Roster::FunctionIndex Roster::GetFunctionIndex(Function function)
{
	return _ordering.GetFunctionIndex(function);
}

Function Roster::GetFunction(FunctionIndex function_index) const
{
	return _ordering.GetFunction(function_index);
}
