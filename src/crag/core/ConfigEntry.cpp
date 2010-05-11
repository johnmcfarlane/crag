/*
 *  ConfigEntry.cpp
 *  Crag
 *
 *  Created by john on 6/26/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "core/ConfigEntry.h"

#if defined(ENABLE_CONFIG)

#include "core/ConfigManager.h"
#include "core/Vector3.h"

#include "gfx/Color.h"

#include "Matrix4.h"


//////////////////////////////////////////////////////////////////////
// ConfigEntry function definitions

ConfigEntry::ConfigEntry(char const * init_name)
: name(init_name)
{
	ConfigManager::AddParameter(* this);
}

ConfigEntry::~ConfigEntry()
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

template<> int Config<bool>::ValueToString(char * string) const
{
	return sprintf(string, "%s", var ? "true" : "false") > 0;
}

template<> int Config<bool>::StringToValue(char const * string)
{
	for (boolean_tag_pair const * tag_iterator = boolean_tags; tag_iterator != boolean_tags_end; ++ tag_iterator) {
		if (tag_iterator->matches(string, false)) {
			var = false;
			return 1;
		}
		if (tag_iterator->matches(string, true)) {
			var = true;
			return 1;
		}
	}
	
	return 0;
}

// int
template<> int Config<int>::ValueToString(char * string) const
{
	return sprintf(string, "%d", var) > 0;
}

template<> int Config<int>::StringToValue(char const * string)
{
	return sscanf(string, "%d", & var) == 1;
}

// float
template<> int Config<float>::ValueToString(char * string) const
{
	return sprintf(string, "%g", var) > 0;
}

template<> int Config<float>::StringToValue(char const * string)
{
	return sscanf(string, "%g", & var) == 1;
}

// double
template<> int Config<double>::ValueToString(char * string) const
{
	return sprintf(string, "%g", var) > 0;
}

template<> int Config<double>::StringToValue(char const * string)
{
	return sscanf(string, "%lg", & var) == 1;
}

// Color
template<> int Config<gfx::Color4f>::ValueToString(char * string) const
{
	return sprintf(string, "%g,%g,%g,%g", var.r, var.g, var.b, var.a) > 0;
}

template<> int Config<gfx::Color4f>::StringToValue(char const * string)
{
	return sscanf(string, "%g,%g,%g,%g", & var.r, & var.g, & var.b, & var.a) == 1;
}

// Vector3f
template<> int Config<Vector3f>::ValueToString(char * string) const
{
	return sprintf(string, "%g,%g,%g", var.x, var.y, var.z) > 0;
}

template<> int Config<Vector3f>::StringToValue(char const * string)
{
	return sscanf(string, "%g,%g,%g", & var.x, & var.y, & var.z) == 1;
}

// Vector3d
template<> int Config<Vector3d>::ValueToString(char * string) const
{
	return sprintf(string, "%g,%g,%g", var.x, var.y, var.z) > 0;
}

template<> int Config<Vector3d>::StringToValue(char const * string)
{
	return sscanf(string, "%lg,%lg,%lg", & var.x, & var.y, & var.z) == 1;
}

// Matrix4f
template<> int Config<Matrix4f>::ValueToString(char * string) const
{
	return sprintf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
				   var[0][0], var[0][1], var[0][2], var[0][3], 
				   var[1][0], var[1][1], var[1][2], var[1][3], 
				   var[2][0], var[2][1], var[2][2], var[2][3], 
				   var[3][0], var[3][1], var[3][2], var[3][3]) > 0;
}

template<> int Config<Matrix4f>::StringToValue(char const * string)
{
	return sscanf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
				  & var[0][0], & var[0][1], & var[0][2], & var[0][3], 
				  & var[1][0], & var[1][1], & var[1][2], & var[1][3], 
				  & var[2][0], & var[2][1], & var[2][2], & var[2][3], 
				  & var[3][0], & var[3][1], & var[3][2], & var[3][3]) == 1;
}

// Matrix4d
template<> int Config<Matrix4d>::ValueToString(char * string) const
{
	return sprintf(string, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
				   var[0][0], var[0][1], var[0][2], var[0][3], 
				   var[1][0], var[1][1], var[1][2], var[1][3], 
				   var[2][0], var[2][1], var[2][2], var[2][3], 
				   var[3][0], var[3][1], var[3][2], var[3][3]) > 0;
}

template<> int Config<Matrix4d>::StringToValue(char const * string)
{
	return sscanf(string, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", 
				  & var[0][0], & var[0][1], & var[0][2], & var[0][3], 
				  & var[1][0], & var[1][1], & var[1][2], & var[1][3], 
				  & var[2][0], & var[2][1], & var[2][2], & var[2][3], 
				  & var[3][0], & var[3][1], & var[3][2], & var[3][3]) == 1;
}

// float
template<> int ConfigAngle<float>::ValueToString(char * string) const
{
	return sprintf(string, "%g", RadToDeg(var)) > 0;
}

template<> int ConfigAngle<float>::StringToValue(char const * string)
{
	if (sscanf(string, "%g", & var) == 1) {
		var = DegToRad(var);
		return 1;
	}
	else {
		return 0;
	}
}

// double
template<> int ConfigAngle<double>::ValueToString(char * string) const
{
	return sprintf(string, "%g", RadToDeg(var)) > 0;
}

template<> int ConfigAngle<double>::StringToValue(char const * string)
{
	if (sscanf(string, "%lg", & var) == 1) {
		var = DegToRad(var);
		return 1;
	}
	else {
		return 0;
	}
}

#endif	// ENABLE_CONFIG
