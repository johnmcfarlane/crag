/*
 *  Color.h
 *  Crag
 *
 *  Created by john on 5/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/floatOps.h"


namespace gfx
{
	template <typename DstComp, typename SrcComp> DstComp ConvertColorComponent(SrcComp lhs);

	template <typename Comp> class Color4
	{
	public:
		Color4() : r(0), g(0), b(0), a(Full()) { }
		Color4(Color4 const & that) : r(that.r), g(that.g), b(that.b), a(that.a) { }
		Color4(Comp red, Comp green, Comp blue, Comp alpha = Full()) : r(red), g(green), b(blue), a(alpha) { }
		Color4(Comp intensity, Comp alpha = Full()) : r(intensity), g(intensity), b(intensity), a(alpha) { }

		// conversion to alternative format
		template <typename ThatComp> Color4(Color4<ThatComp> const & that)
			: r(ConvertColorComponent<Comp>(that.r))
			, g(ConvertColorComponent<Comp>(that.g))
			, b(ConvertColorComponent<Comp>(that.b))
			, a(ConvertColorComponent<Comp>(that.a))
		{

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
			Assert(i >= 0 && i < 4);
			return GetArray() [i];
		}

		Comp operator [] (int i) const
		{
			Assert(i >= 0 && i < 4);
			return GetArray() [i];
		}

		operator Comp const * () const
		{
			return reinterpret_cast<Comp const *>(this);
		}
		
		friend bool operator==(Color4 const & lhs, Color4 const & rhs)
		{
			return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
		}
		
		friend bool operator!=(Color4 const & lhs, Color4 const & rhs)
		{
			return lhs.r != rhs.r || lhs.g != rhs.g || lhs.b != rhs.b || lhs.a != rhs.a;
		}
		
		Color4 & operator += (Color4 const & rhs)
		{
			r += rhs.r;
			g += rhs.g;
			b += rhs.b;
			a += rhs.a;
			return * this;
		}

		friend Color4 operator * (Color4 const & lhs, Comp rhs)
		{
			return Color4(lhs.r * rhs,
						  lhs.g * rhs,
						  lhs.b * rhs,
						  lhs.a * rhs);
		}
		
		friend Color4 operator * (Color4 const & lhs, Color4 const & rhs)
		{
			return Color4(lhs.r * rhs.r,
						  lhs.g * rhs.g,
						  lhs.b * rhs.b,
						  lhs.a * rhs.a);
		}
		
		friend Color4 operator / (Color4 const & lhs, Comp rhs)
		{
			return lhs * (1.f / rhs);
		}

		Comp Min() const
		{
			return ::Min(::Min(r, g), ::Min(b, a));
		}

		Comp Max() const
		{
			return ::Max(::Max(r, g), ::Max(b, a));
		}

	private:
	
		static unsigned char FloatToUchar(Comp f)
		{
			return static_cast<unsigned char>(Clamp(f * 255.f, 0.f, 255.f));
		}

	public:

		static Comp Half();
		static Comp Full();

		static Color4 Black()   { return Color4(0); }
		static Color4 White()   { return Color4(Full()); }
		static Color4 Grey()    { return Color4(Half); }

		static Color4 Red()     { return Color4(Full(), 0, 0); }
		static Color4 Green()   { return Color4(0, Full(), 0); }
		static Color4 Blue()    { return Color4(0, 0, Full()); }

		static Color4 Cyan()    { return Color4(0, Full(), Full()); }
		static Color4 Magenta() { return Color4(Full(), 0, Full()); }
		static Color4 Yellow()  { return Color4(Full(), Full(), 0); }
	
		// Attributes.
		Comp r, g, b, a;
	};
	

	///////////////////////////////////////////////////////////////
	// Color4f

	typedef Color4<float> Color4f;
	
	template<> inline float Color4<float>::Half() {
		return .5f;
	}
	
	template<> inline float Color4<float>::Full() {
		return 1.f;
	}
	

	///////////////////////////////////////////////////////////////
	// Color4b

	typedef Color4<unsigned char> Color4b;

	template<> inline unsigned char Color4<unsigned char>::Half() {
		return 128;
	}
	
	template<> inline unsigned char Color4<unsigned char>::Full() {
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
