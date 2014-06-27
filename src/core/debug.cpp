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

#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(WIN32)
#include "core/windows.h"
#endif

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
	std::vector<char> buffer;

	// redirects output to a string buffer and sends it to CRT debugging console
	void VFPrintF(FILE * out, char const * format, va_list args)
	{
		ASSERT(out == stdout || out == stderr);

		// attempt to print using existing buffer
		auto buffer_size = int(buffer.size());
		auto output_size = vsnprintf(buffer.data(), buffer_size, format, args);
		ASSERT(output_size);

		if (output_size > 0)
		{
			if (output_size <= buffer_size)
			{
				// send buffer to debugging console
				OutputDebugStringA(buffer.data());

				return;
			}

			buffer.resize(output_size + 1);
		}
		else
		{
			buffer.resize(buffer_size << 1);
		}

		// try again
		VFPrintF(out, format, args);
	}

#elif defined(__ANDROID__)

	void VFPrintF(FILE * out, char const * format, va_list args)
	{
		android_LogPriority priority;
		if (out == stderr)
		{
			priority = ANDROID_LOG_ERROR;
		}
		else
		{
			priority = ANDROID_LOG_WARN;
		}
		
		__android_log_vprint(priority, "crag_log", format, args);
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
