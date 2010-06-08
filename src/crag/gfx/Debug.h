/*
 *  Debug.h
 *  Crag
 *
 *  Created by John on 12/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Color.h"

#include <ostream>

#if defined(PROFILE) || ! defined(NDEBUG)
#define GFX_DEBUG
#endif


template <typename S> class Vector3;


namespace sim
{
	struct AppInfo;
}


namespace gfx
{
	class Pov;

	namespace Debug
	{
		// forward declarations
		typedef ::Vector3<double> Vector3;

		class ColorPair
		{
		public:
			ColorPair(Color4f const & init_color, Color4f const & init_hidden_color = Color4f(0, 0))
			: color(init_color)
			, hidden_color(init_hidden_color)
			{
			}

			Color4f color;	// Color to draw pixels that are visible, i.e. pass the z test.
			Color4f hidden_color;	// Color for failed pixels.
		};

		// Warning: points to nullptr unless GFX_DEBUG is defined.
		extern std::ostream & out;

	#if defined(GFX_DEBUG)
		void Init();
		void Deinit();

		double GetVerbosity();
		
		void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1));
		void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b);
		void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors = ColorPair(1, 1));
		void AddFrustum(Pov const & pov);
		void DrawGraphics();
		void DrawText();
	#else
		inline void Init() { }
		inline void Deinit() { }
	
		inline double GetVerbosity() { return 0; }
	
		inline void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1)) { }
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b) { }
		inline void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors = ColorPair(1, 1)) { }
		inline void AddFrustum(Pov const & pov) { }
		inline void DrawGraphics() { }
		inline void DrawText() { }
	#endif
	
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors = ColorPair(1, 1))
		{
			AddLine(a, b, colors, colors);
		}
	}
}
