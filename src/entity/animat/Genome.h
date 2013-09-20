//
//  scripts/ga/Genome.h
//  crag
//
//  Created by John McFarlane on 2013-03-27.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

namespace sim
{
	namespace ga
	{
		typedef float Gene;
		typedef std::vector<Gene> GeneVector;
		typedef GeneVector::iterator GeneIterator;
		typedef GeneVector::const_iterator ConstGeneIterator;

		////////////////////////////////////////////////////////////////////////////////
		// the set of Genes associated with an Animat
		class Genome
		{
		public:
			typedef GeneVector::size_type size_type;
			typedef GeneVector::difference_type difference_type;

			size_type Size() const;

			ConstGeneIterator begin() const;
			GeneIterator begin();

			ConstGeneIterator end() const;
			GeneIterator end();

			void Grow();
		private:
			GeneVector _genes;
		};

		////////////////////////////////////////////////////////////////////////////////
		// reads Genes from a Genome
		class GenomeReader
		{
		public:
			GenomeReader(Genome & genome);

			Gene Read();
		private:
			Genome & _genome;
			ConstGeneIterator _position;
		};
	}
}
