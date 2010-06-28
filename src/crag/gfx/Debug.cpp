/*
 *  Debug.cpp
 *  Crag
 *
 *  Created by John on 12/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Debug.h"

#include "core/ConfigEntry.h"

#include <sstream>

#if defined(GFX_DEBUG)

#include "Color.h"
#include "Font.h"
#include "Pov.h"

#include "glpp/glpp.h"

#include "geom/Vector3.h"

#include <vector>


namespace ANONYMOUS {

using namespace gfx;

class PointArray
{
public:
	PointArray(int init_mode) : mode(init_mode) { }
	
	void Clear()
	{
		points.clear();
	}
	
	void AddPoint(Debug::Vector3 const & pos, Debug::ColorPair const & colors)
	{
		points.push_back(Point(pos, Debug::ColorPair(colors)));
	}
	
	void Draw(bool hidden) const
	{
		gl::Begin(mode);
		for (point_vector::const_iterator it = points.begin(); it != points.end(); ++ it)
		{
			Point const & point = * it;
			point.Draw(hidden);
		}
		gl::End();
	}
	
private:
	
	class Point
	{
	public:
		Point(Vector3f init_pos, Debug::ColorPair const & init_colors)
		: pos(init_pos)
		, colors(init_colors)
		{
		}
		
		void Draw(bool hidden) const
		{
			Color4f const & color = hidden ? colors.hidden_color : colors.color;
			gl::SetColor(color.r, color.g, color.b, color.a);
			gl::Vertex3(pos.x, pos.y, pos.z);
		}
		
		Debug::Vector3 pos;
		Debug::ColorPair colors;
	};
	
	typedef std::vector<Point> point_vector;
	
	point_vector points;
	GLenum mode;
};

PointArray points(GL_POINTS);
PointArray lines(GL_LINES);
PointArray tris(GL_TRIANGLES);

void DrawPrimatives(bool hidden)
{
	if (hidden) {
		gl::SetDepthFunc(GL_GREATER);
	}

	points.Draw(hidden);
	lines.Draw(hidden);
	tris.Draw(hidden);

	if (hidden) {
		gl::SetDepthFunc(GL_LEQUAL);
	}
}

void ClearPrimatives()
{
	points.Clear();
	lines.Clear();
	tris.Clear();
}

std::stringstream out_stream;

gfx::Font * font = nullptr;

CONFIG_DEFINE(debug_verbosity, double, .5);

}


// Can't find this at link time? Make sure you're wrapping reference in a test of GetVerbosity(). 
std::ostream & Debug::out = out_stream;

void Debug::Init()
{
	Assert(font == nullptr);

	// TODO: Is this legit content?
	// http://www.amanith.org/testsuite/amanithvg_gle/data/font_bitmap.png
	// Maybe this one's ok...
	// http://www.ogre3d.org/wiki/index.php/Outlined_Fonts
	font = new Font("font.png", .5f);
}

void Debug::Deinit()
{
	Assert(font != nullptr);
	delete font;
	font = nullptr;
}

double Debug::GetVerbosity() 
{ 
#if defined(NDEBUG)
	return .5;
#else
	return debug_verbosity;
#endif
}

void gfx::Debug::AddPoint(Vector3 const & a, ColorPair const & colors)
{
	points.AddPoint(a, colors);
}

void gfx::Debug::AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b)
{
	lines.AddPoint(a, colors_a);
	lines.AddPoint(b, colors_b);
}

void gfx::Debug::AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors)
{
	tris.AddPoint(a, colors);
	tris.AddPoint(b, colors);
	tris.AddPoint(c, colors);
}

void gfx::Debug::AddFrustum(Pov const & pov)
{
	Vector3f corners[2][2][2];
	Vector3f eye;
	
	{
		Vector3f extent_factors[2];
		
		{
			float aspect = static_cast<float>(pov.frustrum.resolution.x) / pov.frustrum.resolution.y;
			float y_factor = static_cast<float>(Sin(pov.frustrum.fov * .5));
			float x_factor = y_factor * aspect;
			
			extent_factors[0] = Vector3f(- x_factor, - y_factor, static_cast<float>(pov.frustrum.near_z));
			extent_factors[1] = Vector3f(x_factor, y_factor, static_cast<float>(pov.frustrum.far_z));
		}
		
		{
			sim::Matrix4 m = pov.GetCameraMatrix();
			
			for (int z_index = 0; z_index < 2; ++ z_index) {
				for (int y_index = 0; y_index < 2; ++ y_index) {
					for (int x_index = 0; x_index < 2; ++ x_index) {
						Vector4f p (extent_factors[x_index].x * extent_factors[z_index].z, 
									extent_factors[z_index].z,
									extent_factors[y_index].y * extent_factors[z_index].z, 
									1.f);
						//p.z -= pov.frustrum.near_z;
						corners[z_index][y_index][x_index] = static_cast< sim::Vector3 >(m * sim::Vector4(p));
					}
				}
			}
			
			eye = static_cast< sim::Vector3 >(m * sim::Vector4(0, 0, 0, 1));
		}
	}
	
	// view box
	for (int axis = 0; axis < 3; ++ axis) {
		int perp_axis_1 = TriMod(axis + 1);
		int perp_axis_2 = TriMod(axis + 2);
		Vector3i indices;
		
		for (int p = 0; p < 2; ++ p) {
			indices[perp_axis_1] = p;
			for (int q = 0; q < 2; ++ q) {
				indices[perp_axis_2] = q;
				
				Vector3f const * a, * b;
				
				if (axis == 0) {
					a = & eye;
				}
				else {
					indices[axis] = 0;
					a = & corners[indices[0]][indices[1]][indices[2]];
				}
				
				indices[axis] = 1;
				b = & corners[indices[0]][indices[1]][indices[2]];
				
				Color4f visible = Color4f::Black();
				visible[axis] = 1;
				
				Color4f invisible = visible;
				invisible.a = .2f;
				
				gfx::Debug::AddLine(* a, * b, gfx::Debug::ColorPair(visible, invisible));
			}
		}
	}
	
	// pyramid
	/*Vector3f center = static_cast< Vector3<float> >(Vector4f(0, 0, - pov.frustrum.near_z, 0) * m);
	 for (int p = 0; p < 2; ++ p) {
	 for (int q = 0; q < 2; ++ q) {
	 Vector3 const & a = corners[0][q][p];
	 Vector3 const & b = center;
	 
	 gfx::Debug::AddLine(a, b, gfx::Debug::ColorPair(Color(1.f, 1.f), Color(.25f, .5f)));
	 }
	 }*/
}

void gfx::Debug::DrawGraphics()
{
	GLPP_VERIFY;

	// Set state
	gl::Disable(GL_CULL_FACE);
	Assert(! gl::IsEnabled(GL_LIGHT0));
	Assert(! gl::IsEnabled(GL_LIGHTING));
	gl::Enable(GL_DEPTH_TEST);
	
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl::Enable(GL_BLEND);
	
	DrawPrimatives(false);
	DrawPrimatives(true);
	ClearPrimatives();
	
	// Unset state
	gl::Enable(GL_CULL_FACE);
	gl::Disable(GL_DEPTH_TEST);
	gl::Disable(GL_BLEND);

	DrawText();
	
	GLPP_VERIFY;
}

void gfx::Debug::DrawText()
{
	if (! (* font))
	{
		return;
	}
	
	char test[1024];
	strcpy(test, out_stream.str().c_str());

	Vector2i p = Vector2i::Zero();
	font->Print(test, p);

	out_stream.str("");
}

#else

class null_stream : public std::ostream
{
public:
	null_stream()
#if defined(WIN32)
		: std::ostream(std::_Noinit)
#endif
	{
	}
};

null_stream out_stream;

// Can't find this at link time? Make sure you're wrapping reference in a test of GetVerbosity().
std::ostream & gfx::Debug::out = out_stream;

#endif
