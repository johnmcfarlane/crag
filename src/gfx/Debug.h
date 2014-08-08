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
#include "defs.h"

#include "geom/Matrix33.h"
#include "geom/Matrix44.h"

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

#if defined(CRAG_GFX_DEBUG)
		void Init(ResourceManager const & resource_manager);
		void Deinit();
		void Verify();

		double GetVerbosity();
		
		void AddPoint(Vector3 const & a, ColorPair const & colors = ColorPair(1, 1));
		void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b);
		void AddTriangle(Triangle3 const & triangle, ColorPair const & colors = ColorPair(1, 1));
		void AddBasis(Transformation const & transformation, Vector3 const & scale = Vector3(1, 1, 1));
		void AddFrustum(Pov const & pov);
		
		void Draw(Matrix44 const & model_view_matrix, Matrix44 const & projection_matrix);
		void DrawText(char const * text, geom::Vector2i const & position);
		void Clear();
#else
		inline void Init(ResourceManager const &) { }
		inline void Deinit() { }
		inline void Verify() { }
	
		inline double GetVerbosity() { return 0; }
	
		inline void AddPoint(Vector3 const &, ColorPair const & = ColorPair(1, 1)) { }
		inline void AddLine(Vector3 const &, Vector3 const &, ColorPair const &, ColorPair const &) { }
		inline void AddTriangle(Triangle3 const &, ColorPair const & = ColorPair(1, 1)) { }
		inline void AddBasis(Transformation const &, Vector3 const & = Vector3(1, 1, 1)) { }
		inline void AddFrustum(Pov const &) { }

		inline void Draw(Transformation const &, Transformation const &) { }
		inline void DrawText(char const *, geom::Vector2i const &) { }
		inline void Clear() { }
#endif
	
		inline void AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors = ColorPair(1, 1))
		{
			AddLine(a, b, colors, colors);
		}
		inline void AddLine(Ray3 const & ray, ColorPair const & colors = ColorPair(1, 1))
		{
			AddLine(ray.position, ray.position + ray.direction, colors, colors);
		}
		inline void AddBasis(Transformation const & transformation, Scalar scale) 
		{ 
			AddBasis(transformation, Vector3(scale, scale, scale));
		}
	}
}
