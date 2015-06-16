//
//  core/Sigmoid.h
//  crag
//
//  Created by John McFarlane on 2015-06-08.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include <cmath>

namespace crag
{
	namespace core
	{
		// represents the equation, y = 1 / (1 + exp(a * x + b)), for any given a and b
		template <typename T>
		class Sigmoid
		{
		public:
			// types
			using value_type = T;

			// functions
			constexpr Sigmoid(value_type _a, value_type _b) noexcept
				: a(_a), b(_b)
			{
			}

			// given two samples from the curve, find a and b
			/*CRAG_CONSTEXPR*/ Sigmoid(double x1, double y1, double x2, double y2) noexcept
			{
				double l1 = std::log(1. / y1 - 1.);
				double l2 = std::log(1. / y2 - 1.);
				double xr = x1 / x2;
				a = value_type((l1 - l2) / (x1 - x2));
				b = value_type((l1 - l2 * xr) / (1. - xr));

				CRAG_VERIFY_NEARLY_EQUAL((*this)(x1), value_type(y1), value_type(.0001));
				CRAG_VERIFY_NEARLY_EQUAL((*this)(x2), value_type(y2), value_type(.0001));
			}

			constexpr value_type operator() (value_type x) const noexcept
			{
				return value_type(1) / (value_type(1) + std::exp(x * a + b));
			}

		private:
			// variables
			value_type a = 0;    // gradient (gradient)
			value_type b = 0;    // constant (- threshold)
		};
	}
}
