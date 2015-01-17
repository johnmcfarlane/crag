//
//  Sphere.h
//  crag
//
//  Created by John on 10/31/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// SphereProperties class definition/specializations

	template <typename S, int N>
	class SphereProperties
	{
	};

	template <typename S>
	class SphereProperties<S, 2>
	{
	public:
		static S constexpr Circumference(S radius)
		{
			return static_cast<S>(PI * 2.) * radius;
		}
		
		static S constexpr Area(S radius)
		{
			return static_cast<S>(PI) * Squared(radius);
		}
	};

	template <typename S>
	class SphereProperties<S, 3>
	{
	public:
		static S constexpr Area(S radius)
		{
			return static_cast<S>(PI * 4.) * Squared(radius);
		}
		
		static S constexpr Volume(S radius)
		{
			return static_cast<S>(PI * 4. / 3.) * Cubed(radius);
		}
		
		static S constexpr RadiusFromVolume(S volume)
		{
			return CubeRoot(volume / static_cast<S>(PI * 4. / 3.));
		}
	};


	////////////////////////////////////////////////////////////////////////////////
	// Sphere class definition

	// Base class for circles and spheres, i.e. n-spheres.
	template <typename S, int N> 
	class Sphere
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef S Scalar;
		typedef ::geom::Vector<Scalar, N> Vector;
		typedef SphereProperties<Scalar, N> Properties;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		// c'tors
		Sphere() 
#if defined(CRAG_DEBUG)
		: radius(std::numeric_limits<S>::signaling_NaN())
#endif
		{ 
		}
		
		Sphere(Vector const & c, Scalar r) 
		: center(c)
		, radius(r)
		{ 
		}
		
		Sphere(Sphere const & rhs) 
		: center(rhs.center)
		, radius(rhs.radius)
		{ 
		}

		static Sphere Zero() 
		{
			return Sphere(Vector::Zero(), 0); 
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Sphere, s)
			CRAG_VERIFY(s.center);
			CRAG_VERIFY(s.radius);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Vector center;
		Scalar radius;
	};

	// catches pointless casts and bypasses them
	template <typename S, int N>
	Sphere<S, N> const & Cast(Sphere<S, N> const & rhs)
	{
		return rhs;
	}

	// casts between spheres of different scalar types
	template <typename LHS_S, typename RHS_S, int N>
	typename std::enable_if<! std::is_same<LHS_S, RHS_S>::value, Sphere<LHS_S, N>>::type
	Cast(Sphere<RHS_S, N> const & rhs)
	{
		return Sphere<LHS_S, N>(Cast<LHS_S>(rhs.center), static_cast<LHS_S>(rhs.radius));
	}

	// equality operator
	template <typename S, int N>
	bool operator==(Sphere<S, N> const & lhs, Sphere<S, N> const & rhs)
	{
		return lhs.center == rhs.center && lhs.radius == rhs.radius;
	}

	// inequality operator
	template <typename S, int N>
	bool operator!=(Sphere<S, N> const & lhs, Sphere<S, N> const & rhs)
	{
		return lhs.center != rhs.center || lhs.radius != rhs.radius;
	}

	// streaming
	template <typename S, int N>
	std::ostream & operator << (std::ostream & out, Sphere<S, N> const & sphere)
	{
		return out << sphere.center << '+' << sphere.radius;
	}

	template <typename S, int N>
	std::istream & operator >> (std::istream & in, Sphere<S, N> const & sphere)
	{
		return in >> sphere.center >> '+' >> sphere.radius;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Sphere properties

	template<typename S, int N> S Diameter(Sphere<S, N> const & s)
	{
		return s.radius * 2.;
	}

	template<typename S, int N> S Area(Sphere<S, N> const & s)
	{
		return Sphere<S, N>::Properties::Area(s.radius);
	}

	template<typename S, int N> S Volume(Sphere<S, N> const & s)
	{
		return Sphere<S, N>::Properties::Volume(s.radius);
	}
}
