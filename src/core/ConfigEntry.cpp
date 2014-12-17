//
//  ConfigEntry.cpp
//  crag
//
//  Created by john on 6/26/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ConfigEntry.h"

#if defined(ENABLE_CONFIG)

#include "ConfigManager.h"

#include "gfx/Color.h"

#include "geom/Matrix44.h"

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
				
				ASSERT(! isupper(this_char));
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

namespace crag {
namespace core {
namespace config
{
	template<> int ::crag::core::config::GenericEntry<bool>::ValueToString(char * string, bool const & value)
	{
		return sprintf(string, "%s", value ? "true" : "false") > 0;
	}

	template<> int GenericEntry<bool>::StringToValue(bool & value, char const * string)
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
	template<> int GenericEntry<int>::ValueToString(char * string, int const & value)
	{
		return sprintf(string, "%d", value) > 0;
	}

	template<> int GenericEntry<int>::StringToValue(int & value, char const * string)
	{
		return sscanf(string, "%d", & value) == 1;
	}

	// size_t
	template<> int GenericEntry<size_t>::ValueToString(char * string, size_t const & value)
	{
		return sprintf(string, SIZE_T_FORMAT_SPEC, value) > 0;
	}

	template<> int GenericEntry<size_t>::StringToValue(size_t & value, char const * string)
	{
		return sscanf(string, SIZE_T_FORMAT_SPEC, & value) == 1;
	}

	// float
	template<> int GenericEntry<float>::ValueToString(char * string, float const & value)
	{
		return sprintf(string, "%g", value) > 0;
	}

	template<> int GenericEntry<float>::StringToValue(float & value, char const * string)
	{
		return sscanf(string, "%g", & value) == 1;
	}

	// double
	template<> int GenericEntry<double>::ValueToString(char * string, double const & value)
	{
		return sprintf(string, "%g", value) > 0;
	}

	template<> int GenericEntry<double>::StringToValue(double & value, char const * string)
	{
		return sscanf(string, "%lg", & value) == 1;
	}

	// Color
	template<> int GenericEntry<gfx::Color4f>::ValueToString(char * string, gfx::Color4f const & value)
	{
		return sprintf(string, "%g,%g,%g,%g", value.r, value.g, value.b, value.a) > 0;
	}

	template<> int GenericEntry<gfx::Color4f>::StringToValue(gfx::Color4f & value, char const * string)
	{
		return sscanf(string, "%g,%g,%g,%g", & value.r, & value.g, & value.b, & value.a) == 1;
	}

	// geom::Vector3f
	template<> int GenericEntry<geom::Vector3f>::ValueToString(char * string, geom::Vector3f const & value)
	{
		return sprintf(string, "%g,%g,%g", value.x, value.y, value.z) > 0;
	}

	template<> int GenericEntry<geom::Vector3f>::StringToValue(geom::Vector3f & value, char const * string)
	{
		return sscanf(string, "%g,%g,%g", & value.x, & value.y, & value.z) == 1;
	}

	// geom::Vector3d
	template<> int GenericEntry<geom::Vector3d>::ValueToString(char * string, geom::Vector3d const & value)
	{
		return sprintf(string, "%g,%g,%g", value.x, value.y, value.z) > 0;
	}

	template<> int GenericEntry<geom::Vector3d>::StringToValue(geom::Vector3d & value, char const * string)
	{
		return sscanf(string, "%lg,%lg,%lg", & value.x, & value.y, & value.z) == 1;
	}

	// geom::Matrix44f
	template<> int GenericEntry<geom::Matrix44f>::ValueToString(char * string, geom::Matrix44f const & value)
	{
		return sprintf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					   value[0][0], value[0][1], value[0][2], value[0][3], 
					   value[1][0], value[1][1], value[1][2], value[1][3], 
					   value[2][0], value[2][1], value[2][2], value[2][3], 
					   value[3][0], value[3][1], value[3][2], value[3][3]) > 0;
	}

	template<> int GenericEntry<geom::Matrix44f>::StringToValue(geom::Matrix44f & value, char const * string)
	{
		return sscanf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					  & value[0][0], & value[0][1], & value[0][2], & value[0][3], 
					  & value[1][0], & value[1][1], & value[1][2], & value[1][3], 
					  & value[2][0], & value[2][1], & value[2][2], & value[2][3], 
					  & value[3][0], & value[3][1], & value[3][2], & value[3][3]) == 1;
	}

	// geom::Matrix44d
	template<> int GenericEntry<geom::Matrix44d>::ValueToString(char * string, geom::Matrix44d const & value)
	{
		return sprintf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					   value[0][0], value[0][1], value[0][2], value[0][3], 
					   value[1][0], value[1][1], value[1][2], value[1][3], 
					   value[2][0], value[2][1], value[2][2], value[2][3], 
					   value[3][0], value[3][1], value[3][2], value[3][3]) > 0;
	}

	template<> int GenericEntry<geom::Matrix44d>::StringToValue(geom::Matrix44d & value, char const * string)
	{
		return sscanf(string, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", 
					  & value[0][0], & value[0][1], & value[0][2], & value[0][3], 
					  & value[1][0], & value[1][1], & value[1][2], & value[1][3], 
					  & value[2][0], & value[2][1], & value[2][2], & value[2][3], 
					  & value[3][0], & value[3][1], & value[3][2], & value[3][3]) == 1;
	}

	// float
	template<> int GenericEntryAngle<float>::ValueToString(char * string, float const & value)
	{
		return sprintf(string, "%g", RadToDeg(value)) > 0;
	}

	template<> int GenericEntryAngle<float>::StringToValue(float & value, char const * string)
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
	template<> int GenericEntryAngle<double>::ValueToString(char * string, double const & value)
	{
		return sprintf(string, "%g", RadToDeg(value)) > 0;
	}

	template<> int GenericEntryAngle<double>::StringToValue(double & value, char const * string)
	{
		if (sscanf(string, "%lg", & value) == 1) {
			value = DegToRad(value);
			return 1;
		}
		else {
			return 0;
		}
	}
}}}

#endif	// ENABLE_CONFIG
