//
//  Debug.cpp
//  crag
//
//  Created by John on 12/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Debug.h"

#include "core/ConfigEntry.h"

#if defined(CRAG_GFX_DEBUG)

#include "axes.h"
#include "Color.h"
#include "Font.h"
#include "Pov.h"

#include "geom/MatrixOps.h"


using namespace gfx;
using namespace Debug;


////////////////////////////////////////////////////////////////////////////////
// Local definitions

namespace
{
	// Messages of below this verbosity value should get printed.
	CONFIG_DEFINE(debug_verbosity, double, .5);
		
	// Mutex for global functions.
	std::mutex mutex;
	
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
		
		static void VerifyColor(Color const & col)
		{
			VerifyColorComp(col.r);
			VerifyColorComp(col.g);
			VerifyColorComp(col.b);
			VerifyColorComp(col.a);
		}
		
		static void VerifyColorComp(float CRAG_DEBUG_PARAM(c))
		{
			ASSERT(c >= 0 && c <= 1);
		}
		
		void Draw(bool hidden) const
		{
			Color const & color = hidden ? colors._hidden_color : colors._color;
			glColor4fv(color.GetArray());

			glVertex3f(pos.x, pos.y, pos.z);
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
					ASSERT((points.size() % 2) == 0);
					break;
					
				case GL_TRIANGLES:
					ASSERT((points.size() % 3) == 0);
					break;
					
				default:
					ASSERT(false);
			}

			ASSERT(points.size() < 1000000);
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
		
		void Draw(bool hidden) const
		{
			glBegin(mode);
			for (point_vector::const_iterator it = points.begin(); it != points.end(); ++ it)
			{
				Point const & point = * it;
				point.Draw(hidden);
			}
			glEnd();
		}
		
	private:
		
		typedef std::vector<Point> point_vector;
		
		point_vector points;
		GLenum mode;
	};

	PointArray points(GL_POINTS);
	PointArray lines(GL_LINES);
	PointArray tris(GL_TRIANGLES);

	void DrawPrimatives(bool hidden)
	{
		if (hidden)
		{
			glDepthFunc(GL_GREATER);
		}

		points.Draw(hidden);
		lines.Draw(hidden);
		tris.Draw(hidden);

		if (hidden)
		{
			glDepthFunc(GL_LEQUAL);
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
void gfx::Debug::Init()
{
	ASSERT(font == nullptr);

	// Some font sources:
	// http://www.amanith.org/testsuite/amanithvg_gle/data/font_bitmap.png
	// http://www.ogre3d.org/wiki/index.php/Outlined_Fonts

	// Is this failing to load? Perhaps you forgot zlib1.dll or libpng12-0.dll. 
	// http://www.libsdl.org/projects/SDL_image/
	font = new Font("font_bitmap.bmp", .5f);
}


// Close things down.
void gfx::Debug::Deinit()
{
	ASSERT(font != nullptr);
	delete font;
	font = nullptr;
}


// Run any and all sanity checks.
void gfx::Debug::Verify()
{
	points.Verify();
	lines.Verify();
	tris.Verify();
}


// Return a value indicating how much logging output gets printed to screen each frame.
double gfx::Debug::GetVerbosity() 
{ 
#if defined(NDEBUG)
	return .25;
#else
	return debug_verbosity;
#endif
}


void gfx::Debug::AddPoint(Vector3 const & a, ColorPair const & colors)
{
	mutex.lock();
	points.AddPoint(a, colors);
	mutex.unlock();
}

void gfx::Debug::AddLine(Vector3 const & a, Vector3 const & b, ColorPair const & colors_a, ColorPair const & colors_b)
{
	mutex.lock();
	lines.AddPoint(a, colors_a);
	lines.AddPoint(b, colors_b);
	mutex.unlock();
}

void gfx::Debug::AddTriangle(Triangle3 const & triangle, ColorPair const & colors)
{
	mutex.lock();
	tris.AddPoint(triangle.points[0], colors);
	tris.AddPoint(triangle.points[1], colors);
	tris.AddPoint(triangle.points[2], colors);
	mutex.unlock();
}

void gfx::Debug::AddBasis(Vector3 const & center, Vector3 const & scale, Matrix33 const & rotation)
{
	AddLine(center, center + GetAxis(rotation, Direction::right) * scale.x, ColorPair(Color::Red()));
	AddLine(center, center + GetAxis(rotation, Direction::forward) * scale.y, ColorPair(Color::Green()));
	AddLine(center, center + GetAxis(rotation, Direction::up) * scale.z, ColorPair(Color::Blue()));
	
	AddLine(center, center - GetAxis(rotation, Direction::right) * scale.x, ColorPair(Color::Cyan()));
	AddLine(center, center - GetAxis(rotation, Direction::forward) * scale.y, ColorPair(Color::Magenta()));
	AddLine(center, center - GetAxis(rotation, Direction::up) * scale.z, ColorPair(Color::Yellow()));
}

#if 0
void AddFrustum(Pov const & pov)
{
	Vector3 corners[2][2][2];
	Vector3 eye;
	
	{
		Vector3 extent_factors[2];
		
		{
			Frustum const & frustum = pov.GetFrustum();
			
			double aspect = static_cast<float>(frustum.resolution.x) / frustum.resolution.y;
			double y_factor = static_cast<float>(sin(frustum.fov * .5));
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
			
			//eye = static_cast< sim::Vector3 >(m * simgeom::Vector4(0, 0, 0, 1));
			eye = geom::Vector3f::Zero();
		}
	}
	
	// view box
	for (int axis = 0; axis < 3; ++ axis) {
		int perp_axis_1 = TriMod(axis + 1);
		int perp_axis_2 = TriMod(axis + 2);
		geom::Vector3i indices;
		
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
				
				Color visible = Color::Black();
				visible[axis] = 1;
				
				Color invisible = visible;
				invisible.a = .2f;
				
				AddLine(* a, * b, ColorPair(visible, invisible));
			}
		}
	}
	
	// pyramid
	/*geom::Vector3f center = static_cast< Vector3<float> >(geom::Vector4f(0, 0, - frustum.near_z, 0) * m);
	 for (int p = 0; p < 2; ++ p) {
	 for (int q = 0; q < 2; ++ q) {
	 Vector3 const & a = corners[0][q][p];
	 Vector3 const & b = center;
	 
	 AddLine(a, b, ColorPair(Color(1.f, 1.f), Color(.25f, .5f)));
	 }
	 }*/
}
#endif

void gfx::Debug::Draw(Matrix44 const & model_view_matrix, Matrix44 const & projection_matrix)
{
	mutex.lock();
	
	// Set the model view and projection matrices
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(geom::Transposition(ToOpenGl(geom::Inverse(model_view_matrix))).GetArray());
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(geom::Transposition(projection_matrix).GetArray());
	
	GL_VERIFY;
	Verify();

	// Set state
	ASSERT(! IsEnabled(GL_LIGHTING));
	
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	DrawPrimatives(false);
	DrawPrimatives(true);
	
	// Unset state
	glColor3f(1,1,1);

	Verify();
	GL_VERIFY;
	mutex.unlock();
}

void gfx::Debug::Clear()
{
	mutex.lock();
	ClearPrimatives();
	mutex.unlock();
}

void gfx::Debug::DrawText(char const * text, geom::Vector2i const & position)
{
	if (! (* font))
	{
		return;
	}
	
	font->Print(text, geom::Cast<float>(position));
}

#endif
