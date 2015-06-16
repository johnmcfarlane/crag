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
