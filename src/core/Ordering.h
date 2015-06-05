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
		// a set-like container which stores explicit pair-wise comparisons
		template<typename T>
		class Ordering
		{
			/////////////////////////////////////////////////////////////////////////////
			// types

			using ComparisonIntType = std::int8_t;
			enum class ComparisonType : ComparisonIntType
			{
				less_than = -1,
				equal_to = 0,
				greater_than = 1
			};

		public:
			using value_type = T;
			using size_type = std::size_t;

			// Ordering iterators are valid throughout lifetime of container instance
			class const_iterator
			{
				friend Ordering;

				constexpr const_iterator(Ordering const * o, size_type i) noexcept : ordering(o), index(i) { }
			public:
				constexpr const_iterator() = default;

				constexpr value_type const & operator*() const noexcept
				{
					return ordering->value_buffer[index];
				}

				friend const_iterator & operator++(const_iterator & i) noexcept
				{
					CRAG_VERIFY_TRUE(i.ordering);
					CRAG_VERIFY_OP(i.index, <, i.ordering->size());
					++ i.index;
					return i;
				}

				friend constexpr bool operator==(const_iterator const & lhs, const_iterator const & rhs) noexcept
				{
					return lhs.index == rhs.index && lhs.ordering == rhs.ordering;
				}

				friend constexpr bool operator!=(const_iterator const & lhs, const_iterator const & rhs) noexcept
				{
					return ! operator==(lhs, rhs);
				}

			private:
				Ordering const * ordering = nullptr;
				size_type index = 0;
			};

			class iterator : public const_iterator
			{
				friend Ordering;

				constexpr iterator(Ordering * o, size_type i) noexcept : const_iterator(o, i) { }
			};

			/////////////////////////////////////////////////////////////////////////////
			// functions

			constexpr size_type size() const noexcept
			{
				return value_buffer.size();
			}

			constexpr const_iterator begin() const noexcept
			{
				return const_iterator(this, 0);
			}

			iterator begin() noexcept
			{
				return iterator(this, 0);
			}

			constexpr const_iterator end() const noexcept
			{
				return const_iterator(this, size());
			}

			iterator end() noexcept
			{
				return iterator(this, size());
			}

			// enforces comparison, (lhs < rhs); see set_comparison for details
			bool set_less_than(iterator lhs, iterator rhs) noexcept
			{
				return set_comparison(lhs, rhs, ComparisonType::less_than);
			}

			// enforces comparison, (lhs > rhs); see set_comparison for details
			bool set_greater_than(iterator lhs, iterator rhs) noexcept
			{
				return set_comparison(lhs, rhs, ComparisonType::greater_than);
			}

			constexpr bool less_than(const_iterator lhs, const_iterator rhs) const noexcept
			{
				return get_comparison(lhs, rhs) == ComparisonType::less_than;
			}

			constexpr bool greater_than(const_iterator lhs, const_iterator rhs) const noexcept
			{
				return get_comparison(lhs, rhs) == ComparisonType::greater_than;
			}

			iterator find(value_type const & value) noexcept
			{
				auto buffer_begin = std::begin(value_buffer);
				auto buffer_end = std::end(value_buffer);
				auto found = std::find(buffer_begin, buffer_end, value);
				return iterator(this, std::distance(buffer_begin, found));
			}

			const_iterator find(value_type const & value) const noexcept
			{
				auto non_const_this = const_cast<Ordering *>(this);
				return non_const_this->find(value);
			}

			// very similar to std::set::insert;
			// insertion is permanent; there is no erase
			std::pair<iterator, bool> insert(value_type const & value) noexcept
			{
				auto found = find(value);
				if (found != end())
				{
					return std::make_pair(found, false);
				}

				auto old_size = value_buffer.size();
				auto new_size = old_size + 1;

				// create new buffer into which the larger data will be moved
				// TODO: consider a non-moving version; no need for exception safety
				auto new_ordering = Ordering();
				new_ordering.comparison_buffer.reserve(new_size * new_size);
				new_ordering.value_buffer.reserve(new_size);

				// comparison buffer / table
				for (auto i = std::begin(comparison_buffer); i != std::end(comparison_buffer); i += old_size)
				{
					auto new_buffer_end = std::end(new_ordering.comparison_buffer);
					new_ordering.comparison_buffer.insert(new_buffer_end, i, i + old_size);
					new_ordering.comparison_buffer.push_back(ComparisonType::equal_to);
				}
				auto new_buffer_end = std::end(new_ordering.comparison_buffer);
				new_ordering.comparison_buffer.insert(new_buffer_end, new_size, ComparisonType::equal_to);

				// buffer
				new_ordering.value_buffer.insert(std::begin(new_ordering.value_buffer), std::begin(value_buffer), std::end(value_buffer));
				new_ordering.value_buffer.push_back(value);

				std::swap(* this, new_ordering);

				CRAG_VERIFY_EQUAL(value_buffer.size(), new_size);
				CRAG_VERIFY_EQUAL(comparison_buffer.size(), new_size * new_size);
				CRAG_VERIFY(* this);

				// return iterator to new element
				return std::make_pair(iterator(this, old_size), true);
			}

			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Ordering, object)
				auto size = object.value_buffer.size();
				CRAG_VERIFY_EQUAL(object.comparison_buffer.size(), size * size);

				for (auto row = std::begin(object); row != std::end(object); ++ row)
				{
					for (auto column = std::begin(object); column != std::end(object); ++ column)
					{
						auto cell = static_cast<ComparisonIntType>(object.get_ref(row, column));

						// verify cell
						CRAG_VERIFY_OP(cell, >=, -1);
						CRAG_VERIFY_OP(cell, <=, 1);

						// verify that c=r diagonal is clear
						if (row == column)
						{
							CRAG_VERIFY_EQUAL(cell, 0);
							continue;
						}
						else
						{
							// verify that [c][r] == -[r][c]
							CRAG_VERIFY_EQUAL(cell, - static_cast<ComparisonIntType>(object.get_ref(column, row)));
						}
					}
				}
			CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

			void Dump() const
			{
				auto size = this->size();

				std::cout << "  ";
				for (auto column_index = 0; column_index != size; ++ column_index)
				{
					std::cout << ' ' << column_index << ' ';
				}
				std::cout << std::endl;

				for (auto row_index = 0; row_index != size; ++ row_index)
				{
					std::cout << row_index << ' ';
					for (auto column_index = 0; column_index != size; ++ column_index)
					{
						auto cell = get_ref(row_index, column_index);

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
			// stipulates the relative order of lhs and rhs;
			// returns true iff this changed;
			// asserts iff the given comparison contradicts previous stipulation
			bool set_comparison(iterator lhs, iterator rhs, ComparisonType comparison) noexcept
			{
				CRAG_VERIFY(* this);

				// check input
				VerifyElement(lhs);
				VerifyElement(rhs);
				ASSERT(lhs != rhs);
				ASSERT(comparison == ComparisonType::less_than || comparison == ComparisonType::greater_than);

				ComparisonType & _comparison = get_ref(lhs, rhs);
				if (_comparison == comparison)
				{
					// fully expect duplicate calls
					return false;
				}

				// this pair should not already have been set to something different;
				// it means there are conflicting ordering pairs
				CRAG_VERIFY_FALSE(_comparison == comparison);

				// set the 1st order comparison
				_comparison = comparison;

				// set the mirror value
				get_ref(rhs, lhs) = Negative(comparison);

				// propagate the ordering to other pairs

				for (auto rhs2 = begin(); rhs2 != end(); ++ rhs2)
				{
					if (rhs2 != lhs)
					{
						auto comparison2 = get_ref(rhs, rhs2);
						if (static_cast<ComparisonIntType>(comparison2) * static_cast<ComparisonIntType>(comparison) >= 0)
						{
							set_comparison(lhs, rhs2, comparison);
						}
					}
				}

				set_comparison(rhs, lhs, Negative(comparison));

				CRAG_VERIFY(* this);
				return true;
			}

			constexpr ComparisonType get_comparison(const_iterator lhs, const_iterator rhs) const noexcept
			{
				return get_ref(lhs, rhs);
			}

			ComparisonType & get_ref(iterator & lhs, iterator & rhs) noexcept
			{
				return comparison_buffer[get_index(lhs, rhs)];
			}

			constexpr ComparisonType const & get_ref(const_iterator & lhs, const_iterator & rhs) const noexcept
			{
				return comparison_buffer[get_index(lhs, rhs)];
			}

			constexpr size_type get_index(const_iterator & lhs, const_iterator & rhs) const noexcept
			{
				return (lhs.index * size()) + rhs.index;
			}

			void VerifyIterator(const_iterator const & i) const noexcept
			{
				CRAG_VERIFY_EQUAL(i.ordering, this);
				CRAG_VERIFY_OP(i.index, >=, size_type(0));
				CRAG_VERIFY_OP(i.index, <=, size());
			}

			void VerifyElement(const_iterator const & i) const noexcept
			{
				VerifyIterator(i);
				CRAG_VERIFY_OP(i.index, !=, size());
			}

			constexpr static ComparisonIntType ToInt(ComparisonType comparison) noexcept
			{
				return static_cast<ComparisonIntType>(comparison);
			}

			constexpr static ComparisonType ToEnum(ComparisonIntType comparison) noexcept
			{
				return static_cast<ComparisonType>(comparison);
			}

			constexpr static ComparisonType Negative(ComparisonType comparison) noexcept
			{
				return ToEnum(- ToInt(comparison));
			}

			constexpr static ComparisonType Product(ComparisonType lhs, ComparisonType rhs) noexcept
			{
				return ToEnum(ToInt(lhs) * ToInt(rhs));
			}

			// variables
			std::vector<value_type> value_buffer;
			std::vector <ComparisonType> comparison_buffer;
		};
	}
}
