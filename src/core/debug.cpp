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

#if defined(CRAG_OS_ANDROID)
#include <android/log.h>
#elif defined(CRAG_OS_WINDOWS)
#include "core/windows.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// core::DebugSetThreadName/DebugSetThreadName definitions

#if defined(CRAG_DEBUG)
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
// crag::core::Break definition

void crag::core::Break()
{
#if defined(CRAG_COMPILER_MSVC)
	assert(false);	// sometimes __debugbreak does nothing
	__debugbreak();
#elif defined(CRAG_OS_ANDROID)
	__android_log_assert("error", "crag", "internal error");
#elif defined(CRAG_COMPILER_GCC)
	__builtin_trap();
#else
	assert(false);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// core::PrintMessage definition

namespace
{
#if defined(CRAG_OS_WINDOWS)
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

#elif defined(CRAG_OS_ANDROID)

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

#if defined(CRAG_DEBUG) && ! defined(CRAG_OS_WINDOWS)

ReentryGuard::ReentryGuard(int & counter)
: _counter(counter) 
{
	ASSERT(_counter == 0);
	++ _counter;
}

ReentryGuard::~ReentryGuard() 
{
	-- _counter;
	ASSERT(_counter == 0);
}

#endif
