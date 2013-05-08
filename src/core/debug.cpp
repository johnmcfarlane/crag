//
//  debug.cpp
//  crag
//
//  Created by John on 2/10/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "core/debug.h"

////////////////////////////////////////////////////////////////////////////////
// core::DebugSetThreadName/DebugSetThreadName definitions

#if ! defined(NDEBUG)
namespace
{
	constexpr auto _thread_name_max_len = 16;
	thread_local char _thread_name[_thread_name_max_len + 1] = "";
}

void core::DebugSetThreadName(char const * thread_name)
{
	strncpy(_thread_name, thread_name, _thread_name_max_len);
	_thread_name[_thread_name_max_len] = '\0';
}

char const * core::DebugGetThreadName()
{
	return _thread_name;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// core::PrintMessage definition

namespace
{
#if defined(WIN32)

	char * buffer = nullptr;
	std::size_t buffer_size = 0u;

	// redirects output to a string buffer and sends it to CRT debugging console
	void VFPrintF(FILE * out, char const * format, va_list args)
	{
		ASSERT(out == stdout || out == stderr);

		// attempt to print using existing buffer
		std::size_t size = vsnprintf(buffer, buffer_size, format, args);

		// resize buffer if it's too small
		if (size >= buffer_size)
		{
			delete buffer;
			buffer_size = (size + 128) * 2;
			buffer = new char [buffer_size];

			if (vsnprintf(buffer, buffer_size, format, args) != size)
			{
				// this should not happen
				__debugbreak();
			}
		}

		// send buffer to debugging console
		OutputDebugStringA(buffer);
	}

#else

	void VFPrintF(FILE * out, char const * format, va_list args)
	{
		vfprintf(out, format, args);
	}

#endif
}

void PrintMessage(FILE * out, char const * format, ...)
{
	va_list args;
	va_start(args, format);
	VFPrintF(out, format, args);
	va_end(args);
}

////////////////////////////////////////////////////////////////////////////////
// ReentryGuard member definitions

#if ! defined(NDEBUG) && ! defined(WIN32)

ReentryGuard::ReentryGuard(int & counter)
: _counter(counter) 
{ 
	assert(++ _counter == 1); 

} 

ReentryGuard::~ReentryGuard() 
{ 
	assert(-- _counter == 0); 
}

#endif
