/*
 *  Script.cpp
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009 - 2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Script.h"

#include <sstream>


vm::Script::Script()
{
}

void vm::Script::Compile(char const * source_code)
{
	v8::Handle<v8::String> v8_source = v8::String::New(source_code);
	script = v8::Script::Compile(v8_source);
}

void vm::Script::Compile(std::istream & source_file)
{
	source_file.seekg(0,std::ios::end);
	std::streampos length = source_file.tellg();
	source_file.seekg(0,std::ios::beg);
	
	std::vector<char> buffer(length + std::streampos(1));
	source_file.read(&buffer[0],length);
	buffer [length] = '\0';

	char const * source_code = & buffer.front();
	Compile(source_code);
}

void vm::Script::Run()
{
	Assert(! script.IsEmpty());
	
	// Run the script to get the result.
	v8::Handle<v8::Value> result = script->Run();
	
	// Convert the result to an ASCII string and print it.
	v8::String::AsciiValue ascii(result);
	puts(* ascii);
}
