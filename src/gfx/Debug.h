//
//  Debug.h
//  crag
//
//  Created by John on 12/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Color.h"

#include "geom/Matrix33.h"
#include "geom/Matrix44.h"

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
		// typedefs
		typedef float Scalar;
		typedef geom::Vector<Scalar, 3> Vector3;
		typedef geom::Vector<Scalar, 4> Vector4;
		typedef geom::Matrix<Scalar, 3, 3> Matrix33;
		typedef geom::Matrix<Scalar, 4, 4> Matrix44;
		typedef Color4f Color;

		class ColorPair
		{
		public:
			ColorPair(Color const & color, Color const & hidden_color)
			: _color(color)
			, _hidden_color(hidden_color)
			{
			}
			
			ColorPair(Color const & color, float hidden_alpha = .5f)
			: _color(color)
			, _hidden_color(color)
			{
				_hidden_color.a *= hidden_alpha;
			}
			
			Color _color;	// Color to draw pixels that are visible, i.e. pass the z test.
			Color _hidden_color;	// Color for failed pixels.
		};

#if defined(GFX_DEBUG)
		void Init();
		void Deinit();
		void Verify();

		double GetVerbosity();
		
		void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1));
		void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b);
		void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors = ColorPair(1, 1));
		void AddBasis(Vector3 const & center, Vector3 const & scale = Vector3(1, 1, 1), Matrix33 const & rotation = Matrix33::Identity());
		void AddFrustum(Pov const & pov);
		
		void Draw(Vector3 const & camera_pos);
		
		void DrawText(char const * text, geom::Vector2i const & position);
		
#else
		inline void Init() { }
		inline void Deinit() { }
		inline void Verify() { }
	
		inline double GetVerbosity() { return 0; }
	
		inline void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1)) { }
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b) { }
		inline void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors = ColorPair(1, 1)) { }
		inline void AddBasis(Vector3 const & center, Vector3 const & scale = Vector3(1, 1, 1), Matrix33 const & rotation = Matrix33::Identity()) { }
		inline void AddFrustum(Pov const & pov) { }
		inline void Draw(Vector3 const &) { }
		inline void DrawText(char const * text, geom::Vector2i const & position) { }
#endif
	
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors = ColorPair(1, 1))
		{
			AddLine(a, b, colors, colors);
		}
		inline void AddBasis(Vector3 const & center, Scalar scale, Matrix33 const & rotation = Matrix33::Identity()) 
		{ 
			AddBasis(center, Vector3(scale, scale, scale), rotation);
		}
	}
}
