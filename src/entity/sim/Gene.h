//
//  entity/sim/Genome.h
//  crag
//
//  Created by John McFarlane on 2013-03-27.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <core/fixed_point.h>

class Random;

namespace sim
{
	namespace ga
	{
		// a unit of artifical evolution in a genetic algorithm;
		// stored as sequences in Genome;
		// used to express an individual characteristic of an animat
		class Gene
		{
			// types
		public:
			using unit_repr_type = std::uint32_t;
			using unit_type = crag::core::closed_unit<unit_repr_type>;
			using value_type = float;

			// functions
			CRAG_VERIFY_INVARIANTS_DECLARE(Gene);

			Gene() noexcept {}
			Gene(Random & r) noexcept;
			Gene(Gene parent1, Gene parent2, Random & r, float mutation_rate) noexcept;

			// return a value in the range [0,1]
			value_type closed() const noexcept;

			// return a value in the range (0,1)
			value_type open() const noexcept;

			static Gene from_repr(unit_repr_type repr) noexcept;
			unit_repr_type to_repr() const noexcept;

		private:
			unit_type _value;
		};
	}
}
