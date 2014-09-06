//
//  common.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-19.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(GL_ES)

// mirrors C/C++ macro
#define CRAG_USE_GLES

#define COLOR_PRECISION lowp
#define UV_PRECISION mediump

#define COLOR3 COLOR_PRECISION vec3
#define COLOR4 COLOR_PRECISION vec4
#define UV_COORD2 UV_PRECISION vec2
#define UV_COORD3 UV_PRECISION vec3
#define SCALAR VECTOR_PRECISION float
#define VECTOR2 VECTOR_PRECISION vec2
#define VECTOR3 VECTOR_PRECISION vec3
#define VECTOR4 VECTOR_PRECISION vec4
#define MATRIX4 VECTOR_PRECISION mat4
#else
#define COLOR3 vec3
#define COLOR4 vec4
#define UV_COORD2 vec2
#define UV_COORD3 vec3
#define SCALAR float
#define VECTOR2 vec2
#define VECTOR3 vec3
#define VECTOR4 vec4
#define MATRIX4 mat4

// mirrors C/C++ macro
#define CRAG_USE_GL

#endif

float Squared(float s)
{
	return s * s;
}
