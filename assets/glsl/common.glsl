//
//  common.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-19.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#define ENABLE_LIGHTING

#if defined(GL_ES)
#define COLOR3 lowp vec3
#define COLOR4 lowp vec4
#define UV_COORD medp vec2
#define SCALAR highp float
#define VECTOR2 highp vec2
#define VECTOR3 highp vec3
#define VECTOR4 highp vec4
#define MATRIX4 highp mat4
#else
#define COLOR3 vec3
#define COLOR4 vec4
#define UV_COORD vec2
#define SCALAR float
#define VECTOR2 vec2
#define VECTOR3 vec3
#define VECTOR4 vec4
#define MATRIX4 mat4
#endif
