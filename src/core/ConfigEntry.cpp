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
	template<> bool GenericEntry<bool>::ValueToString(bool const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%s", value ? "true" : "false") > 0;
	}

	template<> bool GenericEntry<bool>::StringToValue(bool & value, char const * string)
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
	template<> bool GenericEntry<int>::ValueToString(int const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%d", value) > 0;
	}

	template<> bool GenericEntry<int>::StringToValue(int & value, char const * string)
	{
		return sscanf(string, "%d", & value) == 1;
	}

	// std::uint64_t
	template<> bool GenericEntry<std::uint64_t>::ValueToString(std::uint64_t const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%" PRIu64, value) > 0;
	}

	template<> bool GenericEntry<std::uint64_t>::StringToValue(std::uint64_t & value, char const * string)
	{
		return sscanf(string, "%" PRIu64, &value) == 1;
	}

	// std::uint32_t
	template<> bool GenericEntry<std::uint32_t>::ValueToString(std::uint32_t const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%" PRIu32, value) > 0;
	}

	template<> bool GenericEntry<std::uint32_t>::StringToValue(std::uint32_t & value, char const * string)
	{
		return sscanf(string, "%" PRIu32, & value) == 1;
	}

	// float
	template<> bool GenericEntry<float>::ValueToString(float const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g", value) > 0;
	}

	template<> bool GenericEntry<float>::StringToValue(float & value, char const * string)
	{
		return sscanf(string, "%g", & value) == 1;
	}

	// double
	template<> bool GenericEntry<double>::ValueToString(double const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g", value) > 0;
	}

	template<> bool GenericEntry<double>::StringToValue(double & value, char const * string)
	{
		return sscanf(string, "%lg", & value) == 1;
	}

	// Color
	template<> bool GenericEntry<gfx::Color4f>::ValueToString(gfx::Color4f const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g,%g,%g,%g", value.r, value.g, value.b, value.a) > 0;
	}

	template<> bool GenericEntry<gfx::Color4f>::StringToValue(gfx::Color4f & value, char const * string)
	{
		return sscanf(string, "%g,%g,%g,%g", & value.r, & value.g, & value.b, & value.a) == 4;
	}

	// geom::Vector3f
	template<> bool GenericEntry<geom::Vector3f>::ValueToString(geom::Vector3f const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g,%g,%g", value.x, value.y, value.z) > 0;
	}

	template<> bool GenericEntry<geom::Vector3f>::StringToValue(geom::Vector3f & value, char const * string)
	{
		return sscanf(string, "%g,%g,%g", & value.x, & value.y, & value.z) == 3;
	}

	// geom::Vector3d
	template<> bool GenericEntry<geom::Vector3d>::ValueToString(geom::Vector3d const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g,%g,%g", value.x, value.y, value.z) > 0;
	}

	template<> bool GenericEntry<geom::Vector3d>::StringToValue(geom::Vector3d & value, char const * string)
	{
		return sscanf(string, "%lg,%lg,%lg", & value.x, & value.y, & value.z) == 3;
	}

	// geom::Matrix44f
	template<> bool GenericEntry<geom::Matrix44f>::ValueToString(geom::Matrix44f const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					   value[0][0], value[0][1], value[0][2], value[0][3], 
					   value[1][0], value[1][1], value[1][2], value[1][3], 
					   value[2][0], value[2][1], value[2][2], value[2][3], 
					   value[3][0], value[3][1], value[3][2], value[3][3]) > 0;
	}

	template<> bool GenericEntry<geom::Matrix44f>::StringToValue(geom::Matrix44f & value, char const * string)
	{
		return sscanf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					  & value[0][0], & value[0][1], & value[0][2], & value[0][3], 
					  & value[1][0], & value[1][1], & value[1][2], & value[1][3], 
					  & value[2][0], & value[2][1], & value[2][2], & value[2][3], 
					  & value[3][0], & value[3][1], & value[3][2], & value[3][3]) == 16;
	}

	// geom::Matrix44d
	template<> bool GenericEntry<geom::Matrix44d>::ValueToString(geom::Matrix44d const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					   value[0][0], value[0][1], value[0][2], value[0][3], 
					   value[1][0], value[1][1], value[1][2], value[1][3], 
					   value[2][0], value[2][1], value[2][2], value[2][3], 
					   value[3][0], value[3][1], value[3][2], value[3][3]) > 0;
	}

	template<> bool GenericEntry<geom::Matrix44d>::StringToValue(geom::Matrix44d & value, char const * string)
	{
		return sscanf(string, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", 
					  & value[0][0], & value[0][1], & value[0][2], & value[0][3], 
					  & value[1][0], & value[1][1], & value[1][2], & value[1][3], 
					  & value[2][0], & value[2][1], & value[2][2], & value[2][3], 
					  & value[3][0], & value[3][1], & value[3][2], & value[3][3]) == 16;
	}

	// float
	template<> bool GenericEntryAngle<float>::ValueToString(float const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g", RadToDeg(value)) > 0;
	}

	template<> bool GenericEntryAngle<float>::StringToValue(float & value, char const * string)
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
	template<> bool GenericEntryAngle<double>::ValueToString(double const & value, char * string, std::size_t max_string_size)
	{
		return snprintf(string, max_string_size, "%g", RadToDeg(value)) > 0;
	}

	template<> bool GenericEntryAngle<double>::StringToValue(double & value, char const * string)
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
