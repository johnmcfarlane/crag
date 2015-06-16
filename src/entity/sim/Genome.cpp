//
//  scripts/ga/AnimatController.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Genome.h"

#include <core/ConfigEntry.h>
#include "core/Random.h"

using namespace sim;
using namespace sim::ga;

namespace
{
	CONFIG_DEFINE(mutation_rate, .01f);
}

////////////////////////////////////////////////////////////////////////////////
// sim::ga::Genome member definitions

Genome::Genome(Genome const & parent1, Genome const & parent2)
{
	CRAG_VERIFY_EQUAL(parent1.size(), parent2.size());

	for (
		auto
			iterators = std::make_pair(std::begin(parent1), std::begin(parent2));
			iterators.first != std::end(parent1);
			++ iterators.second, ++ iterators.first)
	{
		CRAG_VERIFY_TRUE(iterators.second != std::end(parent2));

		auto generate_gene = [&] ()
		{
			auto splicing = Random::sequence.GetBool() ? * iterators.first : * iterators.second;

			if (Random::sequence.GetBool(mutation_rate))
			{
				auto mutation_weight = Random::sequence.GetFloat<GeneType>();
				auto existing_weight = 1.f - mutation_weight;
				auto mutated_value = splicing * existing_weight + mutation_weight * mutation_weight;
				return mutated_value;
			}

			return splicing;
		};

		_buffer.push_back(generate_gene());
	}
}

Genome::size_type Genome::size() const
{
	return _buffer.size();
}

Genome::const_iterator Genome::begin() const
{
	return std::begin(_buffer);
}

Genome::iterator Genome::begin()
{
	return std::begin(_buffer);
}

Genome::const_iterator Genome::end() const
{
	return std::end(_buffer);
}

Genome::iterator Genome::end() noexcept
{
	return std::end(_buffer);
}

GeneType Genome::operator[] (size_type index) const noexcept
{
	CRAG_VERIFY_OP(index, <, size());
	return _buffer[index];
}

void Genome::Grow()
{
	_buffer.push_back(Random::sequence.GetFloatInclusive<float>());
}

////////////////////////////////////////////////////////////////////////////////
// sim::ga::Genome member definitions

GenomeReader::GenomeReader(Genome & genome)
: _genome(genome)
{
}

GeneType GenomeReader::Read()
{
	if (_position == _genome.size())
	{
		_genome.Grow();
	}

	return _genome[_position ++];
}
