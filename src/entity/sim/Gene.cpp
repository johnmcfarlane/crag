//
// Created by john on 7/4/15.
//

#include "pch.h"

#include "Gene.h"

#include <core/Random.h>

using namespace sim::ga;

namespace
{
	template <typename unit_type>
	unit_type inherit(unit_type parent_value1, unit_type parent_value2, Random & r, float mutation_rate) noexcept
	{
		auto splicing = r.GetBool() ? parent_value1 : parent_value2;

		// if a mutation occurs
		if (r.GetBool(mutation_rate))
		{
			auto mutation_weight = r.GetFloat<double>();
			auto mutation_weight_fixed = Gene::unit_type(mutation_weight);

			auto random_value = unit_type(r.GetFloat<double>());
			auto mutated_value = crag::lerp(splicing, random_value, mutation_weight_fixed);
			return mutated_value;
		}

		return splicing;
	};

	template <typename VALUE_TYPE, typename REPR_TYPE>
	VALUE_TYPE make_open(crag::core::unit_interval<REPR_TYPE> _value) noexcept
	{
		using repr_type = REPR_TYPE;
		constexpr auto next_digits = cnl::digits<repr_type>::value * 2;
		using next_repr_type = cnl::set_digits_t<repr_type, next_digits>;

		// unit_type is higher precision than float;
		// add/subtract these margins from 0/1 to ensure (0.f, 1.f) range
		constexpr repr_type lower_float_epsilon = 1;
		constexpr repr_type upper_float_epsilon = 257;

		constexpr repr_type repr_max = std::numeric_limits<repr_type>::max();
		constexpr next_repr_type denom = static_cast<next_repr_type>(repr_max) + lower_float_epsilon + upper_float_epsilon;

		auto repr = _value.data();
		auto repr_next = static_cast<next_repr_type>(repr) + lower_float_epsilon;
		auto excl = static_cast<Gene::value_type>(repr_next) / denom;

		CRAG_VERIFY_OP(excl, >, static_cast<Gene::value_type>(0));
		CRAG_VERIFY_OP(excl, <, static_cast<Gene::value_type>(1));

		return excl;
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::ga::Gene member definitions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Gene, self)
	CRAG_VERIFY_OP(static_cast<float>(self._value), >=, 0.f);
	CRAG_VERIFY_OP(static_cast<float>(self._value), <=, 1.f);
CRAG_VERIFY_INVARIANTS_DEFINE_END

Gene::Gene(Random & r) noexcept
	: _value(r.GetFloat<double>())
{
	CRAG_VERIFY(* this);
}

Gene::Gene(Gene parent1, Gene parent2, Random & r, float mutation_rate) noexcept
	: _value(inherit(parent1._value, parent2._value, r, mutation_rate))
{
	CRAG_VERIFY(* this);
}

Gene::value_type Gene::closed() const noexcept
{
	CRAG_VERIFY(* this);
	return static_cast<float>(_value);
}

Gene::value_type Gene::open() const noexcept
{
	CRAG_VERIFY(* this);

	return make_open<value_type, unit_repr_type>(_value);
}

Gene Gene::from_repr(unit_repr_type repr) noexcept
{
	Gene gene;
	gene._value = unit_type::from_data(repr);
	return gene;
}

Gene::unit_repr_type Gene::to_repr() const noexcept
{
	CRAG_VERIFY(* this);

	return _value.data();
}
