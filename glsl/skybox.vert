#version 120

//
//  skybox.vert
//  crag
//
//  Created by John McFarlane on 2013-04-14.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// outputs to skybox.frag
varying vec4 tex_coord;

void main(void)
{
//	vec3 position = (gl_ModelViewMatrix * gl_Vertex).xyz;
//	vec4 vertex = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz + position.xyz, gl_Vertex.w);	
//	gl_Position = vec4(vertex.xyz, vertex.z + 0.1);			// force the vertex to reside on the far plane

//	vec4 vertex = gl_ModelViewProjectionMatrix * gl_Vertex.xyww;
	vec4 vertex = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
	gl_Position = vec4(vertex.xyw, vertex.w);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	tex_coord = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;
}
