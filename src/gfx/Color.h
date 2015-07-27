//
//  Color.h
//  crag
//
//  Created by john on 5/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/floatOps.h"


namespace gfx
{
	template <typename DstComp, typename SrcComp> DstComp ConvertColorComponent(SrcComp lhs);

	// TODO: consolidate with Vector4
	template <typename Comp> class Color4
	{
	public:
		typedef Comp Scalar;
		
		constexpr Color4() noexcept
			: r(0), g(0), b(0), a(Full())
		{ }

		constexpr Color4(Color4 const & that) noexcept
			: r(that.r), g(that.g), b(that.b), a(that.a)
		{ }

		constexpr Color4(Comp red, Comp green, Comp blue, Comp alpha = Full()) noexcept
			: r(red), g(green), b(blue), a(alpha)
		{ }

		constexpr Color4(Comp intensity, Comp alpha = Full()) noexcept
			: r(intensity), g(intensity), b(intensity), a(alpha)
		{ }

		// conversion to alternative format
		template <typename ThatComp> Color4(Color4<ThatComp> const & that)
			: r(ConvertColorComponent<Comp>(that.r))
			, g(ConvertColorComponent<Comp>(that.g))
			, b(ConvertColorComponent<Comp>(that.b))
			, a(ConvertColorComponent<Comp>(that.a))
		{

		}
		
		static constexpr std::size_t Size()
		{
			return 4;
		}
		
		Comp * GetArray()
		{
			return reinterpret_cast<Comp *>(this);
		}
		
		Comp const * GetArray() const
		{
			return reinterpret_cast<Comp const *>(this);
		}

		Comp & operator [] (int i)
		{
			ASSERT(i >= 0 && i < 4);
			return GetArray() [i];
		}

		Comp const & operator [] (int i) const
		{
			ASSERT(i >= 0 && i < 4);
			return GetArray() [i];
		}

		operator Comp const * () const
		{
			return reinterpret_cast<Comp const *>(this);
		}

		friend constexpr bool operator==(Color4 const & lhs, Color4 const & rhs) noexcept
		{
			return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
		}

		friend constexpr bool operator!=(Color4 const & lhs, Color4 const & rhs) noexcept
		{
			return lhs.r != rhs.r || lhs.g != rhs.g || lhs.b != rhs.b || lhs.a != rhs.a;
		}
		
		Color4 & operator += (Color4 const & rhs) noexcept
		{
			r += rhs.r;
			g += rhs.g;
			b += rhs.b;
			a += rhs.a;
			return * this;
		}

		friend constexpr Color4 operator + (Color4 const & lhs, Color4 const & rhs) noexcept
		{
			return Color4(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a);
		}

		friend constexpr Color4 operator * (Color4 const & lhs, Comp rhs) noexcept
		{
			return Color4(lhs.r * rhs,
						  lhs.g * rhs,
						  lhs.b * rhs,
						  lhs.a);
		}

		friend constexpr Color4 operator * (Color4 const & lhs, Color4 const & rhs) noexcept
		{
			return Color4(lhs.r * rhs.r,
						  lhs.g * rhs.g,
						  lhs.b * rhs.b,
						  lhs.a * rhs.a);
		}

		friend constexpr Color4 operator / (Color4 const & lhs, Comp rhs) noexcept
		{
			return lhs * (1.f / rhs);
		}

		Comp Min() const
		{
			return min(min(r, g), min(b, a));
		}

		Comp Max() const
		{
			return max(max(r, g), max(b, a));
		}

	private:
	
		static unsigned char FloatToUchar(Comp f)
		{
			return static_cast<unsigned char>(Clamp(f * 255.f, 0.f, 255.f));
		}

	public:

		static constexpr Comp Half() noexcept;
		static constexpr Comp Full() noexcept;

		static Color4 Black() 
		{
			return Color4(0);
		}
		static Color4 White() 
		{
			return Color4(Full());
		}
		static Color4 Grey()	
		{
			return Color4(Half());
		}

		static Color4 Red()		
		{
			return Color4(Full(), 0, 0);
		}
		static Color4 Green() 
		{
			return Color4(0, Full(), 0);
		}
		static Color4 Blue()	
		{
			return Color4(0, 0, Full());
		}

		static Color4 Cyan()	
		{
			return Color4(0, Full(), Full());
		}
		static Color4 Magenta() 
		{
			return Color4(Full(), 0, Full());
		}
		static Color4 Yellow()
		{
			return Color4(Full(), Full(), 0);
		}
		
		static Color4 Periwinkle()
		{
			return Color4(Comp(.8), Comp(.8), Full());
		}
		static Color4 Orange()
		{
			return Color4(Full(), Half(), 0);
		}
	
		// Attributes.
		Comp r, g, b, a;
	};
	
	template <typename C> 
	std::ostream & operator << (std::ostream & out, Color4<C> const & color)
	{
		return out << color.r << ',' << color.g << ',' << color.b << ',' << color.a;
	}

	template <typename C> 
	std::istream & operator >> (std::istream & in, Color4<C> const & color)
	{
		return in >> color.r >> ',' >> color.g >> ',' >> color.b >> ',' >> color.a;
	}

	///////////////////////////////////////////////////////////////
	// Color4f

	typedef Color4<float> Color4f;
	
	template<>
	inline constexpr float Color4<float>::Half() noexcept
	{
		return .5f;
	}
	
	template<>
	inline constexpr float Color4<float>::Full() noexcept
	{
		return 1.f;
	}
	

	///////////////////////////////////////////////////////////////
	// Color4b

	typedef Color4<unsigned char> Color4b;

	template<>
	inline constexpr unsigned char Color4<unsigned char>::Half() noexcept
	{
		return 128;
	}
	
	template<>
	inline constexpr unsigned char Color4<unsigned char>::Full() noexcept
	{
		return 255;
	}


	///////////////////////////////////////////////////////////////
	// Conversion

	template <> inline float ConvertColorComponent<float, unsigned char>(unsigned char lhs)
	{
		return static_cast<float>(lhs) / (255.999f);
	}

	template <> inline unsigned char ConvertColorComponent<unsigned char, float>(float lhs)
	{
		return static_cast<unsigned char>(lhs * 255.999f);
	}
}
