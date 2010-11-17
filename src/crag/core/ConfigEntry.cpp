/*
 *  ConfigEntry.cpp
 *  Crag
 *
 *  Created by john on 6/26/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "ConfigEntry.h"

#if defined(ENABLE_CONFIG)

#include "ConfigManager.h"

#include "geom/Vector3.h"

#include "gfx/Color.h"

#include "geom/Matrix4.h"

#include <cstdio>


namespace core {


//////////////////////////////////////////////////////////////////////
// core::ConfigEntry function definitions

ConfigEntry::ConfigEntry(char const * init_name)
: super(init_name)
{
}


//////////////////////////////////////////////////////////////////////
// ValueToString / StringToValue specializations

// bool
namespace 
{
	struct boolean_tag_pair {
		char const * strings[2];
		
		bool matches(char const * that_str, bool b) const
		{
			char const * this_str = strings[b];
			char that_char, this_char;
			do {
				that_char = * (that_str ++);
				this_char = * (this_str ++);

				if (this_char == 0) {
					return that_char == 0 || isspace(that_char);
				}
				
				Assert(! isupper(this_char));
			}	while (tolower(that_char) == this_char);
			
			return false;
		}
	};
	
	boolean_tag_pair const boolean_tags[] = {
		{ { "false", "true" } },
		{ { "no", "yes" } },
		{ { "n", "y" } },
		{ { "non", "oui" } },
		{ { "non", "o" } },
		{ { "0", "1" } }
	};
	boolean_tag_pair const * const boolean_tags_end = boolean_tags + sizeof(boolean_tags) / sizeof(* boolean_tags);
	
	
}

template<> int Config<bool>::ValueToString(char * string, bool const & value)
{
	return sprintf(string, "%s", value ? "true" : "false") > 0;
}

template<> int Config<bool>::StringToValue(bool & value, char const * string)
{
	for (boolean_tag_pair const * tag_iterator = boolean_tags; tag_iterator != boolean_tags_end; ++ tag_iterator) {
		if (tag_iterator->matches(string, false)) {
			value = false;
			return 1;
		}
		if (tag_iterator->matches(string, true)) {
			value = true;
			return 1;
		}
	}
	
	return 0;
}

// int
template<> int Config<int>::ValueToString(char * string, int const & value)
{
	return sprintf(string, "%d", value) > 0;
}

template<> int Config<int>::StringToValue(int & value, char const * string)
{
	return sscanf(string, "%d", & value) == 1;
}

// float
template<> int Config<float>::ValueToString(char * string, float const & value)
{
	return sprintf(string, "%g", value) > 0;
}

template<> int Config<float>::StringToValue(float & value, char const * string)
{
	return sscanf(string, "%g", & value) == 1;
}

// double
template<> int Config<double>::ValueToString(char * string, double const & value)
{
	return sprintf(string, "%g", value) > 0;
}

template<> int Config<double>::StringToValue(double & value, char const * string)
{
	return sscanf(string, "%lg", & value) == 1;
}

// Color
template<> int Config<gfx::Color4f>::ValueToString(char * string, gfx::Color4f const & value)
{
	return sprintf(string, "%g,%g,%g,%g", value.r, value.g, value.b, value.a) > 0;
}

template<> int Config<gfx::Color4f>::StringToValue(gfx::Color4f & value, char const * string)
{
	return sscanf(string, "%g,%g,%g,%g", & value.r, & value.g, & value.b, & value.a) == 1;
}

// Vector3f
template<> int Config<Vector3f>::ValueToString(char * string, Vector3f const & value)
{
	return sprintf(string, "%g,%g,%g", value.x, value.y, value.z) > 0;
}

template<> int Config<Vector3f>::StringToValue(Vector3f & value, char const * string)
{
	return sscanf(string, "%g,%g,%g", & value.x, & value.y, & value.z) == 1;
}

// Vector3d
template<> int Config<Vector3d>::ValueToString(char * string, Vector3d const & value)
{
	return sprintf(string, "%g,%g,%g", value.x, value.y, value.z) > 0;
}

template<> int Config<Vector3d>::StringToValue(Vector3d & value, char const * string)
{
	return sscanf(string, "%lg,%lg,%lg", & value.x, & value.y, & value.z) == 1;
}

// Matrix4f
template<> int Config<Matrix4f>::ValueToString(char * string, Matrix4f const & value)
{
	return sprintf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
				   value[0][0], value[0][1], value[0][2], value[0][3], 
				   value[1][0], value[1][1], value[1][2], value[1][3], 
				   value[2][0], value[2][1], value[2][2], value[2][3], 
				   value[3][0], value[3][1], value[3][2], value[3][3]) > 0;
}

template<> int Config<Matrix4f>::StringToValue(Matrix4f & value, char const * string)
{
	return sscanf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
				  & value[0][0], & value[0][1], & value[0][2], & value[0][3], 
				  & value[1][0], & value[1][1], & value[1][2], & value[1][3], 
				  & value[2][0], & value[2][1], & value[2][2], & value[2][3], 
				  & value[3][0], & value[3][1], & value[3][2], & value[3][3]) == 1;
}

// Matrix4d
template<> int Config<Matrix4d>::ValueToString(char * string, Matrix4d const & value)
{
	return sprintf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
				   value[0][0], value[0][1], value[0][2], value[0][3], 
				   value[1][0], value[1][1], value[1][2], value[1][3], 
				   value[2][0], value[2][1], value[2][2], value[2][3], 
				   value[3][0], value[3][1], value[3][2], value[3][3]) > 0;
}

template<> int Config<Matrix4d>::StringToValue(Matrix4d & value, char const * string)
{
	return sscanf(string, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", 
				  & value[0][0], & value[0][1], & value[0][2], & value[0][3], 
				  & value[1][0], & value[1][1], & value[1][2], & value[1][3], 
				  & value[2][0], & value[2][1], & value[2][2], & value[2][3], 
				  & value[3][0], & value[3][1], & value[3][2], & value[3][3]) == 1;
}

// float
template<> int ConfigAngle<float>::ValueToString(char * string, float const & value)
{
	return sprintf(string, "%g", RadToDeg(value)) > 0;
}

template<> int ConfigAngle<float>::StringToValue(float & value, char const * string)
{
	if (sscanf(string, "%g", & value) == 1) {
		value = DegToRad(value);
		return 1;
	}
	else {
		return 0;
	}
}

// double
template<> int ConfigAngle<double>::ValueToString(char * string, double const & value)
{
	return sprintf(string, "%g", RadToDeg(value)) > 0;
}

template<> int ConfigAngle<double>::StringToValue(double & value, char const * string)
{
	if (sscanf(string, "%lg", & value) == 1) {
		value = DegToRad(value);
		return 1;
	}
	else {
		return 0;
	}
}

}	// namespace core

#endif	// ENABLE_CONFIG
