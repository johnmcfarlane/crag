//
//  entity/sim/Genome.h
//  crag
//
//  Created by John McFarlane on 2013-03-27.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace sim
{
	namespace ga
	{
		using GeneType = float;

		////////////////////////////////////////////////////////////////////////////////
		// the set of Genes associated with an Animat
		class Genome
		{
			// types
			using buffer_type = std::vector<GeneType>;
		public:
			using size_type = buffer_type::size_type;
			using difference_type = buffer_type::difference_type;
			using iterator = buffer_type::iterator;
			using const_iterator = buffer_type::const_iterator;

			// functions
			Genome() noexcept;
			Genome(Genome const & parent1, Genome const & parent2) noexcept;

			bool empty() const noexcept;
			size_type size() const noexcept;

			const_iterator begin() const noexcept;
			iterator begin() noexcept;

			const_iterator end() const noexcept;
			iterator end() noexcept;

			GeneType operator[] (size_type index) const noexcept;

			void Grow() noexcept;

		private:
			// variables
			buffer_type _buffer;
		};

		////////////////////////////////////////////////////////////////////////////////
		// reads Genes from a Genome
		class GenomeReader
		{
			OBJECT_NO_COPY(GenomeReader);

		public:
			GenomeReader(Genome & genome) noexcept;

			GeneType Read() noexcept;
		private:
			Genome & _genome;
			Genome::size_type _position = 0;
		};
	}
}
