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

#if defined(GFX_DEBUG)

#include "Color.h"
#include "Font.h"
#include "Pov.h"

#include "sim/axes.h"

#include "glpp/glpp.h"

#include "geom/MatrixOps.h"

#include "smp/Mutex.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// Local definitions

namespace
{
	using namespace Debug;

	// Messages of below this verbosity value should get printed.
	CONFIG_DEFINE(debug_verbosity, double, .5);
		
	// Mutex for global functions.
	smp::Mutex mutex;
	
	// The font used to print the string to screen.
	Font * font = nullptr;
	

	// Stores a point of debug geometry. 
	// It can be a dot or the point of a line or a triangle. 
	// It doesn't matter. 
	
	class Point
	{
	public:
		Point(Vector3 init_pos, ColorPair const & init_colors)
		: pos(init_pos)
		, colors(init_colors)
		{
		}
		
		void Verify() const
		{
			VerifyColor(colors._color);
			VerifyColor(colors._hidden_color);
		}
		
		static void VerifyColor(Color4f const & col)
		{
			VerifyColorComp(col.r);
			VerifyColorComp(col.g);
			VerifyColorComp(col.b);
			VerifyColorComp(col.a);
		}
		
		static void VerifyColorComp(float c)
		{
			Assert(c >= 0 && c <= 1);
		}
		
		void Draw(Vector3 const & camera_pos, bool hidden) const
		{
			Color4f const & color = hidden ? colors._hidden_color : colors._color;
			gl::SetColor(color.r, color.g, color.b, color.a);
			gl::Vertex3(pos.x - camera_pos.x, pos.y - camera_pos.y, pos.z - camera_pos.z);
		}
		
		Vector3 pos;
		ColorPair colors;
	};
	
	class PointArray
	{
	public:
		PointArray(int init_mode) : mode(init_mode) 
		{ 
		}
		
		void Verify() const
		{
			switch (mode)
			{
				case GL_POINTS:
					break;
					
				case GL_LINES:
					Assert((points.size() % 2) == 0);
					break;
					
				case GL_TRIANGLES:
					Assert((points.size() % 3) == 0);
					break;
					
				default:
					Assert(false);
			}

			Assert(points.size() >= 0 && points.size() < 1000000);
			for (point_vector::const_iterator i = points.begin(); i != points.end(); ++ i)
			{
				Point const & p = * i;
				p.Verify();
			}
		}
		
		void Clear()
		{
			points.clear();
		}
		
		void AddPoint(Vector3 const & pos, ColorPair const & colors)
		{
			points.push_back(Point(pos, ColorPair(colors)));
		}
		
		void Draw(Vector3 const & camera_pos, bool hidden) const
		{
			gl::Begin(mode);
			for (point_vector::const_iterator it = points.begin(); it != points.end(); ++ it)
			{
				Point const & point = * it;
				point.Draw(camera_pos, hidden);
			}
			gl::End();
		}
		
	private:
		
		typedef std::vector<Point> point_vector;
		
		point_vector points;
		GLenum mode;
	};

	PointArray points(GL_POINTS);
	PointArray lines(GL_LINES);
	PointArray tris(GL_TRIANGLES);

	void DrawPrimatives(Vector3 const & camera_pos, bool hidden)
	{
		if (hidden) {
			gl::SetDepthFunc(GL_GREATER);
		}

		points.Draw(camera_pos, hidden);
		lines.Draw(camera_pos, hidden);
		tris.Draw(camera_pos, hidden);

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

}



////////////////////////////////////////////////////////////////////////////////
// gfx::Debug defines


// Start things up.
void Debug::Init()
{
	Assert(font == nullptr);

	// Some font sources:
	// http://www.amanith.org/testsuite/amanithvg_gle/data/font_bitmap.png
	// http://www.ogre3d.org/wiki/index.php/Outlined_Fonts

	// Is this failing to load? Perhaps you forgot zlib1.dll or libpng12-0.dll. 
	// http://www.libsdl.org/projects/SDL_image/
	font = new Font("font_bitmap.bmp", .5f);
}


// Close things down.
void Debug::Deinit()
{
	Assert(font != nullptr);
	delete font;
	font = nullptr;
}


// Run any and all sanity checks.
void Debug::Verify()
{
	points.Verify();
	lines.Verify();
	tris.Verify();
}


// Return a value indicating how much logging output gets printed to screen each frame.
double Debug::GetVerbosity() 
{ 
#if defined(NDEBUG)
	return .25;
#else
	return debug_verbosity;
#endif
}


void Debug::AddPoint(Vector3 const & a, ColorPair const & colors)
{
	mutex.Lock();
	points.AddPoint(a, colors);
	mutex.Unlock();
}

void Debug::AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b)
{
	mutex.Lock();
	lines.AddPoint(a, colors_a);
	lines.AddPoint(b, colors_b);
	mutex.Unlock();
}

void Debug::AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, ColorPair const & colors)
{
	mutex.Lock();
	tris.AddPoint(a, colors);
	tris.AddPoint(b, colors);
	tris.AddPoint(c, colors);
	mutex.Unlock();
}

void Debug::AddBasis(Vector3 const & center, double scale)
{
	AddBasis(center, Matrix33::Identity(), scale);
}

void Debug::AddBasis(Vector3 const & center, Matrix33 const & rotation, double scale)
{
	using namespace axes;
	
	Debug::AddLine(center, center + GetAxis(rotation, RIGHT) * scale, Debug::ColorPair(Color4f::Red()));
	Debug::AddLine(center, center + GetAxis(rotation, FORWARD) * scale, Debug::ColorPair(Color4f::Green()));
	Debug::AddLine(center, center + GetAxis(rotation, UP) * scale, Debug::ColorPair(Color4f::Blue()));
	
	Debug::AddLine(center, center - GetAxis(rotation, RIGHT) * scale, Debug::ColorPair(Color4f::Cyan()));
	Debug::AddLine(center, center - GetAxis(rotation, FORWARD) * scale, Debug::ColorPair(Color4f::Magenta()));
	Debug::AddLine(center, center - GetAxis(rotation, UP) * scale, Debug::ColorPair(Color4f::Yellow()));
}

#if 0
void Debug::AddFrustum(Pov const & pov)
{
	Vector3 corners[2][2][2];
	Vector3 eye;
	
	{
		Vector3 extent_factors[2];
		
		{
			Frustum const & frustum = pov.GetFrustum();
			
			double aspect = static_cast<float>(frustum.resolution.x) / frustum.resolution.y;
			double y_factor = static_cast<float>(Sin(frustum.fov * .5));
			double x_factor = y_factor * aspect;
			
			extent_factors[0] = Vector3(- x_factor, - y_factor, static_cast<double>(frustum.near_z));
			extent_factors[1] = Vector3(x_factor, y_factor, static_cast<double>(frustum.far_z));
		}
		
		{
			Matrix44 m = pov.GetTransformation();
			
			for (int z_index = 0; z_index < 2; ++ z_index) {
				for (int y_index = 0; y_index < 2; ++ y_index) {
					for (int x_index = 0; x_index < 2; ++ x_index) {
						Vector4 p (extent_factors[x_index].x * extent_factors[z_index].z, 
									extent_factors[z_index].z,
									extent_factors[y_index].y * extent_factors[z_index].z, 
									1.);
						//p.z -= frustum.near_z;
						
						Vector3 & corner = corners[z_index][y_index][x_index];
						Vector4 mult = m * Vector4(p.x, p.y, p.z, p.w);
						corner.x = mult.x;
						corner.y = mult.y;
						corner.z = mult.z;
					}
				}
			}
			
			//eye = static_cast< sim::Vector3 >(m * sim::Vector4(0, 0, 0, 1));
			eye = Vector3f::Zero();
		}
	}
	
	// view box
	for (int axis = 0; axis < 3; ++ axis) {
		int perp_axis_1 = TriMod(axis + 1);
		int perp_axis_2 = TriMod(axis + 2);
		Vector3i indices;
		
		for (int p = 0; p < 2; ++ p) {
			indices [perp_axis_1] = p;
			for (int q = 0; q < 2; ++ q) {
				indices [perp_axis_2] = q;
				
				Vector3 const * a, * b;
				
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
				
				AddLine(* a, * b, ColorPair(visible, invisible));
			}
		}
	}
	
	// pyramid
	/*Vector3f center = static_cast< Vector3<float> >(Vector4f(0, 0, - frustum.near_z, 0) * m);
	 for (int p = 0; p < 2; ++ p) {
	 for (int q = 0; q < 2; ++ q) {
	 Vector3 const & a = corners[0][q][p];
	 Vector3 const & b = center;
	 
	 AddLine(a, b, ColorPair(Color(1.f, 1.f), Color(.25f, .5f)));
	 }
	 }*/
}
#endif

void Debug::Draw(Vector3 const & camera_pos)
{
	mutex.Lock();
	
	GLPP_VERIFY;
	Verify();

	// Set state
	gl::Disable(GL_CULL_FACE);
	Assert(! gl::IsEnabled(GL_LIGHT0));
	Assert(! gl::IsEnabled(GL_LIGHTING));
	gl::Enable(GL_DEPTH_TEST);
	
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl::Enable(GL_BLEND);
	
	DrawPrimatives(camera_pos, false);
	DrawPrimatives(camera_pos, true);
	ClearPrimatives();
	
	// Unset state
	gl::Enable(GL_CULL_FACE);
	gl::Disable(GL_DEPTH_TEST);
	gl::Disable(GL_BLEND);
	gl::SetColor<GLfloat>(1,1,1);

	Verify();
	GLPP_VERIFY;
	mutex.Unlock();
}

void Debug::DrawText(char const * text, Vector2i const & position)
{
	if (! (* font))
	{
		return;
	}
	
	font->Print(text, position);
}

#endif
