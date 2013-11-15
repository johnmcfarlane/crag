//
//  form/QuaternaBuffer.h
//  crag
//
//  Created by John on 3013-11-05.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/for_each.h"

namespace form
{
	// forward-declarations
	struct Quaterna;
	
	// a variable sized array of Quaterna with a fixed capacity; 
	// used by Surrounding to store, group, sort and churn through sibling nodes;
	// TODO: rename Quaterna to Quadruple and Quaterna::nodes to Quadruple::quadruplets;
	// TODO: consider moving NodeBuffer from Surrounding to QuaternaBuffer
	class QuaternaBuffer
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		QuaternaBuffer(int max_num_quaterne);
		~QuaternaBuffer();
		
#if defined(VERIFY)
		void Verify() const;
#endif

		// high-level actions
		void Clear();
		void UpdateScores();
		void Sort();

		// churn-related methods
		float GetLowestSortedScore() const;
		Quaterna * GetLowestSorted() const;
		void DecrementSorted();

		// growth
		Quaterna & Grow();
		void Shrink();

		// access/storage
		bool empty() const;
		int size() const;
		int capacity() const;

		Quaterna & operator[] (int index);
		Quaterna const & operator[] (int index) const;

		Quaterna & front();
		Quaterna const & front() const;

		Quaterna & back();
		Quaterna const & back() const;

		Quaterna * begin();
		Quaterna const * begin() const;

		Quaterna * end();
		Quaterna const * end() const;

		template <typename FUNCTOR> 
		void ForEach(FUNCTOR f, size_t step_size = 1, bool parallel = false)
		{
			if (empty())
			{
				return;
			}

			if (! parallel && step_size == 1)
			{
				core::for_each<Quaterna *, FUNCTOR>(_quaterne, _quaterne_used_end, f);
			}
			else
			{
				smp::for_each<Quaterna *, FUNCTOR>(_quaterne, _quaterne_used_end, f, step_size, -1);
			}
		}
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Quaterna * const _quaterne;		// [max_num_quaterne]

		Quaterna * _quaterne_sorted_end;			// end of the range the we know is sorted
		Quaterna * _quaterne_used_end;			// end of buffer of actually used quaterna
		Quaterna const * const _quaterne_end;
	};
}
