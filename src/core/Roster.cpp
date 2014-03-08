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

void Function::Dump() const
{
	std::cout << _function.array[0] << ',' << _function.array[1];
}

////////////////////////////////////////////////////////////////////////////////
// core::locality::Ordering member definitions

bool Ordering::SetComparison(FunctionIndex lhs, FunctionIndex rhs, Comparison comparison)
{
	CRAG_VERIFY(* this);

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

	CRAG_VERIFY(* this);
	return true;
}

Ordering::Comparison Ordering::GetComparison(FunctionIndex lhs, FunctionIndex rhs) const
{
	CRAG_VERIFY(* this);
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

		CRAG_VERIFY(* this);
	}
	
	std::size_t index = found - _table.begin();
	ASSERT(index < _table.size());

	return index;
}

Function Ordering::GetFunction(FunctionIndex function_index) const
{
	CRAG_VERIFY(* this);
	static_assert(std::is_unsigned<FunctionIndex>::value, "Need to check for negative value as well");
	CRAG_VERIFY_OP(function_index, <, _table.size());

	return _table[function_index].function;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Ordering, object)
	auto size = object._table.size();
	for (Ordering::FunctionIndex row_index = 0; row_index != size; ++ row_index)
	{
		auto & row = object._table[row_index].comparisons;
		CRAG_VERIFY_EQUAL(object._table.size(), row.size());

		for (Ordering::FunctionIndex column_index = 0; column_index != size; ++ column_index)
		{
			auto cell = row[column_index];

			// verify cell
			CRAG_VERIFY_OP(cell, >=, -1);
			CRAG_VERIFY_OP(cell, <=, 1);

			// verify that c=r diagonal is clear
			if (row_index == column_index)
			{
				CRAG_VERIFY_EQUAL(cell, 0);
				continue;
			}
			else
			{
				// verify that [c][r] == -[r][c]
				CRAG_VERIFY_EQUAL(int(cell), int(- object._table[column_index].comparisons[row_index]));
			}
		}
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Ordering::Dump() const
{
	auto size = _table.size();

	std::cout << "  ";
	for (Ordering::FunctionIndex column_index = 0; column_index != size; ++ column_index)
	{
		std::cout << ' ' << column_index << ' ';
	}
	std::cout << std::endl;

	for (Ordering::FunctionIndex row_index = 0; row_index != size; ++ row_index)
	{
		auto & row = _table[row_index].comparisons;
		CRAG_VERIFY_EQUAL(_table.size(), row.size());

		std::cout << row_index << ' ';
		for (Ordering::FunctionIndex column_index = 0; column_index != size; ++ column_index)
		{
			auto cell = row[column_index];
			
			switch (cell)
			{
			case -1:
				std::cout << "-1 ";
				break;
			case 0:
				std::cout << " 0 ";
				break;
			case +1:
				std::cout << "+1 ";
				break;
			
			default:
				DEBUG_BREAK("Bad enum value, %d", cell);
				break;
			}
		}
		
		std::cout << std::endl;
	}
}

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
		CRAG_VERIFY_TRUE(self._ordering.GetFunction(f) != nullptr);
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

#if ! defined(NDEBUG)
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

void Roster::Dump() const
{
	_ordering.Dump();
	
	for (auto const & command : _commands)
	{
		command.Dump();
	}
}
