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

#include "core/Random.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::ga::Genome member definitions

using namespace sim::ga;

Genome::size_type Genome::Size() const
{
	return _genes.size();
}

ConstGeneIterator Genome::begin() const
{
	return std::begin(_genes);
}

GeneIterator Genome::begin()
{
	return std::begin(_genes);
}

ConstGeneIterator Genome::end() const
{
	return std::end(_genes);
}

GeneIterator Genome::end()
{
	return std::end(_genes);
}

void Genome::Grow()
{
	ASSERT(_genes.size() < 1000);
	_genes.push_back(Random::sequence.GetUnit<float>());
}

////////////////////////////////////////////////////////////////////////////////
// sim::ga::Genome member definitions

GenomeReader::GenomeReader(Genome & genome)
: _genome(genome)
, _position(std::begin(genome))
{
}

Gene GenomeReader::Read()
{
	if (_position == std::end(_genome))
	{
		Genome::size_type index = _position - std::begin(_genome);
		ASSERT(index < _genome.Size());

		_genome.Grow();
		_position = std::begin(_genome) + index;
		ASSERT(_position < std::end(_genome));
	}

	return * _position;
}
