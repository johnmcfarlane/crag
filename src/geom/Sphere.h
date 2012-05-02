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
		static S Circumference(S radius)
		{
			return static_cast<S>(PI * 2.) * radius;
		}
		
		static S Area(S radius)
		{
			return static_cast<S>(PI) * Square(radius);
		}
	};

	template <typename S>
	class SphereProperties<S, 3>
	{
	public:
		static S Area(S radius)
		{
			return static_cast<S>(PI * 4.) * Square(radius);
		}
		
		static S Volume(S radius)
		{
			return static_cast<S>(PI * 4. / 3.) * Cube(radius);
		}
		
		static S RadiusFromVolume(S volume)
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
		typedef Vector<Scalar, N> Vector;
		typedef SphereProperties<Scalar, N> Properties;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		// c'tors
		Sphere() 
#if ! defined(NDEBUG)
		: radius(std::numeric_limits<S>::signaling_NaN())
#endif
		{ 
		}
		
		Sphere(Vector const & c, Scalar r) 
		: center(c)
		, radius(r)
		{ 
		}
		
		template <typename RHS_S>
		Sphere(geom::Vector<RHS_S, N> const & c, RHS_S r) 
		: center(c)
		, radius(r)
		{ 
		}
		
		template <typename RHS_S>
		Sphere(Sphere<RHS_S, N> const & rhs) 
		: center(rhs.center)
		, radius(rhs.radius)
		{ 
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Vector center;
		Scalar radius;
	};


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

	template<typename S, int N> S RadiusFromVolume(Sphere<S, N> const & s)
	{
		return Sphere<S, N>::Properties::RadiusFromVolume(s.radius);
	}
}
