//
//  debug.h
//  crag
//
//  Created by John on 12/2/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

// Utility fns/macros for the GLPP library.
// (Nothing directly to do with GLUT or GLU.)

#pragma once

#include <cassert>


//#define GLPP_LOG


// Place GLPP_VERIFY; in code to assert that no GL errors have been generate. 
#if defined(NDEBUG)
#define GLPP_VERIFY do { } while (false)
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
	
#if defined(NDEBUG)

	inline void Verify(char const * file, int line, char const * statement) { }

#else

	inline void ReportError(char const * file, int line, char const * statement, GLenum error)
	{
		Log(file, line, std::cerr) << "error 0x" << std::hex << error << ": " << gluErrorString(error) << ", \"" << statement << '"' << std::endl << std::flush;
		assert(false);
	}

	inline void Verify(char const * file, int line, char const * statement)
	{
		GLenum error = glGetError(); 
		if (error == GL_NO_ERROR) 
		{
			return;
		}
		ReportError(file, line, statement, error);
	}

#endif
	
	inline void VerifyStatement(char const * file, int line, char const * statement)
	{
#if ! defined(NDEBUG)
#if defined(GLPP_LOG)
		Log(file, line, std::cerr) << statement << std::endl << std::flush;
#endif
		Verify(file, line, statement);
#endif
	}

}
