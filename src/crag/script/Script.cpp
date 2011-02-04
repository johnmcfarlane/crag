/*
 *  Script.cpp
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Script.h"

#include <fstream>


script::Script::Script()
: buffer(nullptr)
{
}

script::Script::Script(std::istream & source_file)
: buffer(nullptr)
{
	Load(source_file);
}

script::Script::Script(char const * source_filename)
: buffer(nullptr)
{
	Load(source_filename);
}

script::Script::~Script()
{
	delete [] buffer;
}

void script::Script::Clear()
{
	if (buffer != nullptr)
	{
		delete [] buffer;
		buffer = nullptr;
	}
}

void script::Script::Load(std::istream & source_file)
{
	// Get the length of the file.
	source_file.seekg(0,std::ios::end);
	std::streampos length = source_file.tellg();
	source_file.seekg(0,std::ios::beg);
	
	// Read file as a string and terminate with null.
	buffer = new char [length + std::streampos(1)];
	source_file.read(buffer,length);
	buffer [length] = '\0';
}

bool script::Script::Load(char const * source_filename)
{
	Clear();
	
	std::ifstream source_file(source_filename);
	if (! source_file.good()) 
	{
		return false;
	}
	
	Load(source_file);
	return true;
}

bool script::Script::IsOk() const
{
	return buffer != nullptr;
}

char const * script::Script::GetBuffer() const
{
	return buffer;
}
