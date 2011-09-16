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

#include "geom/Matrix4.h"
#include "geom/Vector4.h"

#if defined(PROFILE) || ! defined(NDEBUG)
#define GFX_DEBUG
#endif


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
		typedef ::Vector<double, 3> Vector3;
		typedef ::Vector<double, 4> Vector4;
		typedef ::Matrix4<double> Matrix4;

		class ColorPair
		{
		public:
			ColorPair(Color4f const & color, Color4f const & hidden_color)
			: _color(color)
			, _hidden_color(hidden_color)
			{
			}
			
			ColorPair(Color4f const & color, float hidden_alpha = .5f)
			: _color(color)
			, _hidden_color(color)
			{
				_hidden_color.a *= hidden_alpha;
			}
			
			Color4f _color;	// Color to draw pixels that are visible, i.e. pass the z test.
			Color4f _hidden_color;	// Color for failed pixels.
		};

#if defined(GFX_DEBUG)
		void Init();
		void Deinit();
		void Verify();

		double GetVerbosity();
		
		void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1));
		void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b);
		void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors = ColorPair(1, 1));
		void AddBasis(Vector3 const & center, double scale);
		void AddFrustum(Pov const & pov);
		
		void Draw(Vector3 const & camera_pos);
		
		void DrawText(char const * text, Vector2i const & position);
		
#else
		inline void Init() { }
		inline void Deinit() { }
		inline void Verify() { }
	
		inline double GetVerbosity() { return 0; }
	
		inline void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1)) { }
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b) { }
		inline void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors = ColorPair(1, 1)) { }
		inline void AddBasis(Vector3 const & center, double scale) { }
		inline void AddFrustum(Pov const & pov) { }
		inline void Draw(Vector3 const &) { }
		inline void DrawText(char const * text, Vector2i const & position) { }
#endif
	
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors = ColorPair(1, 1))
		{
			AddLine(a, b, colors, colors);
		}
	}
}
