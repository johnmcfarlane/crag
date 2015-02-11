//
//  Ordering.h
//  crag
//
//  Created by John on 2015-02-11.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace crag
{
	namespace core
	{
		// remembers the relative order of a collection of objects
		template<typename T>
		class Ordering
		{
			/////////////////////////////////////////////////////////////////////////////
			// types

			using Value = T;

			using Comparison = signed char;
			using ComparisonVector = std::vector <Comparison>;

			// how a values compares against all the others
			struct Comparisons
			{
				ComparisonVector comparisons;
				Value value;
			};
			typedef std::vector <Comparisons> ComparisonTable;
		public:
			typedef std::size_t Index;

			/////////////////////////////////////////////////////////////////////////////
			// functions

			// returns true iff this changed
			bool SetComparison(Index lhs, Index rhs, Comparison comparison)
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
				CRAG_VERIFY_OP(_comparison, !=, comparison);

				// set the 1st order comparison
				_comparison = comparison;

				// set the mirror value
				_table[rhs].comparisons[lhs] = - comparison;

				// propagate the ordering to other pairs
				auto& rhs_comparisons = _table[rhs].comparisons;
				for (Index index = 0; index != size; ++ index)
				{
					if (index != lhs)
					{
						auto comparison2 = rhs_comparisons[index];
						if (comparison2 * comparison >= 0)
						{
							SetComparison(lhs, index, comparison);
						}
					}
				}

				SetComparison(rhs, lhs, - comparison);

				CRAG_VERIFY(* this);
				return true;
			}

			Comparison GetComparison(Index lhs, Index rhs) const
			{
				CRAG_VERIFY(* this);
				return _table[lhs].comparisons[rhs];
			}

			Index GetIndex(Value const & value)
			{
				auto found = std::find_if(_table.begin(), _table.end(), [value] (Comparisons row)
				{
					return row.value == value;
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
					found->value = value;

					CRAG_VERIFY(* this);
				}

				std::size_t index = found - _table.begin();
				ASSERT(index < _table.size());

				return index;
			}

			Value const & GetValue(Index index) const
			{
				CRAG_VERIFY(* this);
				static_assert(std::is_unsigned<Index>::value, "Need to check for negative value as well");
				CRAG_VERIFY_OP(index, <, _table.size());

				return _table[index].value;
			}

			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Ordering, object)
				auto size = object._table.size();
				for (Ordering::Index row_index = 0; row_index != size; ++ row_index)
				{
					auto & row = object._table[row_index].comparisons;
					CRAG_VERIFY_EQUAL(object._table.size(), row.size());

					for (Ordering::Index column_index = 0; column_index != size; ++ column_index)
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
			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

			void Dump() const
			{
				auto size = _table.size();

				std::cout << "  ";
				for (Ordering::Index column_index = 0; column_index != size; ++ column_index)
				{
					std::cout << ' ' << column_index << ' ';
				}
				std::cout << std::endl;

				for (Index row_index = 0; row_index != size; ++ row_index)
				{
					auto & row = _table[row_index].comparisons;
					CRAG_VERIFY_EQUAL(_table.size(), row.size());

					std::cout << row_index << ' ';
					for (Index column_index = 0; column_index != size; ++ column_index)
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

		private:

			ComparisonTable _table;
		};
	}
}