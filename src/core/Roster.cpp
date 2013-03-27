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

using namespace core::locality;

////////////////////////////////////////////////////////////////////////////////
// core::locality::Function member definitions

Function::Function()
{
	_function.array[0] = nullptr;
	_function.array[1] = nullptr;
}

Function::Function(Function const & rhs)
: _function(rhs._function)
{
}

bool Function::operator == (std::nullptr_t) const
{
	return (* this) == Function();
}

bool Function::operator != (std::nullptr_t) const
{
	return (* this) != Function();
}

bool core::locality::operator == (Function lhs, Function rhs)
{
	return lhs._function.array[0] == rhs._function.array[0]
	&&	lhs._function.array[1] == rhs._function.array[1];
}

bool core::locality::operator != (Function lhs, Function rhs)
{
	return lhs._function.array[0] != rhs._function.array[0]
	||	lhs._function.array[1] != rhs._function.array[1];
}

bool core::locality::operator < (Function lhs, Function rhs)
{
	return lhs._function.array[0] < rhs._function.array[0]
	|| (	lhs._function.array[0] > rhs._function.array[0] 
	&&	lhs._function.array[1] < rhs._function.array[1]);
}

void Function::operator() (void * object) const
{
	static_assert(sizeof(FunctionPointerBuffer) >= sizeof(ArbitraryMemberFunctionType), "this ain't gonna work...");

	FunctionPointerTransmogrifier<ArbitraryMemberFunctionType> transmogrifier;
	transmogrifier.buffer = _function;

	ArbitraryClass & surrogate = * static_cast<ArbitraryClass *>(object);
	ArbitraryMemberFunctionType member_function = transmogrifier.member_function;
	(surrogate.*member_function)();
}

////////////////////////////////////////////////////////////////////////////////
// core::locality::Ordering member definitions

bool Ordering::SetComparison(FunctionIndex lhs, FunctionIndex rhs, Comparison comparison)
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
		return false;
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
	for (FunctionIndex function_index = 0; function_index != size; ++ function_index)
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
	return true;
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

		for (FunctionIndex column_index = 0; column_index != size; ++ column_index)
		{
			auto cell = row[column_index];

			// verify cell
			VerifyOp(cell, >=, -1);
			VerifyOp(cell, <=, 1);

			// verify that c=r diagonal is clear
			if (row_index == column_index)
			{
				VerifyEqual(cell, 0);
				continue;
			}
			else
			{
				// verify that [c][r] == -[r][c]
				VerifyEqual(int(cell), int(- _table[column_index].comparisons[row_index]));
			}
		}
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
// core::locality::Roster member definitions

bool core::locality::Roster::Command::operator==(Command rhs) const
{
		return object == rhs.object
		&& function_index == rhs.function_index;
}

bool core::locality::Roster::Command::operator!=(Command rhs) const
{
	return ! operator==(rhs);
}

Roster::Roster()
{
	VerifyObject(* this);
}

Roster::~Roster()
{
	VerifyObject(* this);

	if (! _commands.empty())
	{
		DEBUG_BREAK(SIZE_T_FORMAT_SPEC " commands remaining", _commands.size());
	}
}

#if defined(VERIFY)
void Roster::Verify() const
{
	// verify order
	VerifyTrue(std::is_sorted(std::begin(_commands), std::end(_commands), [=] (Command lhs, Command rhs)
	{
		return LessThan(lhs, rhs);
	}));

	_ordering.Verify();
}
#endif

void Roster::AddOrdering(Function lhs_function, Function rhs_function)
{
	VerifyObject(* this);
	ASSERT(lhs_function != rhs_function);

	auto lhs_index = GetFunctionIndex(lhs_function);
	auto rhs_index = GetFunctionIndex(rhs_function);
	ASSERT(lhs_index != rhs_index);

	if (_ordering.SetComparison(lhs_index, rhs_index, -1))
	{
		Sort();

		VerifyObject(* this);

#if defined(VERIFY)
	// exhaustive comparison ensures complete integrity of ordering
	auto end = std::end(_commands);
	for (auto lhs = std::begin(_commands); lhs != end; ++ lhs)
	{
		for (auto rhs = lhs; ++ rhs != end; )
		{
			VerifyTrue(! LessThan(* rhs, * lhs));
		}
	};
#endif
	}
}

// TODO: separate sorting into essential sorting and incremental sorting;
// essential sorting would ensure function order; incremental sorting could
// bubble sort for locality of reference during Call loop
void Roster::Call()
{
	VerifyObject(* this);

	// perform all calls
	for (auto command : _commands)
	{
		auto object = command.object;
		auto function = GetFunction(command.function_index);
		function(object);
	}

	VerifyObject(* this);
}

void Roster::AddCommand(Command command)
{
	VerifyObject(* this);
	ASSERT(Find(command) == _commands.end());

	auto insertion_position = Search(command);
	ASSERT(insertion_position == _commands.end() || * insertion_position != command);
	ASSERT(insertion_position == _commands.end() || ! LessThan(* insertion_position, command));

	_commands.insert(insertion_position, command);

	VerifyObject(* this);
}

void Roster::RemoveCommand(Command command)
{
	auto found = Find(command);
	
	if (found == _commands.end())
	{
		DEBUG_BREAK("command not found");
		return;
	}

	_commands.erase(found);

	VerifyObject(* this);
}

Roster::CommandVector::iterator Roster::Search(Command command)
{
	VerifyObject(* this);

	auto found = std::lower_bound(_commands.begin(), _commands.end(), command, [=] (Command lhs, Command rhs) {
		return LessThan(lhs, rhs);
	});

	return found;
}

Roster::CommandVector::iterator Roster::Find(Command command)
{
	Roster::CommandVector::iterator found = Search(command);

	Roster::CommandVector::iterator end = _commands.end();
	if (found != end)
	{
		Command f = * found;
		if (f != command)
		{
			found = end;
		}
	}

#if ! defined(NDEBUG)
	auto slow_found = std::find(_commands.begin(), _commands.end(), command);
	ASSERT(found == slow_found);
#endif

	return found;
}

void Roster::Sort()
{
	std::sort(_commands.begin(), _commands.end(), [=] (Command lhs, Command rhs) {
		return LessThan(lhs, rhs);
	});
}

bool Roster::LessThan(Command lhs, Command rhs) const
{
		auto lhs_index = lhs.function_index;
		auto rhs_index = rhs.function_index;
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
			return lhs.object < rhs.object;
		}
}

Roster::FunctionIndex Roster::GetFunctionIndex(Function function)
{
	return _ordering.GetFunctionIndex(function);
}

Function Roster::GetFunction(FunctionIndex function_index) const
{
	return _ordering.GetFunction(function_index);
}
