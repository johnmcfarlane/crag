//
//  core/RosterObjectDefine.h
//  crag
//
//  Created by John McFarlane on 2015-05-29.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ConfigEntry.h"
#include "iterable_object_pool.h"
#include "Roster.h"

namespace crag
{
	namespace core
	{
		namespace impl
		{
			// a class which owns POOL_OBJECT's allocation object
			// and establishes visiting patterns to its member functions;
			// do NOT use directly; use via CRAG_ROSTER_OBJECT_DEFINE
			template <typename POOL_OBJECT>
			class iterable_object_pool_helper
			{
			public:
				// types
				using value_type = POOL_OBJECT;
				using pool_type = crag::core::iterable_object_pool<value_type>;

				// a function object that adds a command to the given roster which
				// passes each instance of OBJECT_TYPE from the given pool to FUNCTION
				template <typename OBJECT_TYPE, void (OBJECT_TYPE::*FUNCTION)()>
				struct CallBase
				{
					constexpr CallBase(crag::core::Roster & roster) noexcept
						: roster(roster)
					{
					}

					CallBase & operator=(CallBase const &) noexcept = delete;

					void operator() (pool_type & p) const noexcept
					{
						auto current_key = OrderingKeyCast<OBJECT_TYPE, FUNCTION>();

						roster.AddCommand(current_key, [& p]()
						{
							p.for_each([] (OBJECT_TYPE & o)
							{
								(o.*FUNCTION)();
							});
						});
					}

					crag::core::Roster & roster;
				};

				// a function object that adds a command to the given roster which
				// passes each instance of POOL_OBJECT from the given pool to FUNCTION
				template <void (value_type::*FUNCTION)()>
				struct Call : CallBase<value_type, FUNCTION>
				{
					using Base = CallBase<value_type, FUNCTION>;
					constexpr Call(crag::core::Roster & roster) noexcept
						: Base(roster)
					{
					}
				};

				// used in cases where only the storage of objects is required
				struct NoCall
				{
					constexpr NoCall() noexcept { }
					void operator() (pool_type &) const noexcept { }
				};

				// as Call but also stipulates that FUNCTION be called /before/ LATTER_FUNCTION
				template <
					void (value_type::*FUNCTION)(),
					typename LATTER_OBJECT, void (LATTER_OBJECT::*LATTER_FUNCTION)()>
				struct CallBefore
					: Call<FUNCTION>
				{
					using Base = Call<FUNCTION>;

					constexpr CallBefore(crag::core::Roster & roster) noexcept
						: Base(roster)
					{
					}

					void operator() (pool_type & p) const noexcept
					{
						Base::operator() (p);
						Order<value_type, FUNCTION, LATTER_OBJECT, LATTER_FUNCTION>(Base::roster);
					}
				};

				// as Call but also stipulates that FUNCTION be called /after/ LATTER_FUNCTION
				template <
					void (value_type::*FUNCTION)(),
					typename FORMER_OBJECT, void (FORMER_OBJECT::*FORMER_FUNCTION)()>
				struct CallAfter
					: Call<FUNCTION>
				{
					using Base = Call<FUNCTION>;

					constexpr CallAfter(crag::core::Roster & roster) noexcept
						: Base(roster)
					{
					}

					void operator() (pool_type & pool) const noexcept
					{
						Base::operator() (pool);
						Order<FORMER_OBJECT, FORMER_FUNCTION, value_type, FUNCTION>(Base::roster);
					}
				};

				// same as Call
				// but also stipulates that FUNCTION be called /before/ LATTER_FUNCTION and /after/ LATTER_FUNCTION
				template <
					void (value_type::*FUNCTION)(),
					typename FORMER_OBJECT, void (FORMER_OBJECT::*FORMER_FUNCTION)(),
					typename LATTER_OBJECT, void (LATTER_OBJECT::*LATTER_FUNCTION)()>
				struct CallBetween : Call<FUNCTION>
				{
					using Base = Call<FUNCTION>;

					constexpr CallBetween(crag::core::Roster & roster) noexcept
						: Base(roster)
					{
					}

					void operator() (pool_type & pool) const noexcept
					{
						Base::operator() (pool);
						Order<FORMER_OBJECT, FORMER_FUNCTION, value_type, FUNCTION>(Base::roster);
						Order<value_type, FUNCTION, LATTER_OBJECT, LATTER_FUNCTION>(Base::roster);
					}
				};

				// stipulates that FORMER_FUNCTION be called /before/ LATTER_FUNCTION; used by Call-derived classes
				template <
					typename FORMER_OBJECT, void (FORMER_OBJECT::*FORMER_FUNCTION)(),
					typename LATTER_OBJECT, void (LATTER_OBJECT::*LATTER_FUNCTION)()>
				static void Order(crag::core::Roster & roster) noexcept
				{
					roster.AddOrdering(
						OrderingKeyCast<FORMER_OBJECT, FORMER_FUNCTION>(),
						OrderingKeyCast<LATTER_OBJECT, LATTER_FUNCTION>());
				}

				// functions
				iterable_object_pool_helper() = delete;
				iterable_object_pool_helper(iterable_object_pool_helper const &) = delete;
				iterable_object_pool_helper(iterable_object_pool_helper &&) = delete;

				iterable_object_pool_helper(int pool_capacity) noexcept
					: pool(pool_capacity)
				{
				}

				template <typename HEADS, typename ... TAIL>
				iterable_object_pool_helper(int pool_capacity, HEADS && head, TAIL && ... tail)
					: iterable_object_pool_helper(pool_capacity, tail ...)
				{
					head(pool);
				}

				pool_type & GetPool()
				{
					return pool;
				}

			private:
				// hack to provide a compile-time mapping from member function pointer to free function point
				template <typename OBJECT_TYPE, void (OBJECT_TYPE::*FUNCTION)()>
				static constexpr Roster::key_type OrderingKeyCast() noexcept
				{
					return reinterpret_cast<Roster::key_type>(
						reinterpret_cast<void *>(
							static_cast<void(*) (OBJECT_TYPE &)>(
								&OrderingKeyCastDummy<OBJECT_TYPE, FUNCTION>)));
				}

				// helper for OrderingKeyCast function template
				template <typename OBJECT_TYPE, void (OBJECT_TYPE::*FUNCTION)()>
				static void OrderingKeyCastDummy(OBJECT_TYPE &) noexcept
				{
				}

				// variables
				pool_type pool;
			};
		}
	}
}

// inserted into an OBJECT's source file;
// is passed rules governing invocation of OBJECT's member functions
#define CRAG_ROSTER_OBJECT_DEFINE(OBJECT, CAPACITY, ...) \
	namespace { CONFIG_DEFINE(max_num_##OBJECT, CAPACITY); } \
	crag::core::iterable_object_pool<OBJECT> & OBJECT::GetPool() noexcept \
	{ \
		using Pool = crag::core::impl::iterable_object_pool_helper<OBJECT>; \
		static Pool helper(max_num_##OBJECT, __VA_ARGS__); \
		return helper.GetPool(); \
	} \
	void* OBJECT::operator new(size_t sz) noexcept \
	{ \
		CRAG_VERIFY_EQUAL(sz, sizeof(OBJECT)); \
		return GetPool().allocate(); \
	} \
	void* OBJECT::operator new [](size_t) noexcept \
	{ \
		DEBUG_BREAK("cannot allocate arrays of this object"); \
		return nullptr; \
	} \
	void OBJECT::operator delete(void * p) noexcept \
	{ \
		GetPool().free(p); \
	} \
	void OBJECT::operator delete [](void *) noexcept \
	{ \
		DEBUG_BREAK("cannot allocate arrays of this object"); \
	}

// passed a reference of OBJECT in order to perform run-time check against its storage
#define CRAG_ROSTER_OBJECT_VERIFY(OBJECT) ASSERT(GetPool().is_allocated(OBJECT));
