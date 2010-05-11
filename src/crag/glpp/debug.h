/*
 *  GlUtils.h
 *  Crag
 *
 *  Created by John on 12/2/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

// Utility fns/macros for the GLPP library.
// (Nothing directly to do with GLUT or GLU.)

#pragma once

#include <cassert>


//#define GLPP_LOG


// Place GLPP_VERIFY; in code to assert that no GL errors have been generate. 
#if defined(NDEBUG)
#define GLPP_VERIFY DO_NOTHING
#else
#define GLPP_VERIFY gl::Verify(__FILE__, __LINE__, "")
#endif


// Wrap a call to a GL function in CLPP_CALL to perform a GL error check on the way out.
#define GLPP_CALL(statement) statement; gl::VerifyStatement(__FILE__, __LINE__, #statement)


namespace gl
{
	
	inline std::ostream & Log(char const * file, int line, std::ostream & out)
	{
		return out << file << ':' << std::dec << line << ':' << ' ';
	}
	
	// true means everything ok.
#if defined(NDEBUG)
	inline void Verify(char const * file, int line, char const * statement) { }
#else
	inline void Verify(char const * file, int line, char const * statement)
	{
		GLenum gl_error = glGetError(); 
		if (gl_error == GL_NO_ERROR) {
			return;
		}
		Log(file, line, std::cerr) << "error 0x" << std::hex << gl_error << ": " << gluErrorString(gl_error) << ", \"" << statement << '"' << std::endl;
		assert(false);
	}
#endif
	
	inline void VerifyStatement(char const * file, int line, char const * statement)
	{
#if ! defined(NDEBUG)
#if defined(GLPP_LOG)
		Log(file, line, std::cout) << statement << std::endl;
#endif
		Verify(file, line, statement);
#endif
	}

}
