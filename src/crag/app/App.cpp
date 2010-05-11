/*
 *  Input.cpp
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "App.h"

#include "core/ConfigEntry.h"

#include <boost/thread/thread.hpp>


namespace ANONYMOUS
{
	boost::mutex gl_mutex;
}


namespace app
{
bool InitGl()
{
#if defined(GLEW_STATIC )
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK)
	{
		std::cerr << "GLEW Error: " << glewGetErrorString(glew_err) << std::endl;
		return false;
	}

	std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
#endif

	return true;
}

#if 0
bool InitMultithreadedGl()
{
	bool has_multithreaded_gl = false;

	// Run twice?
	Assert(! has_multithreaded_gl);
	
	if (! enable_multithreding) {
		std::cout << "Multithreading is disabled; slow\n";
		return false;
	}
	
#if defined(__APPLE__)
	has_multithreaded_gl = true;
	//	Doesn't seem to do much.
/*	CGLContextObj ctx = CGLGetCurrentContext();
	
	// Enable the multi-threading
	CGLError mt_err =  CGLEnable( ctx, kCGLCEMPEngine);
	
	has_multithreaded_gl = (mt_err == kCGLNoError);*/
#elif defined(WIN32)
	// ... well it seems to work. Mostly.
	has_multithreading = true;
#else
	has_multithreaded_gl = false;
#endif
	
	if (! has_multithreaded_gl)
	{
		std::cout << "Multithreaded OpenGL not supported.\n";
	}

	return has_multithreaded_gl;
}
#endif
	
void LockGl()
{
	gl_mutex.lock();
}

void UnlockGl()
{
	gl_mutex.unlock();
}

/*GlContext GetGlContext()
{
#if defined(__APPLE__)
	return CGLGetCurrentContext();
#elif defined(WIN32)
	render_context(sf::Context::GetGlobal());
#else
	return glXGetCurrentContext();
#endif
}

void SetGlContext(GlContext gl_context)
{
#if defined(__APPLE__)
	CGLSetCurrentContext(gl_context);
#elif defined(WIN32)
	Assert(! sf::Context::IsContextActive());
	render_context.SetActive(true);
	Assert(sf::Context::IsContextActive());
#else
	Bool result = glXMakeCurrent(display, drawable, gl_context);
	if (result != True)
	{
		Assert(false);
	}
#endif
}*/

void Sleep(TimeType t)
{
	boost::this_thread::sleep(boost::posix_time::milliseconds(t));
}
	
}

