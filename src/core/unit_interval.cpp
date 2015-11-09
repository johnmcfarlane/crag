//
//  core/unit_interval.cpp
//  crag
//
//  Created by John McFarlane on 2015-09-23.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include <pch.h>

#include "unit_interval.h"

using namespace crag::core;
using std::uint8_t;
using std::numeric_limits;

template <class ReprType>
struct UnitIntervalTests
{
	static_assert(sizeof(unit_interval<ReprType>) == sizeof(ReprType), "UnitIntervalTests test failed");

	static_assert(static_cast<long double>(unit_interval<ReprType>(1.L)) == 1.L, "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(1).template integral<int>() == numeric_limits<int>::max(), "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(1.L).data() == numeric_limits<ReprType>::max(), "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(1).data() == numeric_limits<ReprType>::max(), "UnitIntervalTests test failed");

	static_assert(static_cast<long double>(unit_interval<ReprType>(0.L).data()) == 0.L, "UnitIntervalTests test failed");
	static_assert(static_cast<int>(unit_interval<ReprType>(0).data()) == 0, "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(0.L).data() == 0, "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(0).data() == 0, "UnitIntervalTests test failed");
};

template <class ReprType>
struct SignedUnitIntervalTests : public UnitIntervalTests<ReprType>
{
	static_assert(static_cast<long double>(unit_interval<ReprType>(-1.L).data()) == -1.L, "UnitIntervalTests test failed");
	static_assert(static_cast<int>(unit_interval<ReprType>(-1).data()) == -1, "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(-1.L).data() == -1, "UnitIntervalTests test failed");
	static_assert(unit_interval<ReprType>(-1).data() == -1, "UnitIntervalTests test failed");
};

template struct UnitIntervalTests<uint8_t>;
template struct UnitIntervalTests<int8_t>;
template struct UnitIntervalTests<uint16_t>;
template struct UnitIntervalTests<int16_t>;
template struct UnitIntervalTests<uint32_t>;
template struct UnitIntervalTests<int32_t>;
template struct UnitIntervalTests<uint64_t>;
template struct UnitIntervalTests<int64_t>;
